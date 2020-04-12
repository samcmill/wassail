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
#include <wassail/data/pciutils.hpp>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#ifdef HAVE_PCI_PCI_H
#include <pci/pci.h>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class pciutils::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

      /*! \brief PCI device attributes */
      struct pci_item {
        std::string slot;        /*!< Slot, bus:dev:func triplet */
        std::string class_name;  /*!< PCI device class name */
        std::string device_name; /*!< PCI device device name */
        std::string vendor_name; /*!< PCI device vendor name */
        uint8_t bus;             /*!< Bus domain id */
        uint16_t class_id;       /*!< PCI device class id */
        uint8_t dev;             /*!< Device id */
        uint16_t device_id;      /*!< PCI device device id */
        uint16_t domain;         /*!< PCI domain id */
        uint8_t func;            /*!< Device function id */
        uint16_t vendor_id;      /*!< PCI device vendor id */
      };

      /*! \brief PCI devices */
      struct {
        std::list<pci_item> devices;
      } data; /*!< PCI devices */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::pciutils::evaluate() */
      void evaluate(pciutils &d, bool force);

    private:
#ifdef WITH_DATA_PCIUTILS
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

    pciutils::pciutils() : pimpl{std::make_unique<impl>()} {}
    pciutils::~pciutils() = default;
    pciutils::pciutils(pciutils &&) = default;            // LCOV_EXCL_LINE
    pciutils &pciutils::operator=(pciutils &&) = default; // LCOV_EXCL_LINE

    bool pciutils::enabled() const {
#ifdef WITH_DATA_PCIUTILS
      return true;
#else
      return false;
#endif
    }

    void pciutils::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void pciutils::impl::evaluate(pciutils &d, bool force) {
      if (force or not collected) {
#ifdef WITH_DATA_PCIUTILS
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        struct pci_access *p;

        handle = dlopen("libpci.so", RTLD_LAZY);
        if (not handle) {
          wassail::internal::logger()->error(
              "unable to load libpci library: {}", dlerror());
          return;
        }

        /* Load symbols */
        auto const _pci_alloc = load_symbol<struct pci_access *()>("pci_alloc");
        auto const _pci_cleanup =
            load_symbol<void(struct pci_access *)>("pci_cleanup");
        auto const _pci_fill_info =
            load_symbol<int(struct pci_dev *, int)>("pci_fill_info");
        auto const _pci_init =
            load_symbol<void(struct pci_access *)>("pci_init");
        auto const _pci_scan_bus =
            load_symbol<void(struct pci_access *)>("pci_scan_bus");
        /* special handling for symbol with variadic arguments */
        char *(*_pci_lookup_name)(struct pci_access *, char *, int, int, ...) =
            (char *(*)(struct pci_access *, char *, int, int, ...))dlsym(
                handle, "pci_lookup_name");

        p = _pci_alloc();
        if (p == NULL) {
          wassail::internal::logger()->error(
              "unable to allocate pci structure");
          dlclose(handle);
          return;
        }

        _pci_init(p);
        _pci_scan_bus(p);

        for (struct pci_dev *dev = p->devices; dev; dev = dev->next) {
          pci_item item;
          char classbuf[1024], devicebuf[1024], slot[16], vendorbuf[1024];

          _pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);

          std::snprintf(slot, sizeof(slot), "%02x:%02x.%d", dev->bus, dev->dev,
                        dev->func);
          item.slot = slot;

          item.device_name = _pci_lookup_name(p, devicebuf, sizeof(devicebuf),
                                              PCI_LOOKUP_DEVICE, dev->vendor_id,
                                              dev->device_id);
          item.class_name =
              _pci_lookup_name(p, classbuf, sizeof(classbuf), PCI_LOOKUP_CLASS,
                               dev->device_class);
          item.vendor_name = _pci_lookup_name(p, vendorbuf, sizeof(vendorbuf),
                                              PCI_LOOKUP_VENDOR, dev->vendor_id,
                                              dev->device_id);

          item.bus = dev->bus;
          item.class_id = dev->device_class;
          item.dev = dev->dev;
          item.device_id = dev->device_id;
          item.domain = dev->domain;
          item.func = dev->func;
          item.vendor_id = dev->vendor_id;

          data.devices.push_back(item);
        }

        d.timestamp = std::chrono::system_clock::now();
        collected = true;

        _pci_cleanup(p);

        dlclose(handle);
#else
        throw std::runtime_error("pciutils data source is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, pciutils &d) {
      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      try {
        for (auto i : j.at("data").at("devices")) {
          pciutils::impl::pci_item p;

          p.bus = i.at("bus").get<uint8_t>();
          p.class_id = i.at("class_id").get<uint16_t>();
          p.class_name = i.at("class_name").get<std::string>();
          p.dev = i.at("dev").get<uint8_t>();
          p.device_id = i.at("device_id").get<uint16_t>();
          p.device_name = i.at("device_name").get<std::string>();
          p.domain = i.at("domain").get<uint16_t>();
          p.func = i.at("func").get<uint8_t>();
          p.slot = i.at("slot").get<std::string>();
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

    void to_json(json &j, const pciutils &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["devices"] = json::array();

      for (auto i : d.pimpl->data.devices) {
        json device;

        device["bus"] = i.bus;
        device["class_id"] = i.class_id;
        device["class_name"] = i.class_name;
        device["dev"] = i.dev;
        device["device_id"] = i.device_id;
        device["device_name"] = i.device_name;
        device["domain"] = i.domain;
        device["func"] = i.func;
        device["slot"] = i.slot;
        device["vendor_id"] = i.vendor_id;
        device["vendor_name"] = i.vendor_name;

        j["data"]["devices"].push_back(device);
      }

      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
