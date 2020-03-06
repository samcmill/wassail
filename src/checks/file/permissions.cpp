/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "internal.hpp"

#include <exception>
#include <iomanip>
#include <sstream>
#include <string>
#include <wassail/checks/file/permissions.hpp>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
const mode_t mask = S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX;
#else
const uint16_t mask = 07777;
#endif

std::string to_oct(uint16_t i) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(4) << std::oct << i;
  return ss.str();
}

namespace wassail {
  namespace check {
    namespace file {
      std::shared_ptr<wassail::result> permissions::check(const json &j) {
        if (j.value("name", "") == "stat") {
          const uint16_t val =
              j.value(json::json_pointer("/data/mode"), 0) & mask;

          /* check mode key exists */
          add_rule([](json j) {
            return j.contains(json::json_pointer("/data/mode"));
          });

          /* check observed value is equal to the reference value */
          add_rule([&](json j) { return val == config.mode; });

          return rules_engine::check(
              j, j.value(json::json_pointer("/data/path"), ""), to_oct(val),
              to_oct(config.mode));
        }
        else {
          throw std::runtime_error("Unrecognized JSON object");
        }
      }

      std::shared_ptr<wassail::result>
      permissions::check(wassail::data::stat &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace file
  }   // namespace check
} // namespace wassail
