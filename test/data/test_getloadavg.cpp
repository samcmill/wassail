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
#include <wassail/data/getloadavg.hpp>

TEST_CASE("getloadavg basic usage") {
  auto d = wassail::data::getloadavg();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["name"] == "getloadavg");
    REQUIRE(j["timestamp"].is_number() == true);
    REQUIRE(j["data"]["load1"].get<double>() >= 0);
    REQUIRE(j["data"]["load5"].get<double>() >= 0);
    REQUIRE(j["data"]["load15"].get<double>() >= 0);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("getloadavg JSON conversions") {
  auto jin = R"(
    {
      "data": {
        "load1": 1.5361328125,
        "load5": 1.48095703125,
        "load15": 1.74267578125
      },
      "hostname": "localhost.local",
      "name": "getloadavg",
      "timestamp": 1528948131,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::getloadavg d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);
  REQUIRE(jout == jin);
}

TEST_CASE("getloadavg invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::getloadavg d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("getloadavg incomplete JSON conversion") {
  auto jin = R"({ "name": "getloadavg", "timestamp": 0, "version": 100})"_json;

  wassail::data::getloadavg d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "getloadavg");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("load1") == 1);
}
