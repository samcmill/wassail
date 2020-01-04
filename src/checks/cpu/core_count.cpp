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
        try {
          if (j.at("name").get<std::string>() == "sysconf") {
            return compare::check(j,
                                  json::json_pointer("/data/nprocessors_onln"),
                                  std::equal_to<uint16_t>{}, config.num_cores);
          }
          else if (j.at("name").get<std::string>() == "sysctl") {
            return compare::check(
                j, json::json_pointer("/data/machdep/cpu/core_count"),
                std::equal_to<uint16_t>{}, config.num_cores);
          }
          else {
            throw std::runtime_error("Unrecognized JSON object");
          }
        }
        catch (std::exception &e) {
          throw std::runtime_error(std::string("Unable to perform check: ") +
                                   std::string(e.what()));
        }
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
