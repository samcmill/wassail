/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <wassail/data/sysinfo.hpp>

#ifdef HAVE_SYS_SYSINFO_H
#include <sys/sysinfo.h>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class sysinfo::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the load
                                average has been collected */

      /*! \brief System information */
      struct {
        long uptime;               /*!< Seconds since boot */
        unsigned long loads[3];    /*!< 1, 5, and 15 minute load averages */
        unsigned long totalram;    /*!< Total usable main memory size */
        unsigned long freeram;     /*!< Available memory size */
        unsigned long sharedram;   /*!< Amount of shared memory */
        unsigned long bufferram;   /*!< Memory used by buffers */
        unsigned long totalswap;   /*!< Total swap space size */
        unsigned long freeswap;    /*!< swap space still available */
        unsigned short procs;      /*!< Number of current processes */
        unsigned long totalhigh;   /*!< Total high memory size */
        unsigned long freehigh;    /*!< Available high memory size */
        unsigned int mem_unit;     /*!< Memory unit size in bytes */
        unsigned long loads_scale; /*!< Load average scale factor */
      } data;                      /*!< System information */

      /*! Private implementation of wassail::data::sysinfo::evaluate() */
      void evaluate(sysinfo &d, bool force);
    };

    sysinfo::sysinfo() : pimpl{std::make_unique<impl>()} {}
    sysinfo::~sysinfo() = default;
    sysinfo::sysinfo(sysinfo &&) = default;            // LCOV_EXCL_LINE
    sysinfo &sysinfo::operator=(sysinfo &&) = default; // LCOV_EXCL_LINE

    bool sysinfo::enabled() const {
#ifdef HAVE_SYSINFO
      return true;
#else
      return false;
#endif
    }

    void sysinfo::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void sysinfo::impl::evaluate(sysinfo &d, bool force = false) {
      if (force or not collected) {
#ifdef HAVE_SYSINFO
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        struct ::sysinfo info;
        int rv = ::sysinfo(&info);

        if (rv == 0) {
          data.uptime = info.uptime;
          data.loads[0] = info.loads[0];
          data.loads[1] = info.loads[1];
          data.loads[2] = info.loads[2];
          data.totalram = info.totalram;
          data.freeram = info.freeram;
          data.sharedram = info.sharedram;
          data.bufferram = info.bufferram;
          data.totalswap = info.totalswap;
          data.freeswap = info.freeswap;
          data.procs = info.procs;
          data.totalhigh = info.totalhigh;
          data.freehigh = info.freehigh;
          data.mem_unit = info.mem_unit;
          data.loads_scale = 1 << SI_LOAD_SHIFT;

          d.timestamp = std::chrono::system_clock::now();
          collected = true;
        }
#else
        throw std::runtime_error("sysinfo() not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, sysinfo &d) {
      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      try {
        auto jdata = j.at("data");
        d.pimpl->data.uptime = jdata.at("uptime").get<long>();
        d.pimpl->data.loads[0] = jdata.at("load1").get<unsigned long>();
        d.pimpl->data.loads[1] = jdata.at("load5").get<unsigned long>();
        d.pimpl->data.loads[2] = jdata.at("load15").get<unsigned long>();
        d.pimpl->data.totalram = jdata.at("totalram").get<unsigned long>();
        d.pimpl->data.freeram = jdata.at("freeram").get<unsigned long>();
        d.pimpl->data.sharedram = jdata.at("sharedram").get<unsigned long>();
        d.pimpl->data.bufferram = jdata.at("bufferram").get<unsigned long>();
        d.pimpl->data.totalswap = jdata.at("totalswap").get<unsigned long>();
        d.pimpl->data.freeswap = jdata.at("freeswap").get<unsigned long>();
        d.pimpl->data.procs = jdata.at("procs").get<unsigned short>();
        d.pimpl->data.totalhigh = jdata.at("totalhigh").get<unsigned long>();
        d.pimpl->data.freehigh = jdata.at("freehigh").get<unsigned long>();
        d.pimpl->data.mem_unit = jdata.at("mem_unit").get<unsigned int>();
        d.pimpl->data.loads_scale =
            jdata.at("loads_scale").get<unsigned long>();
      }
      catch (std::exception &e) {
        throw std::runtime_error(
            std::string("Unable to convert JSON string '") + j.dump() +
            std::string("' to object: ") + e.what());
      }
    }

    void to_json(json &j, const sysinfo &d) {
      j = dynamic_cast<const wassail::data::common &>(d);

      j["name"] = d.name();
      j["version"] = d.version();

      j["data"]["uptime"] = d.pimpl->data.uptime;
      j["data"]["load1"] = d.pimpl->data.loads[0];
      j["data"]["load5"] = d.pimpl->data.loads[1];
      j["data"]["load15"] = d.pimpl->data.loads[2];
      j["data"]["totalram"] = d.pimpl->data.totalram;
      j["data"]["freeram"] = d.pimpl->data.freeram;
      j["data"]["sharedram"] = d.pimpl->data.sharedram;
      j["data"]["bufferram"] = d.pimpl->data.bufferram;
      j["data"]["totalswap"] = d.pimpl->data.totalswap;
      j["data"]["freeswap"] = d.pimpl->data.freeswap;
      j["data"]["procs"] = d.pimpl->data.procs;
      j["data"]["totalhigh"] = d.pimpl->data.totalhigh;
      j["data"]["freehigh"] = d.pimpl->data.freehigh;
      j["data"]["mem_unit"] = d.pimpl->data.mem_unit;
      j["data"]["loads_scale"] = d.pimpl->data.loads_scale;
    }
  } // namespace data
} // namespace wassail