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
#include <wassail/checks/file/permissions.hpp>

using json = nlohmann::json;

TEST_CASE("permissions unknown JSON") {
  json j = {{"name", "unknown"}, {"data", {{"mode", 17407}}}, {"timestamp", 0}};

  auto c = wassail::check::file::permissions(01777);

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("permissions invalid JSON") {
  json j = {{"foo", "bar"}};

  auto c = wassail::check::file::permissions(0755);

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("permissions basic JSON (stat)") {
  auto jin = R"(
    {
      "data": {
        "mode": 17407,
        "path": "/tmp"
      },
      "hostname": "localhost.local",
      "name": "stat",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  auto c1 = wassail::check::file::permissions(01777);

  auto r1 = c1.check(jin);

  REQUIRE(r1->issue == wassail::result::issue_t::NO);
  REQUIRE(r1->system_id.size() == 1);
  REQUIRE(r1->system_id[0] == "localhost.local");
  REQUIRE(r1->timestamp == std::chrono::system_clock::from_time_t(1530420039));

  auto c2 = wassail::check::file::permissions(0777, "Brief {0}", "{1} != {2}",
                                              "Error {0}", "{1} == {2}");

  auto r2 = c2.check(jin);

  REQUIRE(r2->issue == wassail::result::issue_t::YES);
  REQUIRE(r2->brief == "Brief /tmp");
  REQUIRE(r2->detail == "1777 != 0777");
}

TEST_CASE("permissions stat input") {
  auto j = R"(
    {
      "data": {
        "atime": 1542691487.0,
        "blksize": 4096,
        "blocks": 0,
        "ctime": 1542691639.0,
        "device": 16777220,
        "gid": 0,
        "inode": 4312304510,
        "mode": 17407,
        "mtime": 1542691487.0,
        "nlink": 1,
        "path": "/tmp",
        "rdev": 0,
        "size": 11,
        "uid": 0
      },
      "hostname": "localhost.local",
      "name": "stat",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::stat d = j;

  auto c = wassail::check::file::permissions(01777);

  auto r = c.check(d);

  REQUIRE(r->issue == wassail::result::issue_t::NO);
}
