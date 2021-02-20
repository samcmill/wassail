/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <chrono>
#include <limits>
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
    void common::evaluate(bool force) {
      hostname = get_hostname();
      timestamp = std::chrono::system_clock::now();
      uid = getuid();
      set_collected();
    }

    void common::set_collected() { collected_ = true; }

    /*! JSON type conversion */
    void from_json(const json &j, wassail::data::common &d) {
      if (j.contains("data")) {
        d.set_collected();
      }

      d.hostname = j.value("hostname", "");
      d.timestamp = std::chrono::system_clock::from_time_t(
          j.value("timestamp", static_cast<time_t>(0)));
      d.uid =
          j.value("uid", static_cast<uid_t>(std::numeric_limits<uid_t>::max()));
    }

    /*! JSON type conversion */
    void to_json(json &j, const wassail::data::common &d) {
      j["hostname"] = d.hostname;
      j["timestamp"] = std::chrono::system_clock::to_time_t(d.timestamp);
      j["uid"] = d.uid;
    }
  } // namespace data
} // namespace wassail
