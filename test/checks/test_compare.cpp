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
#include <wassail/checks/compare.hpp>
#include <wassail/data/sysconf.hpp>

using json = nlohmann::json;

TEST_CASE("compare basic JSON input") {
  json j = {
      {"name", "fake"},
      {"data", {{"v_int", 4096}, {"v_float", 3.1415}, {"v_string", "foo"}}},
      {"timestamp", 1234}};

  auto c = wassail::check::compare();

  auto r1 =
      c.check(j, json::json_pointer("/data/v_int"), std::equal_to<int>{}, 4096);
  REQUIRE(r1->issue == wassail::result::issue_t::NO);
  REQUIRE(r1->priority == wassail::result::priority_t::INFO);
  REQUIRE(r1->system_id.size() == 0);
  REQUIRE(r1->timestamp == std::chrono::system_clock::from_time_t(1234));

  /* modify json to add hostname entry */
  j["hostname"] = "localhost";

  auto r2 =
      c.check(j, json::json_pointer("/data/v_int"), std::equal_to<int>{}, 4);
  REQUIRE(r2->issue == wassail::result::issue_t::YES);
  REQUIRE(r2->priority == wassail::result::priority_t::WARNING);
  REQUIRE(r2->system_id.size() == 1);
  REQUIRE(r2->system_id[0] == "localhost");

  auto r3 = c.check(j, json::json_pointer("/data/v_float"),
                    std::greater<float>{}, 3.0);
  REQUIRE(r3->issue == wassail::result::issue_t::NO);

  auto r4 = c.check(j, json::json_pointer("/data/v_float"),
                    std::less_equal<float>{}, 3.0);
  REQUIRE(r4->issue == wassail::result::issue_t::YES);

  auto r5 = c.check(j, json::json_pointer("/data/v_string"),
                    std::not_equal_to<std::string>{},
                    static_cast<std::string>("bar"));
  REQUIRE(r5->issue == wassail::result::issue_t::NO);

  auto r6 = c.check(j, json::json_pointer("/data/v_string"),
                    std::not_equal_to<std::string>{},
                    static_cast<std::string>("foo"));
  REQUIRE(r6->issue == wassail::result::issue_t::YES);

  auto r7 = c.check(j, json::json_pointer("/invalid/path"),
                    std::equal_to<int>{}, 4096);
  REQUIRE(r7->issue == wassail::result::issue_t::MAYBE);

  auto r8 = c.check(j, json::json_pointer("/data/v_string"),
                    [](auto a, auto b) { return a == b; },
                    static_cast<std::string>("foo"));
  REQUIRE(r8->issue == wassail::result::issue_t::NO);

  json jcompare = {{"v_int", 4096}, {"v_float", 3.1415}, {"v_string", "foo"}};
  auto r9 = c.check(j, json::json_pointer("/data"), std::equal_to<json>{},
                    static_cast<json>(jcompare));
  REQUIRE(r9->issue == wassail::result::issue_t::NO);

  auto r10 =
      c.check(j, json::json_pointer("/data/v_string"),
              [](auto a, auto b) { return std::regex_match(a, std::regex(b)); },
              static_cast<std::string>(".oo$"));
  REQUIRE(r10->issue == wassail::result::issue_t::NO);

  auto c2 = wassail::check::compare("Brief {0} {1}", "{0} != {1}",
                                    ":shrug:", "{0} == {1}");

  auto r11 =
      c2.check(j, json::json_pointer("/data/v_string"),
               std::equal_to<std::string>{}, static_cast<std::string>("bar"));
  REQUIRE(r11->issue == wassail::result::issue_t::YES);
  REQUIRE(r11->brief == "Brief /data/v_string bar");
  REQUIRE(r11->detail == "foo != bar");
}

