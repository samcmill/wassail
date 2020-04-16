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
#include <wassail/data/umad.hpp>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#ifdef HAVE_INFINIBAND_UMAD_H
#include <infiniband/umad.h>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class umad::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

      /*! \brief Port attributes */
      struct port_item {
        std::string ca_name;    /*!< Name of the device */
        int portnum;            /*!< Physical port number */
        uint base_lid;          /*!< Base port LID */
        uint lmc;               /*!< LMC of LID */
        uint sm_lid;            /*!< SM LID */
        uint sm_sl;             /*!< SM service level */
        uint state;             /*!< Logical port state */
        uint phys_state;        /*!< Physical port state */
        uint rate;              /*!< Port link bit rate */
        uint64_t capmask;       /*!< Port capabilities */
        uint64_t gid_prefix;    /*!< Gid prefix of this port */
        uint64_t port_guid;     /*!< GUID of this port */
        std::string link_layer; /*!< Link layer */
      };

      /*! \brief CA attributes */
      struct ca_item {
        std::string name;     /*!< Name of the device */
        uint node_type;       /*!< Type of the device */
        int numports;         /*!< Number of physical ports */
        std::string fw_ver;   /*!< FW version */
        std::string ca_type;  /*!< CA type */
        std::string hw_ver;   /*!< Hardware version */
        uint64_t node_guid;   /*!< Node GUID */
        uint64_t system_guid; /*!< System image GUID */
        std::list<port_item> ports;
      };

      /*! \brief CAs */
      struct {
        std::list<ca_item> devices;
      } data; /*!< CA devices */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::umad::evaluate() */
      void evaluate(umad &d, bool force);

    private:
