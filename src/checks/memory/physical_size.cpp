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

        try {
          if (j.at("name").get<std::string>() == "sysconf") {
            physical = j.at("data").at("phys_pages").get<uint64_t>() *
                       j.at("data").at("page_size").get<uint64_t>();
          }
          else if (j.at("name").get<std::string>() == "sysctl") {
            physical = j.at("data").at("hw").at("memsize").get<uint64_t>();
          }
          else if (j.at("name").get<std::string>() == "sysinfo") {
            physical = j.at("data").at("totalram").get<uint64_t>() *
                       j.at("data").at("mem_unit").get<uint64_t>();
          }
          else {
            throw std::runtime_error("Unrecognized JSON object");
          }
        }
        catch (std::exception &e) {
          throw std::runtime_error(std::string("Unable to perform check: ") +
                                   std::string(e.what()));
        }

        auto r = make_result(j);
        r->format_brief(fmt_str.brief);
        r->priority = result::priority_t::WARNING;

        if (std::max(physical, config.mem_size) -
                std::min(physical, config.mem_size) >
            config.tolerance) {
          r->issue = result::issue_t::YES;
          r->format_detail(fmt_str.detail_yes, physical, config.mem_size,
                           config.tolerance, "bytes");
        }
        else {
          r->issue = result::issue_t::NO;
          r->format_detail(fmt_str.detail_no, physical, config.mem_size,
                           config.tolerance, "bytes");
        }

        return r;
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
