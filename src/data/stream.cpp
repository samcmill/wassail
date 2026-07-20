/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iterator>
#include <regex>
#include <stdexcept>
#include <string>
#include <wassail/data/stream.hpp>

namespace wassail {
  namespace data {
    void from_json(const json &j, stream &d) {
      if (j.value("name", "") != d.name()) {
        throw std::runtime_error("name mismatch");
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
        j["data"]["copy"] = std::stod(it->str(1));
        j["data"]["scale"] = std::stod(it->str(2));
        j["data"]["add"] = std::stod(it->str(3));
        j["data"]["triad"] = std::stod(it->str(4));
      }
    }
  } // namespace data
} // namespace wassail
