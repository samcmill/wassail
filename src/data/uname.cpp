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
#include <wassail/data/uname.hpp>

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class uname::impl {
    public:
      /*! \brief System data */
      struct {
        std::string sysname; /*!< name of the operating system implementation */
        std::string nodename; /*!< network name of this machine */
        std::string release;  /*!< release level of the operating system */
        std::string version;  /*!< version level of the operating system */
        std::string machine;  /*!< machine hardware platform */
      } data;                 /*!< System data */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::uname::evaluate() */
      void evaluate(uname &d, bool force);
    };

    uname::uname() : pimpl{std::make_unique<impl>()} {}
    uname::~uname() = default;
    uname::uname(uname &&) = default;            // LCOV_EXCL_LINE
    uname &uname::operator=(uname &&) = default; // LCOV_EXCL_LINE

    bool uname::enabled() const {
#ifdef HAVE_UNAME
      return true;
#else
      return false;
#endif
    }

    void uname::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void uname::impl::evaluate(uname &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not d.collected()) {
#ifdef HAVE_UNAME
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        /* collect data */
        struct utsname name;

        int rv = ::uname(&name);
        if (rv == 0) {
          data.sysname = name.sysname;
          data.nodename = name.nodename;
          data.release = name.release;
          data.version = name.version;
          data.machine = name.machine;

          d.common::evaluate(force);
        }
#else
        throw std::runtime_error("uname() is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, uname &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("name", "") != d.name()) {
        throw std::runtime_error("name mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      d.pimpl->data.sysname = j.value(json::json_pointer("/data/sysname"), "");
      d.pimpl->data.nodename =
          j.value(json::json_pointer("/data/nodename"), "");
      d.pimpl->data.release = j.value(json::json_pointer("/data/release"), "");
      d.pimpl->data.version = j.value(json::json_pointer("/data/version"), "");
      d.pimpl->data.machine = j.value(json::json_pointer("/data/machine"), "");
    }

    void to_json(json &j, const uname &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["sysname"] = d.pimpl->data.sysname;
      j["data"]["nodename"] = d.pimpl->data.nodename;
      j["data"]["release"] = d.pimpl->data.release;
      j["data"]["version"] = d.pimpl->data.version;
      j["data"]["machine"] = d.pimpl->data.machine;
      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
