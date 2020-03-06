/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <functional>
#include <vector>
#include <wassail/checks/rules_engine.hpp>
#include <wassail/common.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

using json = nlohmann::json;

namespace wassail {
  namespace check {
    void rules_engine::add_rule(const rules_t &rule) { rules.push_back(rule); }

    std::shared_ptr<wassail::result> rules_engine::check(const json &j) {
      std::shared_ptr<wassail::result> r = make_result(j);
      r->brief = fmt_str.brief;

      try {
        /* All rules must be true. */
        if (std::all_of(rules.cbegin(), rules.cend(),
                        [&](rules_t r) { return (r)(j); })) {
          r->issue = result::issue_t::NO;
          r->priority = result::priority_t::INFO;
          r->detail = fmt_str.detail_no;
        }
        else {
          r->issue = result::issue_t::YES;
          r->priority = result::priority_t::WARNING;
          r->detail = fmt_str.detail_yes;
        }
      }
      catch (std::exception &e) {
        logger(log_level::warn, e.what() + std::string(": ") + j.dump());
        r->issue = result::issue_t::MAYBE;
        r->detail = wassail::format(fmt_str.detail_maybe, e.what());
      }

      return r;
    }
  } // namespace check
} // namespace wassail
