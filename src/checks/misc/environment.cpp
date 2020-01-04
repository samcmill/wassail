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
        bool found = false;
        std::string value;

        try {
          if (j.at("name").get<std::string>() == "environment") {
            auto jdata = j.at("data");

            if (jdata.contains(config.variable)) {
              value = j.at("data").at(config.variable).get<std::string>();
              found = true;
            }
            else {
              value = "";
              found = false;
            }
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
        r->format_brief(fmt_str.brief, config.variable);
        r->priority = result::priority_t::WARNING;

        if (not found or
            (config.regex and
             not regex_search(value, std::regex(config.value))) or
            (not config.regex and value != config.value)) {
          r->issue = result::issue_t::YES;
          r->format_detail(fmt_str.detail_yes, value, config.value);
        }
        else {
          r->issue = result::issue_t::NO;
          r->format_detail(fmt_str.detail_no, value, config.value);
        }

        return r;
      }

      std::shared_ptr<wassail::result>
      environment::check(wassail::data::environment &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace misc
  }   // namespace check
} // namespace wassail
