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

      /*! Private implementation of wassail::data::pciaccess::evaluate() */
      void evaluate(pciaccess &d, bool force);
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
      if (force or not collected) {
#ifdef WITH_DATA_PCIACCESS
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        int rv;
        struct pci_device_iterator *i;
        struct pci_device *dev;

        void *handle = dlopen("libpciaccess.so", RTLD_LAZY);
        if (not handle) {
          throw std::runtime_error(dlerror());
        }

        /* Load symbols */
        struct pci_device *(*_pci_device_next)(struct pci_device_iterator *) =
            (struct pci_device * (*)(struct pci_device_iterator *))
                dlsym(handle, "pci_device_next");
        const char *(*_pci_device_get_device_name)(const struct pci_device *) =
            (const char *(*)(const struct pci_device *))dlsym(
                handle, "pci_device_get_device_name");
        const char *(*_pci_device_get_subdevice_name)(
            const struct pci_device *) =
            (const char *(*)(const struct pci_device *))dlsym(
                handle, "pci_device_get_subdevice_name");
        const char *(*_pci_device_get_subvendor_name)(
            const struct pci_device *) =
            (const char *(*)(const struct pci_device *))dlsym(
                handle, "pci_device_get_subvendor_name");
        const char *(*_pci_device_get_vendor_name)(const struct pci_device *) =
            (const char *(*)(const struct pci_device *))dlsym(
                handle, "pci_device_get_vendor_name");
        struct pci_device_iterator *(*_pci_slot_match_iterator_create)(
            const struct pci_slot_match *) =
            (struct pci_device_iterator * (*)(const struct pci_slot_match *))
                dlsym(handle, "pci_slot_match_iterator_create");
        void (*_pci_system_cleanup)() =
            (void (*)())dlsym(handle, "pci_system_cleanup");
        int (*_pci_system_init)() = (int (*)())dlsym(handle, "pci_system_init");

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
          const char *dev_name, *subdev_name, *subvend_name;

          item.vendor_name = _pci_device_get_vendor_name(dev);

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
      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      try {
        for (auto i : j.at("data").at("devices")) {
          pciaccess::impl::pci_item p;

          p.bus = i.at("bus").get<uint8_t>();
          p.class_id = i.at("class_id").get<uint16_t>();
          p.dev = i.at("dev").get<uint8_t>();
          p.device_id = i.at("device_id").get<uint16_t>();
          p.device_name = i.at("device_name").get<std::string>();
          p.domain = i.at("domain").get<uint32_t>();
          p.func = i.at("func").get<uint8_t>();
          p.irq = i.at("irq").get<int>();
          p.revision = i.at("revision").get<uint8_t>();
          p.slot = i.at("slot").get<std::string>();
          p.subdevice_id = i.at("subdevice_id").get<uint16_t>();
          p.subdevice_name = i.at("subdevice_name").get<std::string>();
          p.subvendor_id = i.at("subvendor_id").get<uint16_t>();
          p.subvendor_name = i.at("subvendor_name").get<std::string>();
          p.vendor_id = i.at("vendor_id").get<uint16_t>();
          p.vendor_name = i.at("vendor_name").get<std::string>();

          d.pimpl->data.devices.push_back(p);
        }
      }
      catch (std::exception &e) {
        throw std::runtime_error("Unable to convert JSON string '" + j.dump() +
                                 "' to object: " + e.what());
      }
    }

    void to_json(json &j, const pciaccess &d) {
      j = dynamic_cast<const wassail::data::common &>(d);

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
