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

namespace wassail {
  namespace check {
    namespace disk {
      std::shared_ptr<wassail::result> amount_free::check(const json &j) {
        bool found = false;
        uint64_t amount = 0.0;

        try {
          if (j.at("name").get<std::string>() == "getfsstat") {
            for (auto i : j.at("data").at("file_systems")) {
              if (i.at("mntonname").get<std::string>() == config.filesystem) {
                amount = i.at("bavail").get<uint64_t>() *
                         i.at("bsize").get<uint64_t>();
                found = true;
                break;
              }
            }
          }
          else if (j.at("name").get<std::string>() == "getmntent") {
            for (auto i : j.at("data").at("file_systems")) {
              if (i.at("dir") == config.filesystem) {
                amount = i.at("bavail").get<uint64_t>() *
                         i.at("bsize").get<uint64_t>();
                found = true;
                break;
              }
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
        r->format_brief(fmt_str.brief, config.filesystem);
        r->priority = result::priority_t::WARNING;

        if (not found) {
          r->issue = result::issue_t::MAYBE;
          r->format_detail(fmt_str.detail_maybe, config.filesystem);
        }
        else if (amount < config.amount) {
          r->issue = result::issue_t::YES;
          r->format_detail(fmt_str.detail_yes, amount, config.amount, "bytes");
        }
        else {
          r->issue = result::issue_t::NO;
          r->format_detail(fmt_str.detail_no, amount, config.amount, "bytes");
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
