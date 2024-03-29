/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define CATCH_CONFIG_MAIN
#include "3rdparty/catch/catch.hpp"
#include "3rdparty/catch/catch_reporter_automake.hpp"

#include <sys/stat.h>
#include <wassail/data/getmntent.hpp>

TEST_CASE("getmntent basic usage") {
  auto d = wassail::data::getmntent();

  if (d.enabled()) {
    d.evaluate();
    json j = d;
    struct stat s;
    if (stat("/etc/mtab", &s) == 0) {
      /* will only pass on machines that actually have a mtab */
      REQUIRE(j["data"]["file_systems"].size() > 0);
    }
    else {
      REQUIRE(j["data"]["file_systems"].size() == 0);
    }
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

TEST_CASE("getmntent common pointer JSON conversion") {
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

  std::shared_ptr<wassail::data::common> d =
      std::make_shared<wassail::data::getmntent>();

  d->from_json(jin);
  json jout = d->to_json();

  REQUIRE(jout.size() >= 0);
  REQUIRE(jout == jin);
}

TEST_CASE("getmntent invalid JSON conversion") {
  auto jin = R"({ "name": "invalid" })"_json;
  wassail::data::getmntent d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("getmntent incomplete JSON conversion") {
  auto jin = R"({ "name": "getmntent", "timestamp": 0, "version": 100})"_json;

  wassail::data::getmntent d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "getmntent");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("file_systems") == 1);
  REQUIRE(jout["data"]["file_systems"].size() == 0);
}

TEST_CASE("getmntent factory evaluate") {
  auto jin = R"({ "name": "getmntent" })"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "getmntent");
    REQUIRE(jout.count("data") == 1);

    struct stat s;
    if (stat("/etc/mtab", &s) == 0) {
      /* will only pass on machines that actually have a mtab */
      REQUIRE(jout["data"]["file_systems"].size() >= 1);
    }
    else {
      REQUIRE(jout["data"]["file_systems"].size() == 0);
    }
  }
}
