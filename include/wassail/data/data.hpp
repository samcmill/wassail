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
      /*! Flag to denote whether the data has been collected */
      bool collected_ = false;

      /*! Get the hostname of the system
       *  \return Hostname
       */
      std::string get_hostname() {
        char hostname[_POSIX_HOST_NAME_MAX];
        gethostname(hostname, _POSIX_HOST_NAME_MAX);
        return std::string(hostname);
      }

    protected:
      /*! Virtual destructor */
      virtual ~common() = default;

      /*! Common evaluate routine for all data sources */
      void evaluate_common();

      /*! Mutex to control parallel data source execution */
#if __cplusplus >= 201703L
      inline static std::shared_timed_mutex mutex;
#else
      static std::shared_timed_mutex mutex;
#endif

    public:
      /*! Query whether the data source has already been evaluated.
       *  \return true if the data source has already been evaluated,
       *          false otherwise
       */
      bool collected() { return collected_; }

      /*! Indicate whether the building block is enabled or not.  If not,
       *  evaluating the building block will throw and exception.
       *  \return true if the building block is available, false otherwise
       */
      virtual bool enabled() const = 0;

      /*! User ID of the user who invoked the data source. */
      uid_t uid = getuid();

      /*! Hostname of the system where the data source was invoked. */
      std::string hostname = get_hostname();

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

      /*! Timestamp corresponding to when the data source was invoked. */
      std::chrono::time_point<std::chrono::system_clock> timestamp;

      /*! Evaluate the data source.  The implementation should cache
       *  the result and return the cached data in subsequent calls
       *  rather than re-evaluating the data source.  The exception
       *  is when force is true, then the information should be
       *  collected again.
       *  \param[in] force Flag to specify whether the data should
       *                   be collected even if it already has been
       *                   evaluated.
       */
      virtual void evaluate(bool force = false) = 0;

      /*! Set the collected state to true */
      void set_collected();

      /*! JSON type conversion
       * \param[in] j
       * \param[in,out] d
       */
      friend void from_json(const json &j, wassail::data::common &d);

      /*! Populate the data source from the JSON representation of
       *  the data source.  This method should only be used with a pointer
       *  of type wassail::data::common.  Otherwise use the from_json
       *  friend method, e.g.,
       *  \code{.cpp}
       *  wassail::data::environment d = j; // preferred
       *
       *  <std::shared_ptr<wassail::data::common> dptr =
       *      std::make_shared<wassail::data::environment>();
       *  dptr->from_json(j); // only for base class pointer
       *  \endcode
       *  \param[in] j
       */
      virtual void from_json(const json &j) = 0;

      /*! JSON type conversion
       * \param[in,out] j
       * \param[in] d
       */
      friend void to_json(json &j, const wassail::data::common &d);

      /*! Return a JSON representation of the data source.
       *  This method should only be used with a pointer of type
       *  wassail::data::common.  Otherwise use the to_json friend
       *  method, e.g,
       *  \code{.cpp}
       *  auto d = wassail::data::environment();
       *  d.evaluate();
       *  json j1 = d; // preferred
       *  auto j2 = static_cast<json>(d); // alternative
       *
       *  std::shared_ptr<wassail::data::common> dptr =
       *      std::make_shared<wassail::data::environment>();
       *  dptr->evaluate();
       *  auto j3 = dptr->to_json(); // only for base class pointer
       *  \endcode
       */
      virtual json to_json() = 0;
    };

    /*! Evaluate the data source corresponding to the JSON input.
     *  \param[in] j Base JSON input.  At a minimum the "name" field
     *               must be specified.
     *  \return JSON representation of the evaluated data source if
     *          successful, or a nullptr if an error occurred during
     *          evaluation.
     */
    json evaluate(const json &j);

    /*! JSON type conversion */
    void from_json(const json &j, wassail::data::common &d);

    /*! JSON type conversion */
    void to_json(json &j, const wassail::data::common &d);
  } // namespace data
} // namespace wassail

#endif
