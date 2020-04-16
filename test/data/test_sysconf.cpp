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
#include <wassail/data/sysconf.hpp>

TEST_CASE("sysconf basic usage") {
  auto d = wassail::data::sysconf();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["nprocessors_onln"].get<long>() >= 1);
    REQUIRE(j["data"]["nprocessors_conf"].get<long>() >=
            j["data"]["nprocessors_onln"].get<long>());
    REQUIRE(j["data"]["page_size"].get<long>() >= 1);
    REQUIRE(j["data"]["phys_pages"].get<long>() >= 1);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("sysconf JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "nprocessors_conf": 4,
        "nprocessors_onln": 4,
        "page_size": 4096,
        "phys_pages": 2097152
      },
      "hostname": "localhost.local",
      "name": "sysconf",
      "timestamp": 1528057219,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::sysconf d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("sysconf invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::sysconf d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("sysconf incomplete JSON conversion") {
  auto jin = R"({ "name": "sysconf", "timestamp": 0, "version": 100})"_json;

  wassail::data::sysconf d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "sysconf");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("nprocessors_conf") == 1);
  REQUIRE(jout["data"]["nprocessors_conf"] == 0);
}
