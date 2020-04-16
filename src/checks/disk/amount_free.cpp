/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <exception>
#include <string>
#include <wassail/checks/disk/amount_free.hpp>
#include <wassail/common.hpp>

namespace wassail {
  namespace check {
    namespace disk {
      std::shared_ptr<wassail::result> amount_free::check(const json &j) {
        bool found = false;
        uint64_t amount = 0.0;

        if (j.value("name", "") == "getfsstat") {
          for (auto i : j.value(json::json_pointer("/data/file_systems"),
                                json::array())) {
            if (i.value("mntonname", "not_real") == config.filesystem) {
              amount = i.value("bavail", 0ULL) * i.value("bsize", 0ULL);
              found = true;
              break;
            }
          }
        }
        else if (j.value("name", "") == "getmntent") {
          for (auto i : j.value(json::json_pointer("/data/file_systems"),
                                json::array())) {
            if (i.value("dir", "not_real") == config.filesystem) {
              amount = i.value("bavail", 0ULL) * i.value("bsize", 0ULL);
              found = true;
              break;
            }
          }
        }
        else {
          throw std::runtime_error("Unrecognized JSON object");
        }

        auto r = make_result(j);
        r->brief = wassail::format(fmt_str.brief, config.filesystem, amount,
                                   config.amount, "bytes");
        r->priority = result::priority_t::WARNING;

        if (not found) {
          r->issue = result::issue_t::MAYBE;
          r->detail = wassail::format(fmt_str.detail_maybe, config.filesystem);
        }
        else if (amount < config.amount) {
          r->issue = result::issue_t::YES;
          r->detail = wassail::format(fmt_str.detail_yes, config.filesystem,
                                      amount, config.amount, "bytes");
        }
        else {
          r->issue = result::issue_t::NO;
          r->detail = wassail::format(fmt_str.detail_no, config.filesystem,
                                      amount, config.amount, "bytes");
        }

        return r;
      }

      std::shared_ptr<wassail::result>
      amount_free::check(wassail::data::getfsstat &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }

      std::shared_ptr<wassail::result>
      amount_free::check(wassail::data::getmntent &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace disk
  }   // namespace check
} // namespace wassail
