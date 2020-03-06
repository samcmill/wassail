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
      std::shared_ptr<wassail::result> load_average::check(const json &j) {
        float load = 99.9;

        if (j.value("name", "") == "getloadavg") {
          if (config.minute == minute_t::ONE) {
            /* 1 minute load average */
            add_rule([](json j) {
              return j.contains(json::json_pointer("/data/load1"));
            });
            load = j.value(json::json_pointer("/data/load1"), 99.9);
          }
          else if (config.minute == minute_t::FIVE) {
            /* 5 minute load average */
            add_rule([](json j) {
              return j.contains(json::json_pointer("/data/load5"));
            });
            load = j.value(json::json_pointer("/data/load5"), 99.9);
          }
          else if (config.minute == minute_t::FIFTEEN) {
            /* 15 minute load average */
            add_rule([](json j) {
              return j.contains(json::json_pointer("/data/load15"));
            });
            load = j.value(json::json_pointer("/data/load15"), 99.9);
          }
        }
        else if (j.value("name", "") == "sysctl") {
          /* Load averages from sysctl are stored as unsigned long and must
           * be converted using the scale factor. */

          add_rule([](json j) {
            return j.contains(json::json_pointer("/data/vm/loadavg/fscale"));
          });
          float scale =
              j.value(json::json_pointer("/data/vm/loadavg/fscale"), 1.0);

          if (config.minute == minute_t::ONE) {
            /* 1 minute load average */
            load = j.value(json::json_pointer("/data/vm/loadavg/load1"), 99.9) /
                   scale;
          }
          else if (config.minute == minute_t::FIVE) {
            /* 5 minute load average */
            load = j.value(json::json_pointer("/data/vm/loadavg/load5"), 99.9) /
                   scale;
          }
          else if (config.minute == minute_t::FIFTEEN) {
            /* 15 minute load average */
            load =
                j.value(json::json_pointer("/data/vm/loadavg/load15"), 99.9) /
                scale;
          }
        }
        else if (j.value("name", "") == "sysinfo") {
          /* Load averages from sysinfo are stored as unsigned long and must
           * be converted using the scale factor. */

          add_rule([](json j) {
            return j.contains(json::json_pointer("/data/loads_scale"));
          });
          float scale = j.value(json::json_pointer("/data/loads_scale"), 0.0);

          if (config.minute == minute_t::ONE) {
            /* 1 minute load average */
            load = j.value(json::json_pointer("/data/load1"), 99.9) / scale;
          }
          else if (config.minute == minute_t::FIVE) {
            /* 5 minute load average */
            load = j.value(json::json_pointer("/data/load5"), 99.9) / scale;
          }
          else if (config.minute == minute_t::FIFTEEN) {
            /* 15 minute load average */
            load = j.value(json::json_pointer("/data/load15"), 99.9) / scale;
          }
        }
        else {
          throw std::runtime_error("Unrecognized JSON object");
        }

        add_rule([&](json j) { return load <= config.load; });

        return rules_engine::check(j, static_cast<int>(config.minute), load,
                                   config.load);
      }

      std::shared_ptr<wassail::result>
      load_average::check(wassail::data::getloadavg &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }

      std::shared_ptr<wassail::result>
      load_average::check(wassail::data::sysctl &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }

      std::shared_ptr<wassail::result>
      load_average::check(wassail::data::sysinfo &d) {
        d.evaluate();
        return check(static_cast<json>(d));
      }
    } // namespace misc
  }   // namespace check
} // namespace wassail
