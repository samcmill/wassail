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
#include <wassail/data/uname.hpp>

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class uname::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

      /*! \brief System data */
      struct {
        std::string sysname; /*!< name of the operating system implementation */
        std::string nodename; /*!< network name of this machine */
        std::string release;  /*!< release level of the operating system */
        std::string version;  /*!< version level of the operating system */
        std::string machine;  /*!< machine hardware platform */
      } data;                 /*!< System data */

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
      if (force or not collected) {
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

          d.timestamp = std::chrono::system_clock::now();
          collected = true;
        }
#else
        throw std::runtime_error("uname() is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, uname &d) {
      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      try {
        auto jdata = j.at("data");
        d.pimpl->data.sysname = jdata.at("sysname").get<std::string>();
        d.pimpl->data.nodename = jdata.at("nodename").get<std::string>();
        d.pimpl->data.release = jdata.at("release").get<std::string>();
        d.pimpl->data.version = jdata.at("version").get<std::string>();
        d.pimpl->data.machine = jdata.at("machine").get<std::string>();
      }
      catch (std::exception &e) {
        throw std::runtime_error(
            std::string("Unable to convert JSON string '") + j.dump() +
            std::string("' to object: ") + e.what());
      }
    }

    void to_json(json &j, const uname &d) {
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