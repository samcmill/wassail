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

#include <chrono>
#include <wassail/checks/misc/environment.hpp>

TEST_CASE("environment unknown JSON") {
  json j = {{"name", "unknown"},
            {"data", {{"PATH", "/bin:/usr/bin"}}},
            {"timestamp", 0}};

  auto c = wassail::check::misc::environment("PATH", "/bin:/usr/bin");

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("environment invalid JSON") {
  json j = {{"foo", "bar"}};

  auto c = wassail::check::misc::environment("PATH", "/bin:/usr/bin");

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("environment basic JSON input (environment)") {
  json j = {{"name", "environment"},
            {"data",
             {{"USER", "ncognito"},
              {"HOME", "/home/ncognito"},
              {"SHELL", "/bin/bash"}}},
            {"timestamp", 0}};

  auto c1 = wassail::check::misc::environment("USER", "ncognito");
  auto r1 = c1.check(j);
  REQUIRE(r1->issue == wassail::result::issue_t::NO);

  auto c2 = wassail::check::misc::environment("SHELL", "/bin/tcsh");
  auto r2 = c2.check(j);
  REQUIRE(r2->issue == wassail::result::issue_t::YES);

  auto c3 = wassail::check::misc::environment("FOO", "bar");
  auto r3 = c3.check(j);
  REQUIRE(r3->issue == wassail::result::issue_t::YES);

  auto c4 = wassail::check::misc::environment("HOME", "^/home", true);
  auto r4 = c4.check(j);
  REQUIRE(r4->issue == wassail::result::issue_t::NO);

  auto c5 = wassail::check::misc::environment("HOME", "^/home/n$", true);
  auto r5 = c5.check(j);
  REQUIRE(r5->issue == wassail::result::issue_t::YES);

  auto c6 = wassail::check::misc::environment("FOO", "bar", true);
  auto r6 = c6.check(j);
  REQUIRE(r6->issue == wassail::result::issue_t::YES);
}
