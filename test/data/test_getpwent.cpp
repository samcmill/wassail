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
#include <wassail/data/getpwent.hpp>

TEST_CASE("getpwent basic usage") {
  auto d = wassail::data::getpwent();

  if (d.enabled()) {
    d.evaluate();
    json j = d;
    REQUIRE(j["data"]["users"].size() > 0);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("getpwent JSON conversion") {
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

  wassail::data::getpwent d = jin;
  json jout = d;

  REQUIRE(jout.size() >= 0);
  REQUIRE(jout == jin);
}

TEST_CASE("getpwent common pointer JSON conversion") {
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

  std::shared_ptr<wassail::data::common> d =
      std::make_shared<wassail::data::getpwent>();

  d->from_json(jin);
  json jout = d->to_json();

  REQUIRE(jout.size() >= 0);
  REQUIRE(jout == jin);
}

TEST_CASE("getpwent invalid JSON conversion") {
  auto jin = R"({ "name": "invalid" })"_json;
  wassail::data::getpwent d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("getpwent incomplete JSON conversion") {
  auto jin = R"({ "name": "getpwent", "timestamp": 0, "version": 100})"_json;

  wassail::data::getpwent d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "getpwent");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("users") == 1);
  REQUIRE(jout["data"]["users"].size() == 0);
}

TEST_CASE("getpwent factory evaluate") {
  auto jin = R"({ "name": "getpwent" })"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "getpwent");
    REQUIRE(jout.count("data") == 1);
    REQUIRE(jout["data"]["users"].size() >= 1);
  }
}
