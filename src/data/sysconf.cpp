/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include <chrono>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <wassail/data/sysconf.hpp>

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class sysconf::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the load
                                average has been collected */

      /*! \brief System configuration */
      struct {
        long nprocessors_conf; /*!< Number of processors configured */
        long nprocessors_onln; /*!< Number of processors currently online */
        long page_size;        /*!< Size of a system page in bytes */
        long phys_pages;       /*!< Number of pages of physical memory */
      } data;                  /*!< System configuration */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::sysconf::evaluate() */
      void evaluate(sysconf &d, bool force);
    };

    sysconf::sysconf() : pimpl{std::make_unique<impl>()} {}
    sysconf::~sysconf() = default;
    sysconf::sysconf(sysconf &&) = default;            // LCOV_EXCL_LINE
    sysconf &sysconf::operator=(sysconf &&) = default; // LCOV_EXCL_LINE

    bool sysconf::enabled() const {
#ifdef HAVE_SYSCONF
      return true;
#else
      return false;
#endif
    }

    void sysconf::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void sysconf::impl::evaluate(sysconf &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not collected) {
#ifdef HAVE_SYSCONF
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        data.nprocessors_conf = ::sysconf(_SC_NPROCESSORS_CONF);
        data.nprocessors_onln = ::sysconf(_SC_NPROCESSORS_ONLN);
        data.page_size = ::sysconf(_SC_PAGESIZE);
        data.phys_pages = ::sysconf(_SC_PHYS_PAGES);

        d.timestamp = std::chrono::system_clock::now();
        collected = true;
      }
#else
        throw std::runtime_error("sysconf() not available");
#endif
    }
    /* \endcond */

    void from_json(const json &j, sysconf &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      try {
        auto jdata = j.at("data");
        d.pimpl->data.nprocessors_conf =
            jdata.at("nprocessors_conf").get<long>();
        d.pimpl->data.nprocessors_onln =
            jdata.at("nprocessors_onln").get<long>();
        d.pimpl->data.page_size = jdata.at("page_size").get<long>();
        d.pimpl->data.phys_pages = jdata.at("phys_pages").get<long>();
      }
      catch (std::exception &e) {
        throw std::runtime_error(
            std::string("Unable to convert JSON string '") + j.dump() +
            std::string("' to object: ") + e.what());
      }
    }

    void to_json(json &j, const sysconf &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["nprocessors_conf"] = d.pimpl->data.nprocessors_conf;
      j["data"]["nprocessors_onln"] = d.pimpl->data.nprocessors_onln;
      j["data"]["page_size"] = d.pimpl->data.page_size;
      j["data"]["phys_pages"] = d.pimpl->data.phys_pages;
      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
