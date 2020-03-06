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

TEST_CASE("format") {
  REQUIRE(wassail::format("{0} {1}", "a", "b") == "a b");
  REQUIRE(wassail::format("{0:s} {1:.2f} {2:d}", "a", 1.23, 4) == "a 1.23 4");
}
