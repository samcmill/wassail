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
#include <wassail/data/sysinfo.hpp>

TEST_CASE("sysinfo basic usage") {
  auto d = wassail::data::sysinfo();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["totalram"].get<unsigned long>() > 0);
    REQUIRE(j["data"]["freeram"].get<unsigned long>() <
            j["data"]["totalram"].get<unsigned long>());
    REQUIRE(j["data"]["totalswap"].get<unsigned long>() >= 0);
    REQUIRE(j["data"]["freeswap"].get<unsigned long>() <
            j["data"]["totalswap"].get<unsigned long>());
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("sysinfo JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "bufferram": 0,
        "freehigh": 0,
        "freeram": 610422784,
        "freeswap": 312442880,
        "load1": 80672,
        "load15": 101664,
        "load5": 87520,
        "loads_scale": 65536,
        "mem_unit": 1,
        "procs": 394,
        "sharedram": 0,
        "totalhigh": 0,
        "totalram": 1040621568,
        "totalswap": 859828224,
        "uptime": 44835
      },
      "hostname": "localhost.local",
      "name": "sysinfo",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::sysinfo d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("sysinfo invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::sysinfo d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("sysinfo incomplete JSON conversion") {
  auto jin = R"({ "name": "sysinfo", "timestamp": 0, "version": 100})"_json;
  wassail::data::sysinfo d;
  REQUIRE_THROWS(d = jin);
}
