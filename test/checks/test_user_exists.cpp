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
#include <wassail/checks/user/exists.hpp>

using json = nlohmann::json;

TEST_CASE("exists unknown JSON") {
  auto j = R"(
    {
      "data": {
        "users": [
          {
            "pw_dir": "/var/root",
            "pw_gid": 0,
            "pw_name": "root",
            "pw_shell": "/bin/sh",
            "pw_uid":0
          }
        ]
      },
      "name": "unknown",
      "timestamp": 0
    }
  )"_json;

  auto c = wassail::check::user::exists("root");

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("exists invalid JSON") {
  json j1 = {{"foo", "bar"}};
  auto j2 = R"(
    {
      "data": {
        "users": 42 
      },
      "name": "getpwent",
      "timestamp": 0
    }
  )"_json;

  auto c = wassail::check::user::exists("root");

  REQUIRE_THROWS(c.check(j1));

  auto r2 = c.check(j2);
  REQUIRE(r2->issue == wassail::result::issue_t::YES);
}

TEST_CASE("exists basic JSON") {
  auto jin = R"(
    {
      "data": {
        "users": [
          {
            "pw_dir": "/var/root",
            "pw_gid": 0,
            "pw_name": "root",
            "pw_shell": "/bin/sh",
            "pw_uid":0
          },
          {
           "pw_dir": "/var/empty",
           "pw_gid": 4294967294,
           "pw_name": "nobody",
           "pw_shell":"/usr/bin/false",
           "pw_uid": 4294967294
          },
          {
           "pw_dir":"/Users/scott",
           "pw_gid":20,
           "pw_name":"scott",
           "pw_shell":"/bin/bash",
           "pw_uid":501}
        ]
      },
      "hostname": "localhost.local",
      "name": "getpwent",
      "timestamp": 1788391957,
      "uid": 501,
      "version": 100
    }
  )"_json;

  auto c1 = wassail::check::user::exists("root");

  auto r1 = c1.check(jin);

  REQUIRE(r1->issue == wassail::result::issue_t::NO);
  REQUIRE(r1->system_id.size() == 1);
  REQUIRE(r1->system_id[0] == "localhost.local");
  REQUIRE(r1->timestamp == std::chrono::system_clock::from_time_t(1788391957));

  auto c2 = wassail::check::user::exists(
      "bob", "Brief {0}", "{0} does not exist", "Error {0}", "{0} exists");

  auto r2 = c2.check(jin);

  REQUIRE(r2->issue == wassail::result::issue_t::YES);
  REQUIRE(r2->brief == "Brief bob");
  REQUIRE(r2->detail == "bob does not exist");
}

TEST_CASE("exists getpwent input") {
  auto j = R"(
    {
      "data": {
        "users": [
          {
            "pw_dir": "/var/root",
            "pw_gid": 0,
            "pw_name": "root",
            "pw_shell": "/bin/sh",
            "pw_uid":0
          },
          {
           "pw_dir": "/var/empty",
           "pw_gid": 4294967294,
           "pw_name": "nobody",
           "pw_shell":"/usr/bin/false",
           "pw_uid": 4294967294
          },
          {
           "pw_dir":"/Users/scott",
           "pw_gid":20,
           "pw_name":"scott",
           "pw_shell":"/bin/bash",
           "pw_uid":501}
        ]
      },
      "hostname": "localhost.local",
      "name": "getpwent",
      "timestamp": 1788391957,
      "uid": 501,
      "version": 100
    }
  )"_json;

  wassail::data::getpwent d = j;

  auto c = wassail::check::user::exists("nobody");

  auto r = c.check(d);

  REQUIRE(r->issue == wassail::result::issue_t::NO);
}
