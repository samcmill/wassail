/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <cstdlib>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <wassail/data/udev.hpp>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#ifdef HAVE_LIBUDEV_H
#include <libudev.h>
#endif

/* The udev path may contain elements not allowed by the JSON specification,
 * such as numbers with leading zeros.  When a JSON data structure is created in
 * place, i.e., `json j = json::object(); j[json::json_pointer("/a/b/01/c/d")] =
 * "foo";`, the nlohmann JSON implementation infers the type of each token, so
 * "01" is processed as a number, and thus is invalid.  However, if the data
 * structure is created iteratively, then each token is always mapped to string
 * key; "01" is a valid JSON string.  Walk the JSON pointer and create JSON
 * objects if the token does not already exist as a key.
 */
json::json_pointer initialize_pointer_object(json &root,
                                             const std::string &str) {
  auto ptr = json::json_pointer(str);

  /* Walk the parent hierarchy using native pointer deconstruction */
  if (!ptr.empty()) {
    auto parent = ptr.parent_pointer();

    /* Use a mini-recursive check to ensure nested parents exist */
    auto init_parents = [](auto &self, json &j,
                           const json::json_pointer &p) -> void {
      if (p.empty())
        return;
      self(self, j, p.parent_pointer());
      if (j[p].is_null()) {
        j[p] = json::object();
      }
    };

    init_parents(init_parents, root, parent);
  }

  return ptr;
}

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class udev::impl {
    public:
      /*! \brief udev sysfs entries */
      struct {
        json devices = json::object(); /* sysfs devices, i.e., /sys/devices */
      } data;                          /* udev sysfs entries */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::udev::evaluate() */
      void evaluate(udev &d, bool force);

    private:
#ifdef WITH_DATA_UDEV
      void *handle = nullptr; /*!< Library handle */

      template <class T>
      std::function<T> load_symbol(std::string const &name) {
        void *const symbol = dlsym(handle, name.c_str());

        if (not symbol) {
          throw std::runtime_error(dlerror());
        }

        return reinterpret_cast<T *>(symbol);
      }
#endif
    };

    udev::udev() : pimpl{std::make_unique<impl>()} {}
    udev::~udev() = default;
    udev::udev(udev &&) = default;            // LCOV_EXCL_LINE
    udev &udev::operator=(udev &&) = default; // LCOV_EXCL_LINE

    bool udev::enabled() const {
#ifdef WITH_DATA_UDEV
      return true;
#else
      return false;
#endif
    }

    void udev::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void udev::impl::evaluate(udev &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not d.collected()) {
#ifdef WITH_DATA_UDEV
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        handle = dlopen("libudev.so.1", RTLD_LAZY);
        if (not handle) {
          wassail::internal::logger()->error(
              "unable to load libudev library: {}", dlerror());
          return;
        }

        /* Load symbols */
        auto const _udev_device_get_sysattr_list_entry =
            load_symbol<struct udev_list_entry *(struct udev_device *)>(
                "udev_device_get_sysattr_list_entry");
        auto const _udev_device_get_sysattr_value =
            load_symbol<const char *(struct udev_device *, const char *)>(
                "udev_device_get_sysattr_value");
        auto const _udev_device_new_from_syspath =
            load_symbol<struct udev_device *(struct udev *, const char *)>(
                "udev_device_new_from_syspath");
        auto const _udev_enumerate_get_list_entry =
            load_symbol<struct udev_list_entry *(struct udev_enumerate *)>(
                "udev_enumerate_get_list_entry");
        auto const _udev_enumerate_new =
            load_symbol<struct udev_enumerate *(struct udev *)>(
                "udev_enumerate_new");
        auto const _udev_enumerate_scan_devices =
            load_symbol<int(struct udev_enumerate *)>(
                "udev_enumerate_scan_devices");
        auto const _udev_enumerate_unref =
            load_symbol<struct udev_enumerate *(struct udev_enumerate *)>(
                "udev_enumerate_unref");
        auto const _udev_list_entry_get_name =
            load_symbol<const char *(struct udev_list_entry *)>(
                "udev_list_entry_get_name");
        auto const _udev_list_entry_get_next =
            load_symbol<struct udev_list_entry *(struct udev_list_entry *)>(
                "udev_list_entry_get_next");
        auto const _udev_new = load_symbol<struct udev *()>("udev_new");
        auto const _udev_unref =
            load_symbol<struct udev *(struct udev *)>("udev_unref");

        struct udev *u = _udev_new();
        if (not u) {
          wassail::internal::logger()->error(
              "unable to initialize libudev library");
          dlclose(handle);
          return;
        }

        struct udev_enumerate *enumerate = _udev_enumerate_new(u);
        _udev_enumerate_scan_devices(enumerate);

        /* loop over devices */
        for (struct udev_list_entry *entry =
                 _udev_enumerate_get_list_entry(enumerate);
             entry != NULL; entry = _udev_list_entry_get_next(entry)) {
          const char *path = _udev_list_entry_get_name(entry);
          struct udev_device *device = _udev_device_new_from_syspath(u, path);

          /* path is equivalent to a json pointer, e.g.,
           * /sys/devices/virtual/net/eth0 */
          try {
            data.devices[json::json_pointer(path)] = json::object();
          }
          catch (const json::parse_error &e) {
            wassail::internal::logger()->warn(
                "unable to process JSON pointer '{0}': {1}", path, e.what());
          }

          /* get attributes */
          for (struct udev_list_entry *attr =
                   _udev_device_get_sysattr_list_entry(device);
               attr != NULL; attr = _udev_list_entry_get_next(attr)) {
            const char *name = _udev_list_entry_get_name(attr);
            const char *value = _udev_device_get_sysattr_value(device, name);

            std::string jstr = std::string(path) + "/" + std::string(name);
            auto jptr = initialize_pointer_object(data.devices, jstr);

            try {
              if (value != NULL) {
                data.devices[jptr] = value;
              }
              else {
                data.devices[jptr] = nullptr;
              }
            }
            catch (const json::parse_error &e) {
              wassail::internal::logger()->warn(
                  "unable to process JSON pointer '{0}': {1}", jstr, e.what());
            }
          }
        }

        _udev_enumerate_unref(enumerate);
        _udev_unref(u);

        d.common::evaluate_common();

        dlclose(handle);
#else
        throw std::runtime_error("udev data source is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, udev &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("name", "") != d.name()) {
        throw std::runtime_error("name mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      d.pimpl->data.devices = j.value("data", json::object());
    }

    void to_json(json &j, const udev &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"] = d.pimpl->data.devices;

      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
