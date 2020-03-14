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

TEST_CASE("shell_oput basic JSON input (remote_shell_command)") {
  auto j = R"(
    {
      "data": [
        {
          "data": {
            "command": "uptime",
            "elapsed": 0.011148364,
            "returncode": 0,
            "stderr": "",
            "stdout": "bar 1\n"
          },
          "hostname": "node1",
          "timestamp": 152894836,
          "uid": 99
        },
        {
          "data": {
            "command": "uptime",
            "elapsed": 0.011124915,
            "returncode": 0,
            "stderr": "",
            "stdout": "bar 2\n"
          },
          "hostname": "node2",
          "timestamp": 152894836,
          "uid": 99
        }
      ],
      "hostname": "localhost.local",
      "name": "remote_shell_command",
      "timestamp": 1528948436,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::remote_shell_command d = j;

  auto c1 = wassail::check::misc::shell_output("bar\n");
  auto r1 = c1.check(j);
  REQUIRE(r1->issue == wassail::result::issue_t::YES);
  REQUIRE(r1->children.size() == 2);
  REQUIRE(r1->children[0]->issue == wassail::result::issue_t::YES);
  REQUIRE(r1->children[0]->detail ==
          "Observed output 'bar 1\n' does not match expected output 'bar\n'");
  REQUIRE(r1->children[1]->issue == wassail::result::issue_t::YES);

  auto c2 = wassail::check::misc::shell_output("bar 2\n");
  auto r2 = c2.check(j);
  REQUIRE(r2->issue == wassail::result::issue_t::YES);
  REQUIRE(r2->children.size() == 2);

  /* The child order is not guaranteed to be deterministic, so look for each
   * node specifically. */
  for (auto &child : r2->children) {
    if (child->system_id[0] == "node1") {
      REQUIRE(child->issue == wassail::result::issue_t::YES);
    }
    else if (child->system_id[0] == "node2") {
      REQUIRE(child->issue == wassail::result::issue_t::NO);
    }
    else {
      /* should never reach here */
      REQUIRE(false);
    }
  }

  auto c3 = wassail::check::misc::shell_output(R"(^bar \d\n$)", true);
  auto r3 = c3.check(j);
  REQUIRE(r3->issue == wassail::result::issue_t::NO);
  REQUIRE(r3->children.size() == 2);
}
