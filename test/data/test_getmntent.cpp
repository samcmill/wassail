/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define CATCH_CONFIG_MAIN
#include "3rdparty/catch/catch.hpp"
#include "3rdparty/catch/catch_reporter_automake.hpp"

#include <wassail/data/getmntent.hpp>

TEST_CASE("getmntent basic usage") {
  auto d = wassail::data::getmntent();

  if (d.enabled()) {
    d.evaluate();
    json j = d;
    REQUIRE(j["data"]["file_systems"].size() >= 1);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("getmntent JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "file_systems": [
          {
            "bavail": 76420,
            "bfree": 76420,
            "blocks": 1621504,
            "bsize": 4096,
            "dir": "/",
            "favail": 64768,
            "ffree": 611763,
            "files": 778784,
            "flag": 4096,
            "frsize": 4096,
            "fsid": 64768,
            "fsname": "/dev/mapper/centos_centos7-root",
            "type": "xfs"
          }
	]
      },
      "hostname": "localhost.local",
      "name": "getmntent",
      "timestamp": 1529033551,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::getmntent d = jin;
  json jout = d;

  REQUIRE(jout.size() >= 0);
  REQUIRE(jout == jin);
}

TEST_CASE("getmntent invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::getmntent d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("getmntent incomplete JSON conversion") {
  auto jin = R"({ "name": "getmntent", "timestamp": 0, "version": 100})"_json;
  wassail::data::getmntent d;
  REQUIRE_THROWS(d = jin);
}
