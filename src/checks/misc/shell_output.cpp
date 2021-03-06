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
#include <wassail/checks/misc/shell_output.hpp>

namespace wassail {
  namespace check {
    namespace misc {
      std::shared_ptr<wassail::result> shell_output::check(const json &j) {
        /* set rules to check shell output */
        auto set_rules = [&]() {
          /* check shell command stdout key exists */
          add_rule([](json j) {
            return j.contains(json::json_pointer("/data/stdout"));
          });

          if (config.regex) {
            /* check output matches the reference regex */
            add_rule([&](json j) {
              return std::regex_search(
                  j.at(json::json_pointer("/data/stdout")).get<std::string>(),
                  std::regex(config.output));
            });
          }
          else {
            /* check output is equal to the reference output */
            add_rule([&](json j) {
              return j.value(json::json_pointer("/data/stdout"), "") ==
                     config.output;
            });
          }
        };

        if (j.value("name", "") == "remote_shell_command") {
          set_rules();

          auto r = wassail::make_result(j);
          r->brief = fmt_str.brief;

          wassail::internal::for_each(
              j["data"].begin(), j["data"].end(), [&](json j) {
                auto child = rules_engine::check(
                    j, j.value(json::json_pointer("/data/stdout"), ""),
                    config.output);
                r->add_child(child);
              });

          r->propagate();
          return r;
        }
        else if (j.value("name", "") == "shell_command") {
          set_rules();

          return rules_engine::check(
              j, j.value(json::json_pointer("/data/stdout"), ""),
              config.output);
        }
        else {
          throw std::runtime_error("Unrecognized JSON object");
        }
      }

      std::shared_ptr<wassail::result>
      shell_output::check(wassail::data::remote_shell_command &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }

      std::shared_ptr<wassail::result>
      shell_output::check(wassail::data::shell_command &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace misc
  }   // namespace check
} // namespace wassail
