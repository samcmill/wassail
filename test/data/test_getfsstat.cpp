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
#include <wassail/data/getfsstat.hpp>

TEST_CASE("getfsstat basic usage") {
  auto d = wassail::data::getfsstat();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["file_systems"].size() >= 1);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("getfsstat JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "file_systems": [
          {
            "bavail": 5771575,
            "bfree": 6722834,
            "blocks": 61228134,
            "bsize": 4096,
            "ffree": 9223372036853658231,
            "files": 9223372036854775807,
            "flags": 75550720,
            "fstypename": "apfs",
            "mntfromname": "/dev/disk1s1",
            "mntonname": "/",
            "owner": 0
          },
          {
            "bavail": 0,
            "bfree": 0,
            "blocks": 0,
            "bsize": 1024,
            "ffree": 0,
            "files": 0,
            "flags": 72351744,
            "fstypename": "apfs",
            "mntfromname": "map auto_home",
            "mntonname": "/home",
            "owner": 0
          }
        ]
      },
      "hostname": "localhost.local",
      "name": "getfsstat",
      "timestamp": 1529033551,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::getfsstat d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);
  REQUIRE(jout == jin);
}

TEST_CASE("getfsstat invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::getfsstat d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("getfsstat incomplete JSON conversion") {
  auto jin = R"({ "name": "getfsstat", "timestamp": 0, "version": 100})"_json;
  wassail::data::getfsstat d;
  REQUIRE_THROWS(d = jin);
}
