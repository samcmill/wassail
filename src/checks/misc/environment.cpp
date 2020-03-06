/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "internal.hpp"

#include <exception>
#include <regex>
#include <string>
#include <wassail/checks/misc/environment.hpp>

namespace wassail {
  namespace check {
    namespace misc {
      std::shared_ptr<wassail::result> environment::check(const json &j) {
        if (j.value("name", "") == "environment") {
          /* check environment variable key exists */
          add_rule([&](json j) {
            return j.contains(json::json_pointer("/data/" + config.variable));
          });

          if (config.regex) {
            /* check environment variable matches the reference regex */
            add_rule([&](json j) {
              return std::regex_search(
                  j.at(json::json_pointer("/data/" + config.variable))
                      .get<std::string>(),
                  std::regex(config.value));
            });
          }
          else {
            /* check environment variable is equal to the reference value */
            add_rule([&](json j) {
              return j["data"].value(config.variable, "") == config.value;
            });
          }

          return rules_engine::check(
              j, config.variable,
              j.value(json::json_pointer("/data/" + config.variable), ""),
              config.value);
        }
        else {
          throw std::runtime_error("Unrecognized JSON object");
        }
      }

      std::shared_ptr<wassail::result>
      environment::check(wassail::data::environment &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace misc
  }   // namespace check
} // namespace wassail
