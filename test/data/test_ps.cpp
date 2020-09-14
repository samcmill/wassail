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
#include <wassail/data/ps.hpp>

TEST_CASE("ps basic usage") {
  auto d = wassail::data::ps();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["processes"].size() >= 1);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("ps JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "command": "ps -eo user,pid,pcpu,pmem,vsz,rss,tt,state,start,time,command",
        "elapsed": 0.088170979,
        "returncode": 0,
        "stderr": "",
        "stdout": "USER               PID  %CPU %MEM      VSZ    RSS   TT  STAT STARTED      TIME COMMAND\nscott              792   0.0  0.0  4296080    772 s001  S     7Aug18   0:00.51 -tcsh\nscott            42117   0.0  0.0  4269348   1108 s001  S+   11:05PM   0:00.01 wassail-dump\n"
      },
      "hostname": "localhost.local",
      "name": "ps",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::ps d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  auto gold = R"(
        [
          {
            "command": "-tcsh",
            "pcpu": 0.0,
            "pid": 792,
            "pmem": 0.0,
            "rss": 772,
            "start": "7Aug18",
            "state": "S",
            "time": "0:00.51",
            "tt": "s001",
            "user": "scott",
            "vsz": 4296080
          },
          {
            "command": "wassail-dump",
            "pcpu": 0.0,
            "pid": 42117,
            "pmem": 0.0,
            "rss": 1108,
            "start": "11:05PM",
            "state": "S+",
            "time": "0:00.01",
            "tt": "s001",
            "user": "scott",
            "vsz": 4269348
          }
        ]
  )"_json;
  REQUIRE(jout["data"]["processes"] == gold);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("processes");
  REQUIRE(jout == jin);
}

TEST_CASE("ps JSON conversion - Linux") {
  auto jin = R"(
    {
      "data": {
        "command": "ps -eo user,pid,pcpu,pmem,vsz,rss,tt,state,start,time,command",
        "elapsed": 0.088170979,
        "returncode": 0,
        "stderr": "",
        "stdout": "USER       PID %CPU %MEM    VSZ   RSS TT       S  STARTED     TIME COMMAND\nroot         1  0.0  0.0 194016  7048 ?        S   Aug 13 00:33:21 /sbin/init\nroot     39820  0.0  0.0      0     0 ?        S 08:27:38 00:00:00 [kworker/1:2]\n"
      },
      "hostname": "localhost.local",
      "name": "ps",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::ps d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  auto gold = R"(
        [
          {
            "command": "/sbin/init",
            "pcpu": 0.0,
            "pid": 1,
            "pmem": 0.0,
            "rss": 7048,
            "start": "Aug 13",
            "state": "S",
            "time": "00:33:21",
            "tt": "?",
            "user": "root",
            "vsz": 194016
          },
          {
            "command": "[kworker/1:2]",
            "pcpu": 0.0,
            "pid": 39820,
            "pmem": 0.0,
            "rss": 0,
            "start": "08:27:38",
            "state": "S",
            "time": "00:00:00",
            "tt": "?",
            "user": "root",
            "vsz": 0
          }
        ]
  )"_json;
  REQUIRE(jout["data"]["processes"] == gold);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("processes");
  REQUIRE(jout == jin);
}
