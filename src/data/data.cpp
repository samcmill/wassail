/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <chrono>
#include <shared_mutex>
#include <string>
#include <unistd.h>
#include <wassail/common.hpp>
#include <wassail/data/data.hpp>
#include <wassail/json/json.hpp>

using json = nlohmann::json;

#if __cplusplus < 201703L
std::shared_timed_mutex wassail::data::common::mutex;
#endif

namespace wassail {
  namespace data {
    /*! JSON type conversion */
    void from_json(const json &j, wassail::data::common &d) {
      try {
        d.hostname = j.at("hostname").get<std::string>();
        d.timestamp = std::chrono::system_clock::from_time_t(
            j.at("timestamp").get<time_t>());
        d.uid = j.at("uid").get<uid_t>();
      }
      /* LCOV_EXCL_START */
      catch (std::exception &e) {
        wassail::internal::logger()->error("Unable to convert JSON string '" +
                                           j.dump() +
                                           "' to object: " + e.what());
      }
      /* LCOV_EXCL_STOP */
    }

    /*! JSON type conversion */
    void to_json(json &j, const wassail::data::common &d) {
      j["hostname"] = d.hostname;
      j["timestamp"] = std::chrono::system_clock::to_time_t(d.timestamp);
      j["uid"] = d.uid;
    }
  } // namespace data
} // namespace wassail
