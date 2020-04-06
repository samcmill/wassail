/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* This sample calls each data building block.  If the building block
 * is enabled, then the result is printed as JSON to stdout.  Otherwise,
 * an error message is printed to stderr.
 */

#include <iostream>
#include <wassail/wassail.hpp>

template <typename T>
int dump(T data) {
  try {
    data.evaluate();
    json j = data;
    std::cout << j.dump(-1, ' ', false, json::error_handler_t::replace)
              << std::endl;
    return 0;
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}

int main(int argc, char **argv) {
  wassail::initialize();

  dump(wassail::data::environment());
  dump(wassail::data::getcpuid());
  dump(wassail::data::getfsstat());
  dump(wassail::data::getloadavg());
  dump(wassail::data::getmntent());
  dump(wassail::data::getrlimit());
  dump(wassail::data::nvml());
  dump(wassail::data::pciaccess());
  dump(wassail::data::pciutils());
  dump(wassail::data::ps());
  dump(wassail::data::shell_command("uptime"));
  dump(wassail::data::stat("/tmp"));
  dump(wassail::data::stream());
  dump(wassail::data::sysconf());
  dump(wassail::data::sysctl());
  dump(wassail::data::sysinfo());
  dump(wassail::data::umad());
  dump(wassail::data::uname());

  exit(0);
}
