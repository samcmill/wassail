/* Copyright (c) 2018-2021 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "internal.hpp"

#include <exception>
#include <string>
#include <wassail/data/environment.hpp>
#include <wassail/data/getcpuid.hpp>
#include <wassail/data/getfsstat.hpp>
#include <wassail/data/getloadavg.hpp>
#include <wassail/data/getmntent.hpp>
#include <wassail/data/getrlimit.hpp>
#include <wassail/data/mpirun.hpp>
#include <wassail/data/nvml.hpp>
#include <wassail/data/osu_micro_benchmarks.hpp>
#include <wassail/data/pciaccess.hpp>
#include <wassail/data/pciutils.hpp>
#include <wassail/data/ps.hpp>
#include <wassail/data/remote_shell_command.hpp>
#include <wassail/data/shell_command.hpp>
#include <wassail/data/stat.hpp>
#include <wassail/data/stream.hpp>
#include <wassail/data/sysconf.hpp>
#include <wassail/data/sysctl.hpp>
#include <wassail/data/sysinfo.hpp>
#include <wassail/data/udev.hpp>
#include <wassail/data/umad.hpp>
#include <wassail/data/uname.hpp>
#include <wassail/json/json.hpp>

using json = nlohmann::json;

namespace wassail {
  namespace data {
    json evaluate(const json &j) {
      const std::string name = j.value("name", "");

      auto evaluate_ = [](auto &d) {
        if (d.enabled()) {
          try {
            d.evaluate();
            return static_cast<json>(d);
          }
          catch (std::exception &e) {
            wassail::internal::logger()->error(
                "error evaluating data source: {}", e.what());
          }
        }
        else {
          wassail::internal::logger()->warn("data source {} not enabled",
                                            d.name());
        }

        return static_cast<json>(nullptr);
      };

      if (name == "environment") {
        wassail::data::environment d = j;
        return evaluate_(d);
      }
      else if (name == "getcpuid") {
        wassail::data::getcpuid d = j;
        return evaluate_(d);
      }
      else if (name == "getfsstat") {
        wassail::data::getfsstat d = j;
        return evaluate_(d);
      }
      else if (name == "getloadavg") {
        wassail::data::getloadavg d = j;
        return evaluate_(d);
      }
      else if (name == "getmntent") {
        wassail::data::getmntent d = j;
        return evaluate_(d);
      }
      else if (name == "getrlimit") {
        wassail::data::getrlimit d = j;
        return evaluate_(d);
      }
      else if (name == "mpirun") {
        wassail::data::mpirun d = j;
        return evaluate_(d);
      }
      else if (name == "nvml") {
        wassail::data::nvml d = j;
        return evaluate_(d);
      }
      else if (name == "osu_micro_benchmarks") {
        wassail::data::osu_micro_benchmarks d = j;
        return evaluate_(d);
      }
      else if (name == "pciaccess") {
        wassail::data::pciaccess d = j;
        return evaluate_(d);
      }
      else if (name == "pciutils") {
        wassail::data::pciutils d = j;
        return evaluate_(d);
      }
      else if (name == "ps") {
        wassail::data::ps d = j;
        return evaluate_(d);
      }
      else if (name == "remote_shell_command") {
        wassail::data::remote_shell_command d = j;
        return evaluate_(d);
      }
      else if (name == "shell_command") {
        wassail::data::shell_command d = j;
        return evaluate_(d);
      }
      else if (name == "stat") {
        wassail::data::stat d = j;
        return evaluate_(d);
      }
      else if (name == "stream") {
        wassail::data::stream d = j;
        return evaluate_(d);
      }
      else if (name == "sysconf") {
        wassail::data::sysconf d = j;
        return evaluate_(d);
      }
      else if (name == "sysctl") {
        wassail::data::sysctl d = j;
        return evaluate_(d);
      }
      else if (name == "sysinfo") {
        wassail::data::sysinfo d = j;
        return evaluate_(d);
      }
      else if (name == "udev") {
        wassail::data::udev d = j;
        return evaluate_(d);
      }
      else if (name == "umad") {
        wassail::data::umad d = j;
        return evaluate_(d);
      }
      else if (name == "uname") {
        wassail::data::uname d = j;
        return evaluate_(d);
      }
      else {
        wassail::internal::logger()->error("unrecognized data source: {}",
                                           name);
        return static_cast<json>(nullptr);
      }
    }
  } // namespace data
} // namespace wassail
