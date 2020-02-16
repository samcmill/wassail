/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* The operator<< overloads must be included before the catch header */
#include "tostring.h"

#define CATCH_CONFIG_MAIN
#include "3rdparty/catch/catch.hpp"
#include "3rdparty/catch/catch_reporter_automake.hpp"

#include <regex>
#include <wassail/checks/rules_engine.hpp>
#include <wassail/data/sysconf.hpp>

using json = nlohmann::json;

TEST_CASE("rules_engine basic JSON input") {
  json j = {
      {"name", "fake"},
      {"data", {{"v_int", 4096}, {"v_float", 3.1415}, {"v_string", "foo"}}},
      {"timestamp", 1234}};

  auto c = wassail::check::rules_engine();

  c.add_rule([](json j) {
    return j.at(json::json_pointer("/data/v_int")).get<int>() == 4096;
  });
  auto r1 = c.check(j);
  REQUIRE(r1->issue == wassail::result::issue_t::NO);
  REQUIRE(r1->priority == wassail::result::priority_t::INFO);
  REQUIRE(r1->system_id.size() == 0);
  REQUIRE(r1->timestamp == std::chrono::system_clock::from_time_t(1234));

  /* modify json to add hostname entry */
  j["hostname"] = "localhost";

  auto r2 = c.check(j);
  REQUIRE(r2->issue == wassail::result::issue_t::NO);
  REQUIRE(r2->priority == wassail::result::priority_t::INFO);
  REQUIRE(r2->system_id.size() == 1);
  REQUIRE(r2->system_id[0] == "localhost");

  c.add_rule([](json j) {
    return j.at(json::json_pointer("/data/v_float")).get<float>() > 3;
  });
  auto r3 = c.check(j);
  REQUIRE(r3->issue == wassail::result::issue_t::NO);

  c.add_rule([](json j) {
    return j.at(json::json_pointer("/data/v_float")).get<float>() <= 3;
  });
  auto r4 = c.check(j);
  REQUIRE(r4->issue == wassail::result::issue_t::YES);
}

TEST_CASE("rules_engine bad rule") {
  json j = {
      {"name", "fake"},
      {"data", {{"v_int", 4096}, {"v_float", 3.1415}, {"v_string", "foo"}}},
      {"timestamp", 1234}};

  auto c = wassail::check::rules_engine();

  c.add_rule([](json j) {
    return j.at(json::json_pointer("/bad/key")).get<std::string>() == "missing";
  });
  auto r = c.check(j);
  REQUIRE(r->issue == wassail::result::issue_t::MAYBE);
}

TEST_CASE("rules_engine sysconf input") {
  auto c = wassail::check::rules_engine();
  c.add_rule([](json j) {
    return j.at(json::json_pointer("/data/nprocessors_conf")).get<int>() > 0;
  });
  c.add_rule([](json j) {
    return j.at(json::json_pointer("/data/nprocessors_conf")).get<int>() <
           1000000;
  });

  auto d = wassail::data::sysconf();
  /* d is implicitly converted to json, so must call evaluate() first */
  d.evaluate();

  auto r = c.check(d);
  REQUIRE(r->issue == wassail::result::issue_t::NO);
}
