/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <exception>
#include <string>
#include <wassail/checks/cpu/core_count.hpp>

namespace wassail {
  namespace check {
    namespace cpu {
      std::shared_ptr<wassail::result> core_count::check(const json &j) {
        json::json_pointer key;

        if (j.value("name", "") == "sysconf") {
          key = json::json_pointer("/data/nprocessors_onln");
        }
        else if (j.value("name", "") == "sysctl") {
          key = json::json_pointer("/data/machdep/cpu/core_count");
        }
        else {
          throw std::runtime_error("Unrecognized JSON object");
        }

        /* check key exists */
        add_rule([&](json j) { return j.contains(key); });

        /* check observed value is equal to the reference value */
        add_rule([&](json j) { return j.value(key, 0) == config.num_cores; });

        return rules_engine::check(j, j.value(key, 0), config.num_cores);
      }

      std::shared_ptr<wassail::result>
      core_count::check(wassail::data::sysconf &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }

      std::shared_ptr<wassail::result>
      core_count::check(wassail::data::sysctl &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace cpu
  }   // namespace check
} // namespace wassail
