/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* The operator<< overloads must be included before the catch header */
#include "tostring.h"

#define CATCH_CONFIG_MAIN
#include "3rdparty/catch/catch.hpp"
#include "3rdparty/catch/catch_reporter_automake.hpp"

#include <string>

#include "config.h"
#include <wassail/common.hpp>

TEST_CASE("version") {
  std::string version = wassail::version();

  uint16_t major = wassail::version_major();
  uint16_t minor = wassail::version_minor();
  uint16_t micro = wassail::version_micro();

  REQUIRE(version.size() > 0);
  REQUIRE(version == PACKAGE_VERSION);
  REQUIRE(major == VERSION_MAJOR);
  REQUIRE(minor == VERSION_MINOR);
  REQUIRE(micro == VERSION_MICRO);

  std::string version2 = std::to_string(major) + "." + std::to_string(minor) +
                         "." + std::to_string(micro);
  REQUIRE(version == version2);
}
