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

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

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
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not d.collected()) {
#ifdef WITH_DATA_GETRLIMIT
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        /* collect data */
        int rv;
        struct rlimit rl;

        rv = ::getrlimit(RLIMIT_CORE, &rl);
        if (rv == 0) {
          data.core_hard = rl.rlim_max;
          data.core_soft = rl.rlim_cur;

          d.common::evaluate_common();
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
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("name", "") != d.name()) {
        throw std::runtime_error("name mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      d.pimpl->data.core_hard =
          j.value(json::json_pointer("/data/hard/core"), 0ULL);
      d.pimpl->data.core_soft =
          j.value(json::json_pointer("/data/soft/core"), 0ULL);
      d.pimpl->data.cpu_hard =
          j.value(json::json_pointer("/data/hard/cpu"), 0ULL);
      d.pimpl->data.cpu_soft =
          j.value(json::json_pointer("/data/soft/cpu"), 0ULL);
      d.pimpl->data.data_hard =
          j.value(json::json_pointer("/data/hard/data"), 0ULL);
      d.pimpl->data.data_soft =
          j.value(json::json_pointer("/data/soft/data"), 0ULL);
      d.pimpl->data.fsize_hard =
          j.value(json::json_pointer("/data/hard/fsize"), 0ULL);
      d.pimpl->data.fsize_soft =
          j.value(json::json_pointer("/data/soft/fsize"), 0ULL);
      d.pimpl->data.memlock_hard =
          j.value(json::json_pointer("/data/hard/memlock"), 0ULL);
      d.pimpl->data.memlock_soft =
          j.value(json::json_pointer("/data/soft/memlock"), 0ULL);
      d.pimpl->data.nofile_hard =
          j.value(json::json_pointer("/data/hard/nofile"), 0ULL);
      d.pimpl->data.nofile_soft =
          j.value(json::json_pointer("/data/soft/nofile"), 0ULL);
      d.pimpl->data.nproc_hard =
          j.value(json::json_pointer("/data/hard/nproc"), 0ULL);
      d.pimpl->data.nproc_soft =
          j.value(json::json_pointer("/data/soft/nproc"), 0ULL);
      d.pimpl->data.rss_hard =
          j.value(json::json_pointer("/data/hard/rss"), 0ULL);
      d.pimpl->data.rss_soft =
          j.value(json::json_pointer("/data/soft/rss"), 0ULL);
      d.pimpl->data.stack_hard =
          j.value(json::json_pointer("/data/hard/stack"), 0ULL);
      d.pimpl->data.stack_soft =
          j.value(json::json_pointer("/data/soft/stack"), 0ULL);
    }

    void to_json(json &j, const getrlimit &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

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
