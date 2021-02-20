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
#include <wassail/data/getrlimit.hpp>

TEST_CASE("getrlimit basic usage") {
  auto d = wassail::data::getrlimit();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["hard"]["core"].get<uint64_t>() >= 0);
    REQUIRE(j["data"]["hard"]["core"].get<uint64_t>() >=
            j["data"]["soft"]["core"].get<uint64_t>());
    REQUIRE(j["data"]["hard"]["cpu"].get<uint64_t>() > 0);
    REQUIRE(j["data"]["hard"]["cpu"].get<uint64_t>() >=
            j["data"]["soft"]["cpu"].get<uint64_t>());
    REQUIRE(j["data"]["hard"]["data"].get<uint64_t>() > 0);
    REQUIRE(j["data"]["hard"]["data"].get<uint64_t>() >=
            j["data"]["soft"]["data"].get<uint64_t>());
    REQUIRE(j["data"]["hard"]["fsize"].get<uint64_t>() > 0);
    REQUIRE(j["data"]["hard"]["fsize"].get<uint64_t>() >=
            j["data"]["soft"]["fsize"].get<uint64_t>());
    REQUIRE(j["data"]["hard"]["memlock"].get<uint64_t>() > 0);
    REQUIRE(j["data"]["hard"]["memlock"].get<uint64_t>() >=
            j["data"]["soft"]["memlock"].get<uint64_t>());
    REQUIRE(j["data"]["hard"]["nofile"].get<uint64_t>() > 0);
    REQUIRE(j["data"]["hard"]["nofile"].get<uint64_t>() >=
            j["data"]["soft"]["nofile"].get<uint64_t>());
    REQUIRE(j["data"]["hard"]["nproc"].get<uint64_t>() > 0);
    REQUIRE(j["data"]["hard"]["nproc"].get<uint64_t>() >=
            j["data"]["soft"]["nproc"].get<uint64_t>());
    REQUIRE(j["data"]["hard"]["rss"].get<uint64_t>() > 0);
    REQUIRE(j["data"]["hard"]["rss"].get<uint64_t>() >=
            j["data"]["soft"]["rss"].get<uint64_t>());
    REQUIRE(j["data"]["hard"]["stack"].get<uint64_t>() > 0);
    REQUIRE(j["data"]["hard"]["stack"].get<uint64_t>() >=
            j["data"]["soft"]["stack"].get<uint64_t>());
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("getrlimit JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "hard": {
          "core": 9223372036854775807,
          "cpu": 9223372036854775807,
          "data": 9223372036854775807,
          "fsize": 9223372036854775807,
          "memlock": 9223372036854775807,
          "nofile": 9223372036854775807,
          "nproc": 1064,
          "rss": 9223372036854775807,
          "stack": 67104768
        },
        "soft": {
          "core": 0,
          "cpu": 9223372036854775807,
          "data": 9223372036854775807,
          "fsize": 9223372036854775807,
          "memlock": 9223372036854775807,
          "nofile": 256,
          "nproc": 709,
          "rss": 9223372036854775807,
          "stack": 8388608
        }
      },
      "hostname": "localhost.local",
      "name": "getrlimit",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::getrlimit d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("getrlimit invalid JSON conversion") {
  auto jin = R"({ "name": "invalid" })"_json;
  wassail::data::getrlimit d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("getrlimit incomplete JSON conversion") {
  auto jin = R"({ "name": "getrlimit", "timestamp": 0, "version": 100})"_json;

  wassail::data::getrlimit d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "getrlimit");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("hard") == 1);
  REQUIRE(jout["data"].count("soft") == 1);
}

TEST_CASE("getrlimit factory evaluate") {
  auto jin = R"({ "name": "getrlimit" })"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "getrlimit");
    REQUIRE(jout.count("data") == 1);
    REQUIRE(jout["data"]["hard"]["core"].get<uint64_t>() >= 0);
  }
}
