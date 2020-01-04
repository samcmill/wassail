/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <exception>
#include <string>
#include <wassail/checks/misc/load_average.hpp>

namespace wassail {
  namespace check {
    namespace misc {
      std::shared_ptr<wassail::result>
      load_average_generic::check(const json &j) {
        try {
          uint16_t minute = get_minute();

          if (j.at("name").get<std::string>() == "getloadavg") {
            std::string json_ptr = "/data/load" + std::to_string(minute);
            auto r = compare::check(j, json::json_pointer(json_ptr),
                                    std::less_equal<float>{}, config.load);
            r->format_brief(fmt_str.brief, minute);
            return r;
          }
          else if (j.at("name").get<std::string>() == "sysctl") {
            /* Load averages from sysctl are stored as unsigned long and must
             * be converted using the scale factor. */

            std::string json_ptr =
                "/data/vm/loadavg/load" + std::to_string(minute);
            float scale =
                j.at("data").at("vm").at("loadavg").at("fscale").get<float>();

            auto r = compare::check(j, json::json_pointer(json_ptr),
                                    std::less_equal<float>{}, config.load,
                                    [&](const auto &val) -> float {
                                      return static_cast<float>(val) / scale;
                                    });
            r->format_brief(fmt_str.brief, minute);
            return r;
          }
          else if (j.at("name").get<std::string>() == "sysinfo") {
            /* Load averages from sysinfo are stored as unsigned long and must
             * be converted using the scale factor. */

            std::string json_ptr = "/data/load" + std::to_string(minute);
            float scale = j.at("data").at("loads_scale").get<float>();

            auto r = compare::check(j, json::json_pointer(json_ptr),
                                    std::less_equal<float>{}, config.load,
                                    [&](const auto &val) -> float {
                                      return static_cast<float>(val) / scale;
                                    });
            r->format_brief(fmt_str.brief, minute);
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
      load_average_generic::check(wassail::data::getloadavg &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }

      std::shared_ptr<wassail::result>
      load_average_generic::check(wassail::data::sysctl &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }

      std::shared_ptr<wassail::result>
      load_average_generic::check(wassail::data::sysinfo &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace misc
  }   // namespace check
} // namespace wassail
