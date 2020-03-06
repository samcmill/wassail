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

#include <chrono>
#include <wassail/result.hpp>

TEST_CASE("result instantiation") {
  auto r = wassail::result();
  r.action = "Action";
  r.brief = "Brief";
  r.detail = "Detail";
  r.issue = wassail::result::issue_t::YES;
  r.priority = wassail::result::priority_t::CRITICAL;

  REQUIRE(r.action == "Action");
  REQUIRE(r.brief == "Brief");
  REQUIRE(r.children.size() == 0);
  REQUIRE(r.detail == "Detail");
  REQUIRE(r.issue == wassail::result::issue_t::YES);
  REQUIRE(r.priority == wassail::result::priority_t::CRITICAL);
}

TEST_CASE("make_result instantiation") {
  auto r = wassail::make_result();

  r->brief = "Brief";

  REQUIRE(r->brief == "Brief");
}

TEST_CASE("make_result instantiation - JSON variant") {
  auto j = R"(
    {
      "data": {
      },
      "hostname": "localhost.local",
      "name": "fake",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  auto r = wassail::make_result(j);

  REQUIRE(r->system_id.size() == 1);
  REQUIRE(r->system_id[0] == "localhost.local");
  REQUIRE(r->timestamp == std::chrono::system_clock::from_time_t(1530420039));
}

TEST_CASE("add_child") {
  auto a = wassail::make_result();

  auto b1 = wassail::make_result();
  auto b2 = wassail::make_result();

  a->add_child(b1);
  a->add_child(b2);

  REQUIRE(a->children.size() == 2);
}

TEST_CASE("second level add_child") {
  auto a = wassail::make_result();
  a->brief = "A";

  auto b = wassail::make_result();
  b->brief = "B";
  a->add_child(b);

  /* Note that c is added after b has been added to a */
  auto c = wassail::make_result();
  c->brief = "C";
  b->add_child(c);

  REQUIRE(a->children.size() == 1);
  REQUIRE(b->children.size() == 1);

  for (auto i : a->children) {
    REQUIRE(i->children.size() == 1);
    for (auto j : i->children) {
      REQUIRE(j->brief == "C");
    }
  }
}

TEST_CASE("match_issue") {
  auto r = wassail::make_result();

  /* match_issue only matches children */
  r->issue = wassail::result::issue_t::YES;
  REQUIRE(r->match_issue(wassail::result::issue_t::YES) == false);
  REQUIRE(r->match_issue(wassail::result::issue_t::MAYBE) == false);
  REQUIRE(r->match_issue(wassail::result::issue_t::NO) == false);
}

TEST_CASE("second level match_issue, max_issue, and propagate") {
  auto a = wassail::make_result();

  REQUIRE(a->max_issue() == wassail::result::issue_t::MAYBE);

  auto b1 = wassail::make_result();
  b1->issue = wassail::result::issue_t::YES;
  a->add_child(b1);

  auto b2 = wassail::make_result();
  b2->issue = wassail::result::issue_t::MAYBE;
  a->add_child(b2);

  auto c = wassail::make_result();
  c->issue = wassail::result::issue_t::YES;
  b1->add_child(c);

  REQUIRE(a->match_issue(wassail::result::issue_t::YES) == true);
  REQUIRE(a->match_issue(wassail::result::issue_t::MAYBE) == true);
  REQUIRE(a->match_issue(wassail::result::issue_t::NO) == false);

  REQUIRE(a->max_issue() == wassail::result::issue_t::YES);

  a->propagate();

  REQUIRE(a->issue == wassail::result::issue_t::YES);
}

TEST_CASE("match_priority") {
  auto r = wassail::make_result();

  /* match_priority only matches children */
  r->priority = wassail::result::priority_t::INFO;
  REQUIRE(r->match_priority(wassail::result::priority_t::EMERGENCY) == false);
  REQUIRE(r->match_priority(wassail::result::priority_t::ALERT) == false);
  REQUIRE(r->match_priority(wassail::result::priority_t::ERROR) == false);
  REQUIRE(r->match_priority(wassail::result::priority_t::WARNING) == false);
  REQUIRE(r->match_priority(wassail::result::priority_t::NOTICE) == false);
  REQUIRE(r->match_priority(wassail::result::priority_t::INFO) == false);
  REQUIRE(r->match_priority(wassail::result::priority_t::DEBUG) == false);
}

TEST_CASE("second level match_priority, max_priority, and propagate") {
  auto a = wassail::make_result();

  REQUIRE(a->max_priority() == wassail::result::priority_t::NOTICE);

  auto b1 = wassail::make_result();
  b1->priority = wassail::result::priority_t::ERROR;
  a->add_child(b1);

  auto b2 = wassail::make_result();
  b2->priority = wassail::result::priority_t::NOTICE;
  a->add_child(b2);

  auto c = wassail::make_result();
  c->priority = wassail::result::priority_t::INFO;
  b1->add_child(c);

  REQUIRE(a->match_priority(wassail::result::priority_t::EMERGENCY) == false);
  REQUIRE(a->match_priority(wassail::result::priority_t::ALERT) == false);
  REQUIRE(a->match_priority(wassail::result::priority_t::ERROR) == true);
  REQUIRE(a->match_priority(wassail::result::priority_t::WARNING) == false);
  REQUIRE(a->match_priority(wassail::result::priority_t::NOTICE) == true);
  REQUIRE(a->match_priority(wassail::result::priority_t::INFO) == true);
  REQUIRE(a->match_priority(wassail::result::priority_t::DEBUG) == false);

  REQUIRE(a->max_priority() == wassail::result::priority_t::ERROR);

  a->propagate();

  REQUIRE(a->priority == wassail::result::priority_t::ERROR);
}

TEST_CASE("result JSON conversion") {
  auto jout = R"(
    {
      "action": "Action",
      "brief": "Brief",
      "children": [
        {
          "action": "",
          "brief": "Child1a",
          "children": [
            {
              "action": "",
              "brief": "Child2",
              "children": [],
              "detail": "",
              "issue": 1,
              "priority": 2,
              "system_id": [],
              "timestamp": 0
            }
          ],
          "detail": "",
          "issue": 1,
          "priority": 2,
          "system_id": [],
          "timestamp": 0
        },
        {
          "action": "",
          "brief": "Child1b",
          "children": [],
          "detail": "",
          "issue": 1,
          "priority": 2,
          "system_id": [],
          "timestamp": 0
        }
      ],
      "detail": "Detail",
      "issue": 2,
      "priority": 5,
      "system_id": ["localhost"],
      "timestamp": 1528948131
    }
  )"_json;

  auto r = wassail::make_result();
  r->action = "Action";
  r->brief = "Brief";
  r->detail = "Detail";
  r->issue = wassail::result::issue_t::YES;
  r->priority = wassail::result::priority_t::CRITICAL;
  r->system_id.push_back("localhost");
  r->timestamp = std::chrono::system_clock::from_time_t(1528948131);

  auto r1a = wassail::make_result();
  r1a->brief = "Child1a";
  r->add_child(r1a);

  auto r1b = wassail::make_result();
  r1b->brief = "Child1b";
  r->add_child(r1b);

  auto r2 = wassail::make_result();
  r2->brief = "Child2";
  r1a->add_child(r2);

  json j = static_cast<json>(r);

  REQUIRE(j == jout);
}
