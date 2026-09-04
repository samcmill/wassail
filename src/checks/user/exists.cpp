/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "internal.hpp"

#include <exception>
#include <string>
#include <wassail/checks/user/exists.hpp>

namespace wassail {
  namespace check {
    namespace user {
      std::shared_ptr<wassail::result> exists::check(const json &j) {
        if (j.value("name", "") == "getpwent") {
          /* check users key exists */
          add_rule([](json j) {
            return j.contains(json::json_pointer("/data/users"));
          });

          /* check users key is an array */
          add_rule([](json j) { return j["data"]["users"].is_array(); });

          /* check user is in the list of users */
          add_rule([&](json j) {
            for (auto i :
                 j.value(json::json_pointer("/data/users"), json::array())) {
              if (i.value("pw_name", "") == config.username) {
                return true;
              }
            }
            return false;
          });

          return rules_engine::check(j, config.username);
        }
        else {
          throw std::runtime_error("Unrecognized JSON object");
        }
      }

      std::shared_ptr<wassail::result>
      exists::check(wassail::data::getpwent &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace user
  } // namespace check
} // namespace wassail
