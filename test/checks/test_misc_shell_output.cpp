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
#include <wassail/checks/misc/shell_output.hpp>

TEST_CASE("shell_output unknown JSON") {
  json j = {
      {"name", "unknown"}, {"data", {{"stdout", "bar\n"}}}, {"timestamp", 0}};

  auto c = wassail::check::misc::shell_output("bar\n");

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("shell_output invalid JSON") {
  json j = {{"foo", "bar"}};

  auto c = wassail::check::misc::shell_output("bar\n");

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("shell_output basic JSON input (shell_command)") {
  json j = {{"name", "shell_command"},
            {"data", {{"command", "echo 'bar'"}, {"stdout", "bar\n"}}},
            {"timestamp", 0}};

  auto c1 = wassail::check::misc::shell_output("bar\n");
  auto r1 = c1.check(j);
  REQUIRE(r1->issue == wassail::result::issue_t::NO);
  REQUIRE(r1->detail ==
          "Observed output 'bar\n' matches expected output 'bar\n'");

  auto c2 = wassail::check::misc::shell_output("foo\n");
  auto r2 = c2.check(j);
  REQUIRE(r2->issue == wassail::result::issue_t::YES);
  REQUIRE(r2->detail ==
          "Observed output 'bar\n' does not match expected output 'foo\n'");

  auto c3 = wassail::check::misc::shell_output("^bar", true);
  auto r3 = c3.check(j);
  REQUIRE(r3->issue == wassail::result::issue_t::NO);

  auto c4 = wassail::check::misc::shell_output("^bar$", true);
  auto r4 = c4.check(j);
  REQUIRE(r4->issue == wassail::result::issue_t::YES);
}
