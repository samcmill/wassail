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
#include <wassail/data/getcpuid.hpp>

TEST_CASE("getcpuid basic usage") {
  auto d = wassail::data::getcpuid();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["name"] == "getcpuid");
    REQUIRE(j["timestamp"].is_number() == true);
    REQUIRE(j["data"]["family"].get<uint32_t>() > 0);
    REQUIRE(j["data"]["model"].get<uint32_t>() > 0);
    REQUIRE(j["data"]["name"].size() > 0);
    REQUIRE(j["data"]["stepping"].get<uint32_t>() > 0);
    REQUIRE(j["data"]["type"].get<uint32_t>() >= 0);
    REQUIRE(j["data"]["vendor"].size() > 0);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("getcpuid JSON conversions") {
  auto jin = R"(
    {
      "data": {
        "family": 6,
        "model": 58,
        "name": "Intel(R) Core(TM) i5-3230M CPU @ 2.60GHz",
        "stepping": 9,
        "type": 0,
        "vendor": "GenuineIntel"
      },
      "hostname": "localhost.local",
      "name": "getcpuid",
      "timestamp": 1528860991,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::getcpuid d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);
  REQUIRE(jout == jin);
}

TEST_CASE("getcpuid invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::getcpuid d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("getcpuid incomplete JSON conversion") {
  auto jin = R"({ "name": "getcpuid", "timestamp": 0, "version": 100})"_json;

  wassail::data::getcpuid d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "getcpuid");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("vendor") == 1);
  REQUIRE(jout["data"]["vendor"] == "");
}
