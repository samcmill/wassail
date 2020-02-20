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
        try {
          if (j.at("name").get<std::string>() == "stat") {
            const uint16_t ref =
                j.at(json::json_pointer("/data/mode")).get<uint16_t>() & mask;
            const uint16_t val = config.mode;

            add_rule([&](json j) { return ref == val; });
            auto r = wassail::check::rules_engine::check(j);

            r->format_brief(
                fmt_str.brief,
                j.at(json::json_pointer("/data/path")).get<std::string>());

            if (r->issue == wassail::result::issue_t::YES) {
              r->format_detail(fmt_str.detail_yes, to_oct(val), to_oct(ref));
            }
            else if (r->issue == wassail::result::issue_t::NO) {
              r->format_detail(fmt_str.detail_no, to_oct(val), to_oct(ref));
            }

            return r;
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
      permissions::check(wassail::data::stat &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace file
  }   // namespace check
} // namespace wassail
