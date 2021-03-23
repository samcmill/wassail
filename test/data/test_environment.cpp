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
#include <cstdlib>
#include <string>
#include <wassail/data/environment.hpp>

TEST_CASE("environment basic usage") {
  auto d = wassail::data::environment();

  /* set a dummy environment variable */
  int rv = setenv("__ZZZ", "FOO=BAR", 1);
  REQUIRE(rv == 0);

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"].size() > 0);
    REQUIRE(j["data"]["__ZZZ"].get<std::string>() == "FOO=BAR");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("environment JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "USER": "ncognito",
        "HOME": "/home/ncognito",
        "SHELL": "/bin/bash"
      },
      "hostname": "localhost.local",
      "name": "environment",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::environment d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("environment common pointer JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "USER": "ncognito",
        "HOME": "/home/ncognito",
        "SHELL": "/bin/bash"
      },
      "hostname": "localhost.local",
      "name": "environment",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  std::shared_ptr<wassail::data::common> d =
      std::make_shared<wassail::data::environment>();

  d->from_json(jin);
  json jout = d->to_json();

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("environment invalid JSON conversion") {
  auto jin = R"({ "name": "invalid" })"_json;
  wassail::data::environment d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("environment incomplete JSON conversion") {
  auto jin = R"({ "name": "environment", "timestamp": 0, "version": 100})"_json;

  wassail::data::environment d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "environment");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].size() == 0);
}

TEST_CASE("environment factory evaluate") {
  auto jin = R"({ "name": "environment" })"_json;

  /* set a dummy environment variable */
  int rv = setenv("__ZZZ", "FOO=BAR", 1);
  REQUIRE(rv == 0);

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "environment");
    REQUIRE(jout.count("data") == 1);
    REQUIRE(jout["data"]["__ZZZ"].get<std::string>() == "FOO=BAR");
  }
}
