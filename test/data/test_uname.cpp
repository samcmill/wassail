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
#include <string>
#include <wassail/data/uname.hpp>

TEST_CASE("uname basic usage") {
  auto d = wassail::data::uname();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["machine"].get<std::string>().size() > 0);
    REQUIRE(j["data"]["nodename"].get<std::string>().size() > 0);
    REQUIRE(j["data"]["release"].get<std::string>().size() > 0);
    REQUIRE(j["data"]["sysname"].get<std::string>().size() > 0);
    REQUIRE(j["data"]["version"].get<std::string>().size() > 0);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("uname JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "machine":"x86_64",
        "nodename":"localhost.local",
        "release":"18.0.0",
        "sysname":"Darwin",
        "version":"Darwin Kernel Version 18.0.0: Wed Aug 22 20:13:40 PDT 2018; root:xnu-4903.201.2~1/RELEASE_X86_64"
      },
      "hostname": "localhost.local",
      "name": "uname",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::uname d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("uname invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::uname d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("uname incomplete JSON conversion") {
  auto jin = R"({ "name": "uname", "timestamp": 0, "version": 100})"_json;
  wassail::data::uname d;
  REQUIRE_THROWS(d = jin);
}
