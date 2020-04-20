/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <chrono>
#include <cstdlib>
#include <list>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <wassail/data/udev.hpp>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#ifdef HAVE_LIBUDEV_H
#include <libudev.h>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class udev::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

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

      if (force or not collected) {
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

        struct udev_enumerate *e = _udev_enumerate_new(u);
        _udev_enumerate_scan_devices(e);

        /* loop over devices */
        for (struct udev_list_entry *d = _udev_enumerate_get_list_entry(e); d;
             d = _udev_list_entry_get_next(d)) {
          const char *path = _udev_list_entry_get_name(d);
          struct udev_device *device = _udev_device_new_from_syspath(u, path);

          /* path is equivalent to a json pointer, e.g.,
           * /sys/devices/virtual/net/eth0 */
          data.devices[json::json_pointer(path)] = json::object();

          /* get attributes */
          for (struct udev_list_entry *a =
                   _udev_device_get_sysattr_list_entry(device);
               a; a = _udev_list_entry_get_next(a)) {
            const char *name = _udev_list_entry_get_name(a);
            const char *value = _udev_device_get_sysattr_value(device, name);

            if (value != NULL) {
              data.devices[json::json_pointer(std::string(path) + "/" +
                                              std::string(name))] = value;
            }
            else {
              data.devices[json::json_pointer(std::string(path) + "/" +
                                              std::string(name))] = nullptr;
            }
          }
        }

        _udev_enumerate_unref(e);
        _udev_unref(u);

        d.timestamp = std::chrono::system_clock::now();
        collected = true;

        dlclose(handle);
#else
        throw std::runtime_error("udev data source is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, udev &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("version", 0) != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      d.pimpl->data.devices = j.value("data", json::object());

      d.pimpl->collected = true;
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
