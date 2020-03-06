/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "internal.hpp"

#include <exception>
#include <string>
#include <wassail/checks/disk/percent_free.hpp>
#include <wassail/common.hpp>

namespace wassail {
  namespace check {
    namespace disk {
      std::shared_ptr<wassail::result> percent_free::check(const json &j) {
        bool found = false;
        float percent = 0.0;

        try {
          if (j.at("name").get<std::string>() == "getfsstat") {
            for (auto i : j.at("data").at("file_systems")) {
              if (i.at("mntonname").get<std::string>() == config.filesystem) {
                percent = 100.0 * i.at("bavail").get<float>() /
                          i.at("blocks").get<float>();
                found = true;
                break;
              }
            }
          }
          else if (j.at("name").get<std::string>() == "getmntent") {
            for (auto i : j.at("data").at("file_systems")) {
              if (i.at("dir") == config.filesystem) {
                percent = 100.0 * i.at("bavail").get<float>() /
                          i.at("blocks").get<float>();
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
        r->brief = wassail::format(fmt_str.brief, config.filesystem, percent,
                                   config.percent);
        r->priority = result::priority_t::WARNING;

        if (not found) {
          r->issue = result::issue_t::MAYBE;
          r->detail = wassail::format(fmt_str.detail_maybe, config.filesystem);
        }
        else if (percent < config.percent) {
          r->issue = result::issue_t::YES;
          r->detail = wassail::format(fmt_str.detail_yes, config.filesystem,
                                      percent, config.percent);
        }
        else {
          r->issue = result::issue_t::NO;
          r->detail = wassail::format(fmt_str.detail_no, config.filesystem,
                                      percent, config.percent);
        }

        return r;
      }

      std::shared_ptr<wassail::result>
      percent_free::check(wassail::data::getfsstat &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }

      std::shared_ptr<wassail::result>
      percent_free::check(wassail::data::getmntent &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace disk
  }   // namespace check
} // namespace wassail
