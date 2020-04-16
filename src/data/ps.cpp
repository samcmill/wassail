/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <string>
#include <wassail/data/ps.hpp>

namespace wassail {
  namespace data {
    void from_json(const json &j, ps &d) {
      if (j.value("version", 0) != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<shell_command &>(d));
    }

    void to_json(json &j, const ps &d) {
      j = dynamic_cast<const shell_command &>(d);

      std::regex re(
          "(\\S+)\\s+(\\d+)\\s+(\\d+\\.\\d+)\\s+(\\d+\\.\\d+)\\s+(\\d+)\\s+("
          "\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(.*?)\\s+(\\d+:\\d+[:\\.]\\d+)\\s+(.*?)"
          "\\n");

      std::string stdout = j["data"]["stdout"].get<std::string>();

      for (auto it = std::sregex_iterator(stdout.begin(), stdout.end(), re);
           it != std::sregex_iterator(); ++it) {
        json temp;

        temp["user"] = it->str(1);
        temp["pid"] = std::atoi(it->str(2).c_str());
        temp["pcpu"] = std::atof(it->str(3).c_str());
        temp["pmem"] = std::atof(it->str(4).c_str());
        temp["vsz"] = std::atoi(it->str(5).c_str());
        temp["rss"] = std::atoi(it->str(6).c_str());
        temp["tt"] = it->str(7);
        temp["state"] = it->str(8);
        temp["start"] = it->str(9);
        temp["time"] = it->str(10);
        temp["command"] = it->str(11);

        j["data"]["processes"].push_back(temp);
      }
    }
  } // namespace data
} // namespace wassail
