/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include <chrono>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <wassail/data/getrlimit.hpp>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#else
typedef uint64_t rlim_t;
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class getrlimit::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

      /*! \brief System data */
      struct {
        rlim_t core_hard;    /*!< The largest size (in bytes) core file that
                                  may be created */
        rlim_t core_soft;    /*!< The largest size (in bytes) core file that
                                  may be created */
        rlim_t cpu_hard;     /*!< The maximum amount of cpu time (in seconds)
                                  to be used by each process */
        rlim_t cpu_soft;     /*!< The maximum amount of cpu time (in seconds)
                                  to be used by each process */
        rlim_t data_hard;    /*!< The maximum size (in bytes) of the data
                                  segment for a process */
        rlim_t data_soft;    /*!< The maximum size (in bytes) of the data
                                  segment for a process */
        rlim_t fsize_hard;   /*!< The largest size (in bytes) file that may be
                                  created */
        rlim_t fsize_soft;   /*!< The largest size (in bytes) file that may be
                                  created */
        rlim_t memlock_hard; /*!< The maximum size (in bytes) which a process
                                  may lock into memory using the mlock(2)
                                  function */
        rlim_t memlock_soft; /*!< The maximum size (in bytes) which a process
                                  may lock into memory using the mlock(2)
                                  function */
        rlim_t nofile_hard;  /*!< The maximum number of open files for this
                                  process */
        rlim_t nofile_soft;  /*!< The maximum number of open files for this
                                  process */
        rlim_t nproc_hard;   /*!< The maximum number of simultaneous processes
                                  for this user id */
        rlim_t nproc_soft;   /*!< The maximum number of simultaneous processes
                                  for this user id */
        rlim_t rss_hard;     /*!< The maximum size (in bytes) to which a
                                  process's resident set size may grow */
        rlim_t rss_soft;     /*!< The maximum size (in bytes) to which a
                                  process's resident set size may grow */
        rlim_t stack_hard;   /*!< The maximum size (in bytes) of the stack
                                  segment for a process */
        rlim_t stack_soft;   /*!< The maximum size (in bytes) of the stack
                                  segment for a process */
      } data;                /*!< System data */

      /*! Private implementation of wassail::data::getrlimit::evaluate() */
      void evaluate(getrlimit &d, bool force);
    };

    getrlimit::getrlimit() : pimpl{std::make_unique<impl>()} {}
    getrlimit::~getrlimit() = default;
    getrlimit::getrlimit(getrlimit &&) = default;            // LCOV_EXCL_LINE
    getrlimit &getrlimit::operator=(getrlimit &&) = default; // LCOV_EXCL_LINE

    bool getrlimit::enabled() const {
#ifdef HAVE_GETRLIMIT
      return true;
#else
      return false;
#endif
    }

    void getrlimit::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void getrlimit::impl::evaluate(getrlimit &d, bool force) {
      if (force or not collected) {
#ifdef WITH_DATA_GETRLIMIT
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        /* collect data */
        int rv;
        struct rlimit rl;

        rv = ::getrlimit(RLIMIT_CORE, &rl);
        if (rv == 0) {
          data.core_hard = rl.rlim_max;
          data.core_soft = rl.rlim_cur;

          d.timestamp = std::chrono::system_clock::now();
          collected = true;
        }

        rv = ::getrlimit(RLIMIT_CPU, &rl);
        if (rv == 0) {
          data.cpu_hard = rl.rlim_max;
          data.cpu_soft = rl.rlim_cur;
        }

        rv = ::getrlimit(RLIMIT_DATA, &rl);
        if (rv == 0) {
          data.data_hard = rl.rlim_max;
          data.data_soft = rl.rlim_cur;
        }

        rv = ::getrlimit(RLIMIT_FSIZE, &rl);
        if (rv == 0) {
          data.fsize_hard = rl.rlim_max;
          data.fsize_soft = rl.rlim_cur;
        }

        rv = ::getrlimit(RLIMIT_MEMLOCK, &rl);
        if (rv == 0) {
          data.memlock_hard = rl.rlim_max;
          data.memlock_soft = rl.rlim_cur;
        }

        rv = ::getrlimit(RLIMIT_NOFILE, &rl);
        if (rv == 0) {
          data.nofile_hard = rl.rlim_max;
          data.nofile_soft = rl.rlim_cur;
        }

        rv = ::getrlimit(RLIMIT_NPROC, &rl);
        if (rv == 0) {
          data.nproc_hard = rl.rlim_max;
          data.nproc_soft = rl.rlim_cur;
        }

        rv = ::getrlimit(RLIMIT_RSS, &rl);
        if (rv == 0) {
          data.rss_hard = rl.rlim_max;
          data.rss_soft = rl.rlim_cur;
        }

        rv = ::getrlimit(RLIMIT_STACK, &rl);
        if (rv == 0) {
          data.stack_hard = rl.rlim_max;
          data.stack_soft = rl.rlim_cur;
        }
#else
        throw std::runtime_error("getrlimit() is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, getrlimit &d) {
      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      try {
        auto hard = j.at("data").at("hard");
        auto soft = j.at("data").at("soft");
        d.pimpl->data.core_hard = hard.at("core").get<uint64_t>();
        d.pimpl->data.core_soft = soft.at("core").get<uint64_t>();
        d.pimpl->data.cpu_hard = hard.at("cpu").get<uint64_t>();
        d.pimpl->data.cpu_soft = soft.at("cpu").get<uint64_t>();
        d.pimpl->data.data_hard = hard.at("data").get<uint64_t>();
        d.pimpl->data.data_soft = soft.at("data").get<uint64_t>();
        d.pimpl->data.fsize_hard = hard.at("fsize").get<uint64_t>();
        d.pimpl->data.fsize_soft = soft.at("fsize").get<uint64_t>();
        d.pimpl->data.memlock_hard = hard.at("memlock").get<uint64_t>();
        d.pimpl->data.memlock_soft = soft.at("memlock").get<uint64_t>();
        d.pimpl->data.nofile_hard = hard.at("nofile").get<uint64_t>();
        d.pimpl->data.nofile_soft = soft.at("nofile").get<uint64_t>();
        d.pimpl->data.nproc_hard = hard.at("nproc").get<uint64_t>();
        d.pimpl->data.nproc_soft = soft.at("nproc").get<uint64_t>();
        d.pimpl->data.rss_hard = hard.at("rss").get<uint64_t>();
        d.pimpl->data.rss_soft = soft.at("rss").get<uint64_t>();
        d.pimpl->data.stack_hard = hard.at("stack").get<uint64_t>();
        d.pimpl->data.stack_soft = soft.at("stack").get<uint64_t>();
      }
      catch (std::exception &e) {
        throw std::runtime_error(
            std::string("Unable to convert JSON string '") + j.dump() +
            std::string("' to object: ") + e.what());
      }
    }

    void to_json(json &j, const getrlimit &d) {
      j = dynamic_cast<const wassail::data::common &>(d);

      j["name"] = d.name();
      j["version"] = d.version();

      j["data"]["hard"]["core"] = d.pimpl->data.core_hard;
      j["data"]["soft"]["core"] = d.pimpl->data.core_soft;
      j["data"]["hard"]["cpu"] = d.pimpl->data.cpu_hard;
      j["data"]["soft"]["cpu"] = d.pimpl->data.cpu_soft;
      j["data"]["hard"]["data"] = d.pimpl->data.data_hard;
      j["data"]["soft"]["data"] = d.pimpl->data.data_soft;
      j["data"]["hard"]["fsize"] = d.pimpl->data.fsize_hard;
      j["data"]["soft"]["fsize"] = d.pimpl->data.fsize_soft;
      j["data"]["hard"]["memlock"] = d.pimpl->data.memlock_hard;
      j["data"]["soft"]["memlock"] = d.pimpl->data.memlock_soft;
      j["data"]["hard"]["nofile"] = d.pimpl->data.nofile_hard;
      j["data"]["soft"]["nofile"] = d.pimpl->data.nofile_soft;
      j["data"]["hard"]["nproc"] = d.pimpl->data.nproc_hard;
      j["data"]["soft"]["nproc"] = d.pimpl->data.nproc_soft;
      j["data"]["hard"]["rss"] = d.pimpl->data.rss_hard;
      j["data"]["soft"]["rss"] = d.pimpl->data.rss_soft;
      j["data"]["hard"]["stack"] = d.pimpl->data.stack_hard;
      j["data"]["soft"]["stack"] = d.pimpl->data.stack_soft;
    }
  } // namespace data
} // namespace wassail