TEST_CASE("vector comparison") {
  json j1 = {
      {"name", "fake"},
      {"data", {{"v_int", 4096}, {"v_float", 3.1415}, {"v_string", "foo"}}},
      {"timestamp", 1111}};
  json j2 = {
      {"name", "fake"},
      {"data", {{"v_int", 2048}, {"v_float", 3.1415}, {"v_string", "foo"}}},
      {"timestamp", 2222}};
  json j3 = {
      {"name", "fake"},
      {"data", {{"v_int", 1024}, {"v_float", 3.1415}, {"v_string", "foo"}}},
      {"timestamp", 3333}};
  json j4 = {{"name", "fake"},
             {"data", {{"v_int", 1024}, {"v_float", 3.1415}}},
             {"timestamp", 4444}};

  std::vector<json> v = {j1, j2, j3};

  std::vector<json> v2 = {j3, j4};

  auto c = wassail::check::compare();

  auto r1 = c.check(v.cbegin(), v.cend(), json::json_pointer("/data/v_int"),
                    std::equal_to<int>{}, 4096);
  REQUIRE(r1->issue == wassail::result::issue_t::YES);
  REQUIRE(r1->priority == wassail::result::priority_t::WARNING);
  REQUIRE(r1->children.size() == 3);

  auto r2 =
      c.check(v.cbegin(), v.cend(), json::json_pointer("/data/v_string"),
              std::equal_to<std::string>{}, static_cast<std::string>("foo"));
  REQUIRE(r2->issue == wassail::result::issue_t::NO);
  REQUIRE(r2->priority == wassail::result::priority_t::INFO);
  REQUIRE(r2->children.size() == 3);

  auto r3 = c.check(v.cbegin(), v.cend(), json::json_pointer("/data/v_float"),
                    std::greater<float>{}, 3.0);
  REQUIRE(r3->issue == wassail::result::issue_t::NO);
  REQUIRE(r3->children.size() == 3);

  auto r4 = c.check(v.cbegin(), v.cend(), json::json_pointer("/data/v_int"),
                    std::not_equal_to<int>{}, 2048);
  REQUIRE(r4->issue == wassail::result::issue_t::YES);
  REQUIRE(r4->children.size() == 3);

  auto r5 = c.check(v.cbegin(), v.cend(), json::json_pointer("/data/v_string"),
                    [](auto a, auto b) { return a == b; },
                    static_cast<std::string>("foo"));
  REQUIRE(r5->issue == wassail::result::issue_t::NO);
  REQUIRE(r5->children.size() == 3);

  auto r6 = c.check(v.cbegin(), v.cend(), json::json_pointer("/invalid/path"),
                    std::equal_to<int>{}, 1);
  REQUIRE(r6->issue == wassail::result::issue_t::MAYBE);
  REQUIRE(r6->children.size() == 3);

  auto r7 =
      c.check(v2.cbegin(), v2.cend(), json::json_pointer("/data/v_string"),
              std::equal_to<std::string>{}, static_cast<std::string>("foo"));
  REQUIRE(r7->issue == wassail::result::issue_t::MAYBE);
  REQUIRE(r7->children.size() == 2);

  auto c2 = wassail::check::compare("Brief {0} {1}", "{0} != {1}",
                                    ":shrug:", "{0} == {1}");

  auto r8 = c2.check(v.cbegin(), v.cend(), json::json_pointer("/data/v_int"),
                     std::equal_to<int>{}, 2048);
  REQUIRE(r8->issue == wassail::result::issue_t::YES);
  REQUIRE(r8->children.size() == 3);
  REQUIRE(r8->children[0]->brief == "Brief /data/v_int 2048");
  REQUIRE(r8->children[0]->detail == "4096 != 2048");
  REQUIRE(r8->children[0]->timestamp ==
          std::chrono::system_clock::from_time_t(1111));
  REQUIRE(r8->children[1]->brief == "Brief /data/v_int 2048");
  REQUIRE(r8->children[1]->detail == "2048 == 2048");
  REQUIRE(r8->children[1]->timestamp ==
          std::chrono::system_clock::from_time_t(2222));
  REQUIRE(r8->children[2]->brief == "Brief /data/v_int 2048");
  REQUIRE(r8->children[2]->detail == "1024 != 2048");
  REQUIRE(r8->children[2]->timestamp ==
          std::chrono::system_clock::from_time_t(3333));
}

TEST_CASE("compare sysconf input") {
  auto c = wassail::check::compare();
  auto d = wassail::data::sysconf();
  /* d is implicitly converted to json, so must call evaluate() first */
  d.evaluate();
  auto r = c.check(d, json::json_pointer("/data/nprocessors_conf"),
                   std::greater<int>{}, 0);
  REQUIRE(r->issue == wassail::result::issue_t::NO);
}
