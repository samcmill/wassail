/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "internal.hpp"

#include <algorithm>
#include <exception>
#include <string>
#include <wassail/checks/memory/physical_size.hpp>

namespace wassail {
  namespace check {
    namespace memory {
      std::shared_ptr<wassail::result> physical_size::check(const json &j) {
        uint64_t physical = 0;

        if (j.value("name", "") == "sysconf") {
          physical =
              j.value(json::json_pointer("/data/phys_pages"),
                      std::uint64_t(0)) *
              j.value(json::json_pointer("/data/page_size"), std::uint64_t(0));
        }
        else if (j.value("name", "") == "sysctl") {
          physical =
              j.value(json::json_pointer("/data/hw/memsize"), std::uint64_t(0));
        }
        else if (j.value("name", "") == "sysinfo") {
          physical =
              j.value(json::json_pointer("/data/totalram"), std::uint64_t(0)) *
              j.value(json::json_pointer("/data/mem_unit"), std::uint64_t(0));
        }
        else {
          throw std::runtime_error("Unrecognized JSON object");
        }

        add_rule([&](json j) {
          return std::max(physical, config.mem_size) -
                     std::min(physical, config.mem_size) <=
                 config.tolerance;
        });

        return rules_engine::check(j, physical, config.mem_size,
                                   config.tolerance, "bytes");
      }

      std::shared_ptr<wassail::result>
      physical_size::check(wassail::data::sysconf &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }

      std::shared_ptr<wassail::result>
      physical_size::check(wassail::data::sysctl &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }

      std::shared_ptr<wassail::result>
      physical_size::check(wassail::data::sysinfo &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace memory
  }   // namespace check
} // namespace wassail
