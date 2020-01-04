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

TEST_CASE("environment invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::environment d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("environment incomplete JSON conversion") {
  auto jin = R"({ "name": "environment", "timestamp": 0, "version": 100})"_json;
  wassail::data::environment d;
  REQUIRE_THROWS(d = jin);
}
