/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DATA_H
#define _WASSAIL_DATA_H

#include <cassert>
#include <chrono>
#include <limits.h>
#include <shared_mutex>
#include <string>
#include <unistd.h>
#include <wassail/common.hpp>
#include <wassail/json/json.hpp>

using json = nlohmann::json;

namespace wassail {
  namespace data {
    /*! \brief Parent class for all data source building blocks */
    class common {
    private:
      /*! Get the hostname of the system
       *  \return Hostname
       */
      std::string get_hostname() {
        char hostname[_POSIX_HOST_NAME_MAX];
        gethostname(hostname, _POSIX_HOST_NAME_MAX);
        return std::string(hostname);
      }

      /*! Indicate whether the building block is enabled or not.  If not,
       *  evaluating the building block will throw and exception.
       *  \return true if the building block is available, false otherwise
       */
      virtual bool enabled() const = 0;

      /*! Unique name for this building block
       *  \return Building block identifier
       */
      virtual std::string name() const = 0;

      /*! Interface version of the building block
       *  The version format is XXXYY where XXX is the major version
       *  and YY is the minor version, e.g. 1.01 would be 101.
       *  \return Building block interface version
       */
      virtual uint16_t version() const = 0;

    protected:
      /*! Virtual destructor */
      virtual ~common() = default;

      /*! Mutex to control parallel data source execution. */
#if __cplusplus >= 201703L
      inline static std::shared_timed_mutex mutex;
#else
      static std::shared_timed_mutex mutex;
#endif

    public:
      /*! Hostname of the system where the data source was invoked. */
      std::string hostname = get_hostname();

      /*! Timestamp corresponding to when the data source was invoked. */
      std::chrono::time_point<std::chrono::system_clock> timestamp;

      /*! User ID of the user who invoked the data source. */
      uid_t uid = getuid();

      /*! Evaluate the data source.  The implementation should cache
       *  the result and return the cached data in subsequent calls
       *  rather than re-evaluating the data source.  The exception
       *  is when force is true, then the information should be
       *  collected again.
       *  \param[in] force Flag to specify whether the data should
       *                   be collected even if it already has been
       *                   evaluated.
       */
      virtual void evaluate(bool force = false);

      /*! JSON type conversion
       * \param[in] j
       * \param[in,out] d
       */
      friend void from_json(const json &j, wassail::data::common &d);

      /*! JSON type conversion
       * \param[in,out] j
       * \param[in] d
       */
      friend void to_json(json &j, const wassail::data::common &d);
    };

    /*! JSON type conversion */
    void from_json(const json &j, wassail::data::common &d);

    /*! JSON type conversion */
    void to_json(json &j, const wassail::data::common &d);
  } // namespace data
} // namespace wassail

#endif
