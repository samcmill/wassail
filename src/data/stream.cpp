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
#include <wassail/data/stream.hpp>

namespace wassail {
  namespace data {
    void from_json(const json &j, stream &d) {
      if (j.value("version", 0) != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<shell_command &>(d));
    }

    void to_json(json &j, const stream &d) {
      j = dynamic_cast<const shell_command &>(d);

      std::regex re("Copy:\\s+(\\d+\\.\\d+).*?\\n"
                    "Scale:\\s+(\\d+\\.\\d+).*?\\n"
                    "Add:\\s+(\\d+\\.\\d+).*?\\n"
                    "Triad:\\s+(\\d+\\.\\d+).*?\\n");

      std::string stdout = j["data"]["stdout"].get<std::string>();

      for (auto it = std::sregex_iterator(stdout.begin(), stdout.end(), re);
           it != std::sregex_iterator(); ++it) {
        j["data"]["copy"] = std::atof(it->str(1).c_str());
        j["data"]["scale"] = std::atof(it->str(2).c_str());
        j["data"]["add"] = std::atof(it->str(3).c_str());
        j["data"]["triad"] = std::atof(it->str(4).c_str());
      }
    }
  } // namespace data
} // namespace wassail
