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
#include <string>
#include <wassail/data/pciaccess.hpp>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#ifdef HAVE_PCIACCESS_H
#include <pciaccess.h>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class pciaccess::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

      /*! \brief PCI device attributes */
      struct pci_item {
        std::string slot;           /*!< Slot, bus:dev:func triplet */
        std::string device_name;    /*!< PCI device device name */
        std::string subdevice_name; /*!< PCI device subdevice name */
        std::string subvendor_name; /*!< PCI device subvendor name */
        std::string vendor_name;    /*!< PCI device vendor name */
        uint8_t bus;                /*!< Bus domain id */
        uint16_t class_id;          /*!< PCI device class id */
        uint8_t dev;                /*!< Device id */
        uint16_t device_id;         /*!< PCI device device id */
        uint32_t domain;            /*!< PCI domain id */
        uint8_t func;               /*!< Device function id */
        int irq;                    /*!< IRQ associated with the device */
        uint8_t revision;           /*!< Device revision number */
        uint16_t subdevice_id;      /*!< PCI sub-device id */
        uint16_t subvendor_id;      /*!< PCI sub-vendor id */
        uint16_t vendor_id;         /*!< PCI device vendor id */
      };

      /*! \brief PCI devices */
      struct {
        std::list<pci_item> devices;
      } data; /*!< PCI devices */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::pciaccess::evaluate() */
      void evaluate(pciaccess &d, bool force);

    private:
#ifdef WITH_DATA_PCIACCESS
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

    pciaccess::pciaccess() : pimpl{std::make_unique<impl>()} {}
    pciaccess::~pciaccess() = default;
    pciaccess::pciaccess(pciaccess &&) = default;            // LCOV_EXCL_LINE
    pciaccess &pciaccess::operator=(pciaccess &&) = default; // LCOV_EXCL_LINE

    bool pciaccess::enabled() const {
#ifdef WITH_DATA_PCIACCESS
      return true;
#else
      return false;
#endif
    }

    void pciaccess::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void pciaccess::impl::evaluate(pciaccess &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not collected) {
#ifdef WITH_DATA_PCIACCESS
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        int rv;
        struct pci_device_iterator *i;
        struct pci_device *dev;

        handle = dlopen("libpciaccess.so", RTLD_LAZY);
        if (not handle) {
          wassail::internal::logger()->error(
              "unable to load libpciaccess library: {}", dlerror());
          return;
        }

        /* Load symbols */
        auto const _pci_device_next =
            load_symbol<struct pci_device *(struct pci_device_iterator *)>(
                "pci_device_next");
        auto const _pci_device_get_device_name =
            load_symbol<const char *(const struct pci_device *)>(
                "pci_device_get_device_name");
        auto const _pci_device_get_subdevice_name =
            load_symbol<const char *(const struct pci_device *)>(
                "pci_device_get_subdevice_name");
        auto const _pci_device_get_subvendor_name =
            load_symbol<const char *(const struct pci_device *)>(
                "pci_device_get_subdevice_name");
        auto const _pci_device_get_vendor_name =
            load_symbol<const char *(const struct pci_device *)>(
                "pci_device_get_vendor_name");
        auto const _pci_slot_match_iterator_create =
            load_symbol<struct pci_device_iterator *(
                const struct pci_slot_match *)>(
                "pci_slot_match_iterator_create");
        auto const _pci_system_cleanup =
            load_symbol<void()>("pci_system_cleanup");
        auto const _pci_system_init = load_symbol<int()>("pci_system_init");

        rv = _pci_system_init();
        if (rv != 0) {
          wassail::internal::logger()->error("unable to initialize pci system");
          dlclose(handle);
          return;
        }

        i = _pci_slot_match_iterator_create(NULL);

        while ((dev = _pci_device_next(i)) != NULL) {
          pci_item item;
          char slot[16];
          const char *dev_name, *subdev_name, *subvend_name, *vendor_name;

          vendor_name = _pci_device_get_vendor_name(dev);
          if (vendor_name != NULL) {
            item.vendor_name = vendor_name;
          }

          dev_name = _pci_device_get_device_name(dev);
          if (dev_name != NULL) {
            item.device_name = dev_name;
          }

          subdev_name = _pci_device_get_subdevice_name(dev);
          if (subdev_name != NULL) {
            item.subdevice_name = subdev_name;
          }

          subvend_name = _pci_device_get_subvendor_name(dev);
          if (subvend_name != NULL) {
            item.subvendor_name = subvend_name;
          }

          std::snprintf(slot, sizeof(slot), "%02x:%02x.%d", dev->bus, dev->dev,
                        dev->func);
          item.slot = slot;

          item.bus = dev->bus;
          item.class_id = dev->device_class;
          item.device_id = dev->device_id;
          item.dev = dev->dev;
          item.domain = dev->domain;
          item.func = dev->func;
          item.irq = dev->irq;
          item.revision = dev->revision;
          item.subdevice_id = dev->subdevice_id;
          item.subvendor_id = dev->subvendor_id;
          item.vendor_id = dev->vendor_id;

          data.devices.push_back(item);
        }

        d.timestamp = std::chrono::system_clock::now();
        collected = true;

        _pci_system_cleanup();

        dlclose(handle);
#else
        throw std::runtime_error("pciaccess data source is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, pciaccess &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("version", 0) != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      for (auto i :
           j.value(json::json_pointer("/data/devices"), json::array())) {
        pciaccess::impl::pci_item p;

        p.bus = i.value("bus", 0U);
        p.class_id = i.value("class_id", 0U);
        p.dev = i.value("dev", 0U);
        p.device_id = i.value("device_id", 0U);
        p.device_name = i.value("device_name", "");
        p.domain = i.value("domain", 0UL);
        p.func = i.value("func", 0U);
        p.irq = i.value("irq", 0);
        p.revision = i.value("revision", 0U);
        p.slot = i.value("slot", "");
        p.subdevice_id = i.value("subdevice_id", 0U);
        p.subdevice_name = i.value("subdevice_name", "");
        p.subvendor_id = i.value("subvendor_id", 0U);
        p.subvendor_name = i.value("subvendor_name", "");
        p.vendor_id = i.value("vendor_id", 0U);
        p.vendor_name = i.value("vendor_name", "");

        d.pimpl->data.devices.push_back(p);
      }
    }

    void to_json(json &j, const pciaccess &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["devices"] = json::array();

      for (auto i : d.pimpl->data.devices) {
        json device;

        device["bus"] = i.bus;
        device["class_id"] = i.class_id;
        device["dev"] = i.dev;
        device["device_id"] = i.device_id;
        device["device_name"] = i.device_name;
        device["domain"] = i.domain;
        device["func"] = i.func;
        device["irq"] = i.irq;
        device["revision"] = i.revision;
        device["slot"] = i.slot;
        device["subdevice_id"] = i.subdevice_id;
        device["subdevice_name"] = i.subdevice_name;
        device["subvendor_id"] = i.subvendor_id;
        device["subvendor_name"] = i.subvendor_name;
        device["vendor_id"] = i.vendor_id;
        device["vendor_name"] = i.vendor_name;

        j["data"]["devices"].push_back(device);
      }

      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
