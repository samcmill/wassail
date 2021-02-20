/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define CATCH_CONFIG_MAIN
#include "3rdparty/catch/catch.hpp"
#include "3rdparty/catch/catch_reporter_automake.hpp"

#include <chrono>
#include <wassail/data/stat.hpp>

TEST_CASE("stat basic usage") {
  auto d = wassail::data::stat("/tmp");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["path"].get<std::string>() == "/tmp");
    REQUIRE(j["data"]["atime"].get<double>() > 0);
    REQUIRE(j["data"]["inode"].get<uint32_t>() != 0);
    REQUIRE(j["data"]["uid"].get<uint32_t>() == 0);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("stat JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "path": "/tmp"
      },
      "data": {
        "atime": 1542691487.0,
        "blksize": 4096,
        "blocks": 0,
        "ctime": 1542691639.0,
        "device": 16777220,
        "gid": 0,
        "inode": 4312304510,
        "mode": 41453,
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

  wassail::data::stat d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("stat invalid JSON conversion") {
  auto jin = R"({ "name": "invalid" })"_json;
  wassail::data::stat d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("stat incomplete JSON conversion") {
  auto jin = R"({ "name": "stat", "timestamp": 0, "version": 100})"_json;

  wassail::data::stat d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "stat");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("path") == 1);
  REQUIRE(jout["data"]["path"] == "");
}

TEST_CASE("stat factory evaluate") {
  auto jin = R"(
    {
      "configuration": {
        "path": "/tmp"
      },
      "name": "stat"
    }
  )"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "stat");
    REQUIRE(jout.count("data") == 1);
    REQUIRE(jout["data"]["inode"].get<uint32_t>() != 0);
  }
}
