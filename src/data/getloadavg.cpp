/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include <cstdlib>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <wassail/data/getloadavg.hpp>

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class getloadavg::impl {
    public:
      /*! \brief System load average data */
      struct {
        double load1;  /*!< 1 minute load average */
        double load5;  /*!< 5 minute load average */
        double load15; /*!< 15 minute load average */
      } data;          /*!< System load average data */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::getloadavg::evaluate() */
      void evaluate(getloadavg &d, bool force);
    };

    getloadavg::getloadavg() : pimpl{std::make_unique<impl>()} {}
    getloadavg::~getloadavg() = default;
    getloadavg::getloadavg(getloadavg &&) = default; // LCOV_EXCL_LINE
    getloadavg &getloadavg::
    operator=(getloadavg &&) = default; // LCOV_EXCL_LINE

    bool getloadavg::enabled() const {
#ifdef HAVE_GETLOADAVG
      return true;
#else
      return false;
#endif
    }

    void getloadavg::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void getloadavg::impl::evaluate(getloadavg &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not d.collected()) {
#ifdef HAVE_GETLOADAVG
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        double loadavg[3];
        int rv = ::getloadavg(loadavg, 3);
        if (rv != -1) {
          data.load1 = loadavg[0];
          data.load5 = loadavg[1];
          data.load15 = loadavg[2];

          d.common::evaluate(force);
        }

#else
        throw std::runtime_error("getloadavg() not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, getloadavg &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("name", "") != d.name()) {
        throw std::runtime_error("name mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      d.pimpl->data.load1 = j.value(json::json_pointer("/data/load1"), 0.0);
      d.pimpl->data.load5 = j.value(json::json_pointer("/data/load5"), 0.0);
      d.pimpl->data.load15 = j.value(json::json_pointer("/data/load15"), 0.0);
    }

    void to_json(json &j, const getloadavg &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["load1"] = d.pimpl->data.load1;
      j["data"]["load5"] = d.pimpl->data.load5;
      j["data"]["load15"] = d.pimpl->data.load15;
      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