#ifdef WITH_DATA_UMAD
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

    umad::umad() : pimpl{std::make_unique<impl>()} {}
    umad::~umad() = default;
    umad::umad(umad &&) = default;            // LCOV_EXCL_LINE
    umad &umad::operator=(umad &&) = default; // LCOV_EXCL_LINE

    bool umad::enabled() const {
#ifdef WITH_DATA_UMAD
      return true;
#else
      return false;
#endif
    }

    void umad::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void umad::impl::evaluate(umad &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not collected) {
#ifdef WITH_DATA_UMAD
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        int rv;

        handle = dlopen("libibumad.so", RTLD_LAZY);
        if (not handle) {
          wassail::internal::logger()->error(
              "unable to load libibumad library: {}", dlerror());
          return;
        }

        /* Load symbols */
        auto const _umad_init = load_symbol<int()>("umad_init");
        auto const _umad_get_cas_names =
            load_symbol<int(char(*)[UMAD_CA_NAME_LEN], int)>(
                "umad_get_cas_names");
        auto const _umad_get_ca =
            load_symbol<int(char *, umad_ca_t *)>("umad_get_ca");
        auto const _umad_release_ca =
            load_symbol<int(umad_ca_t *)>("umad_release_ca");

        rv = _umad_init();
        if (rv != 0) {
          wassail::internal::logger()->error(
              "unable to initialize umad library");
          dlclose(handle);
          return;
        }

        char names[UMAD_MAX_DEVICES][UMAD_CA_NAME_LEN];
        int num_cas = _umad_get_cas_names(names, UMAD_MAX_DEVICES);
        if (num_cas > 0) {
          for (int i = 0; i < num_cas; i++) {
            ca_item item;
            umad_ca_t ca;

            rv = _umad_get_ca(names[i], &ca);
            if (rv != 0) {
              wassail::internal::logger()->error(
                  "unable to get device attributes for {}", names[i]);
              continue;
            }

            item.name = ca.ca_name;
            item.node_type = ca.node_type;
            item.numports = ca.numports;
            item.fw_ver = ca.fw_ver;
            item.ca_type = ca.ca_type;
            item.hw_ver = ca.hw_ver;
            item.node_guid = ca.node_guid;
            item.system_guid = ca.system_guid;

            for (int j = 1; j <= ca.numports; j++) {
              port_item pitem;

              if (not ca.ports[j]) {
                wassail::internal::logger()->error(
                    "unable to get port attributes for {0}:{1}", names[i], i);
                continue;
              }

              pitem.ca_name = ca.ports[j]->ca_name;
              pitem.portnum = ca.ports[j]->portnum;
              pitem.base_lid = ca.ports[j]->base_lid;
              pitem.lmc = ca.ports[j]->lmc;
              pitem.sm_lid = ca.ports[j]->sm_lid;
              pitem.sm_sl = ca.ports[j]->sm_sl;
              pitem.state = ca.ports[j]->state;
              pitem.phys_state = ca.ports[j]->phys_state;
              pitem.rate = ca.ports[j]->rate;
              pitem.capmask = ca.ports[j]->capmask;
              pitem.gid_prefix = ca.ports[j]->gid_prefix;
              pitem.port_guid = ca.ports[j]->port_guid;
              pitem.link_layer = ca.ports[j]->link_layer;

              item.ports.push_back(pitem);
            }

            data.devices.push_back(item);

            rv = _umad_release_ca(&ca);
            if (rv != 0) {
              wassail::internal::logger()->warn(
                  "error releasing device attribute for {}", names[i]);
            }
          }
        }
        else {
          wassail::internal::logger()->error(
              "unable to list InfiniBand device names");
          dlclose(handle);
          return;
        }

        d.timestamp = std::chrono::system_clock::now();
        collected = true;

        dlclose(handle);
#else
        throw std::runtime_error("umad data source is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, umad &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("version", 0) != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      for (auto i :
           j.value(json::json_pointer("/data/devices"), json::array())) {
        umad::impl::ca_item ca;

        ca.name = i.value("name", "");
        ca.node_type = i.value("node_type", 0U);
        ca.numports = i.value("numports", 0);
        ca.fw_ver = i.value("fw_ver", "");
        ca.ca_type = i.value("ca_type", "");
        ca.hw_ver = i.value("hw_ver", "");
        ca.node_guid = i.value("node_guid", 0ULL);
        ca.system_guid = i.value("system_guid", 0ULL);

        for (auto p : i.value("ports", json::array())) {
          umad::impl::port_item port;

          port.ca_name = p.value("ca_name", "");
          port.portnum = p.value("portnum", 0);
          port.base_lid = p.value("base_lid", 0U);
          port.lmc = p.value("lmc", 0U);
          port.sm_lid = p.value("sm_lid", 0U);
          port.sm_sl = p.value("sm_sl", 0U);
          port.state = p.value("state", 0U);
          port.phys_state = p.value("phys_state", 0U);
          port.rate = p.value("rate", 0U);
          port.capmask = p.value("capmask", 0ULL);
          port.gid_prefix = p.value("gid_prefix", 0ULL);
          port.port_guid = p.value("port_guid", 0ULL);
          port.link_layer = p.value("link_layer", "");

          ca.ports.push_back(port);
        }

        d.pimpl->data.devices.push_back(ca);
      }
    }

    void to_json(json &j, const umad &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["devices"] = json::array();

      for (auto i : d.pimpl->data.devices) {
        json device;

        device["name"] = i.name;
        device["node_type"] = i.node_type;
        device["numports"] = i.numports;
        device["fw_ver"] = i.fw_ver;
        device["ca_type"] = i.ca_type;
        device["hw_ver"] = i.hw_ver;
        device["node_guid"] = i.node_guid;
        device["system_guid"] = i.system_guid;

        device["ports"] = json::array();

        for (auto p : i.ports) {
          json port;

          port["ca_name"] = p.ca_name;
          port["portnum"] = p.portnum;
          port["base_lid"] = p.base_lid;
          port["lmc"] = p.lmc;
          port["sm_lid"] = p.sm_lid;
          port["sm_sl"] = p.sm_sl;
          port["state"] = p.state;
          port["phys_state"] = p.phys_state;
          port["rate"] = p.rate;
          port["capmask"] = p.capmask;
          port["gid_prefix"] = p.gid_prefix;
          port["port_guid"] = p.port_guid;
          port["link_layer"] = p.link_layer;

          device["ports"].push_back(port);
        }

        j["data"]["devices"].push_back(device);
      }

      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
