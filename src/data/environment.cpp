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
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <wassail/data/environment.hpp>

#ifdef HAVE_EXTERN_ENVIRON
extern char **environ;
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class environment::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

      /*! \brief System data */
      struct {
        std::map<std::string, std::string> envvar;
      } data; /*!< System data */

      /*! Private implementation of wassail::data::environment::evaluate() */
      void evaluate(environment &d, bool force);
    };

    environment::environment() : pimpl{std::make_unique<impl>()} {}
    environment::~environment() = default;
    environment::environment(environment &&) = default; // LCOV_EXCL_LINE
    environment &environment::
    operator=(environment &&) = default; // LCOV_EXCL_LINE

    bool environment::enabled() const {
#ifdef WITH_DATA_ENVIRONMENT
      return true;
#else
      return false;
#endif
    }

    void environment::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void environment::impl::evaluate(environment &d, bool force) {
      if (force or not collected) {
#ifdef WITH_DATA_ENVIRONMENT
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        /* collect data */
        char **e = environ;

        if (e) {
          while (*e) {
            std::string s(*e++);

            /* split on first "=" */
            size_t pos = s.find("=");
            if (pos != std::string::npos) {
              std::string variable = s.substr(0, pos);
              std::string value = s.substr(pos + 1, std::string::npos);
              data.envvar[variable] = value;
            }
            else {
              wassail::internal::logger()->warn(
                  "Malformed environment string: '{}'", s);
            }
          }

          d.timestamp = std::chrono::system_clock::now();
        }
#else
        throw std::runtime_error("environment data source is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, environment &d) {
      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      try {
        auto jdata = j.at("data");
        for (auto &e : jdata.items()) {
          d.pimpl->data.envvar[e.key()] = e.value();
        }
      }
      catch (std::exception &e) {
        throw std::runtime_error("Unable to convert JSON string '" + j.dump() +
                                 "' to object: " + e.what());
      }
    }

    void to_json(json &j, const environment &d) {
      j = dynamic_cast<const wassail::data::common &>(d);

      for (auto &e : d.pimpl->data.envvar) {
        j["data"][e.first] = e.second;
      }

      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
