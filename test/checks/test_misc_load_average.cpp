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
#include <wassail/checks/misc/load_average.hpp>

TEST_CASE("load_average unknown JSON") {
  json j = {
      {"name", "unknown"}, {"data", {{"load_average", 4}}}, {"timestamp", 0}};

  auto c = wassail::check::misc::load_average_1min(4);

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("load_average invalid JSON") {
  json j = {{"foo", "bar"}};

  auto c = wassail::check::misc::load_average_1min(4);

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("load_average basic JSON input (getloadavg)") {
  json j = {{"name", "getloadavg"},
            {"data", {{"load1", 2.0}, {"load5", 2.1}, {"load15", 2.2}}},
            {"timestamp", 0}};

  auto c1 = wassail::check::misc::load_average_1min();
  c1.config.load = 2.0;
  auto r1 = c1.check(j);
  REQUIRE(r1->issue == wassail::result::issue_t::NO);

  auto c5 = wassail::check::misc::load_average_5min(2.0);
  auto r5 = c5.check(j);
  REQUIRE(r5->issue == wassail::result::issue_t::YES);

  auto c15 = wassail::check::misc::load_average_15min(
      2.1234, "Brief {0}", "{0} > {1}", ":shrug:", "{0} <= {1}");
  auto r15 = c15.check(j);
  REQUIRE(r15->issue == wassail::result::issue_t::YES);
  REQUIRE(r15->brief == "Brief 15");
  REQUIRE(r15->detail == "2.2 > 2.1234");
}

TEST_CASE("load_average basic JSON input (sysinfo)") {
  json j = {{"name", "sysinfo"},
            {"data",
             {{"load1", 80672},
              {"load5", 87520},
              {"load15", 101664},
              {"loads_scale", 65536}}}};

  auto c1 = wassail::check::misc::load_average_1min();
  c1.config.load = 1.0;
  auto r1 = c1.check(j);
  REQUIRE(r1->issue == wassail::result::issue_t::YES);

  auto c5 = wassail::check::misc::load_average_5min(2.0);
  auto r5 = c5.check(j);
  REQUIRE(r5->issue == wassail::result::issue_t::NO);

  auto c15 = wassail::check::misc::load_average_15min(
      1.5678, "Brief {0}", "{0} > {1}", ":shrug:", "{0} <= {1}");
  auto r15 = c15.check(j);
  REQUIRE(r15->issue == wassail::result::issue_t::NO);
  REQUIRE(r15->brief == "Brief 15");
  REQUIRE(r15->detail == "1.55127 <= 1.5678");
}

TEST_CASE("load_average basic JSON input (sysctl)") {
  json j = {{"name", "sysctl"},
            {"data",
             {{"vm",
               {{"loadavg",
                 {{"fscale", 2048},
                  {"load1", 3010},
                  {"load5", 3225},
                  {"load15", 3063}}}}}}},
            {"timestamp", 0}};

  auto c1 = wassail::check::misc::load_average_1min();
  c1.config.load = 1.0;
  auto r1 = c1.check(j);
  REQUIRE(r1->issue == wassail::result::issue_t::YES);

  auto c5 = wassail::check::misc::load_average_5min(2.0);
  auto r5 = c5.check(j);
  REQUIRE(r5->issue == wassail::result::issue_t::NO);

  auto c15 = wassail::check::misc::load_average_15min(
      1.1234, "Brief {0}", "{0} > {1}", ":shrug:", "{0} <= {1}");
  auto r15 = c15.check(j);
  REQUIRE(r15->issue == wassail::result::issue_t::YES);
  REQUIRE(r15->brief == "Brief 15");
  REQUIRE(r15->detail == "1.49561 > 1.1234");
}

TEST_CASE("load_average getloadavg input") {
  auto j = R"(
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

  wassail::data::getloadavg d = j;

  auto c1 = wassail::check::misc::load_average_1min(1.0);
  auto r1 = c1.check(d);
  REQUIRE(r1->issue == wassail::result::issue_t::YES);
  REQUIRE(r1->system_id.size() == 1);
  REQUIRE(r1->system_id[0] == "localhost.local");
  REQUIRE(r1->timestamp == std::chrono::system_clock::from_time_t(1528948131));
}

TEST_CASE("load_average sysctl input") {
  auto j = R"(
    {
      "data": {
        "hw": {
          "cpufamily": 526772277,
          "cpufrequency": 2600000000,
          "cpufrequency_max": 2600000000,
          "cputype": 7,
          "logicalcpu": 4,
          "logicalcpu_max": 4,
          "machine": "x86_64",
          "memsize": 8589934592,
          "model": "MacBookPro10,2",
          "ncpu":4,
          "packages": 1,
          "physicalcpu": 2,
          "physicalcpu_max": 2
        },
        "kern": {
          "hostname": "localhost.local",
          "osrelease": "17.5.0",
          "osrevision": 199506,
          "ostype": "Darwin",
          "osversion": "17E202",
          "version": "Darwin Kernel Version 17.5.0: Fri Apr 13 19:32:32 PDT 2018; root:xnu-4570.51.2~1/RELEASE_X86_64"
        },
        "machdep": {
          "cpu": {
            "brand_string": "Intel(R) Core(TM) i5-3230M CPU @ 2.60GHz",
            "core_count": 2,
            "cores_per_package": 8,
            "family": 6,
            "logical_per_package": 16,
            "model": 58,
            "stepping": 9,
            "vendor": "GenuineIntel"
          }
        },
        "vm": {
          "loadavg": {
            "fscale": 2048,
            "load1": 3010,
            "load15": 3225,
            "load5": 3063
          },
          "swapusage": {
            "xsu_avail": 907804672,
            "xsu_pagesize": 4096,
            "xsu_total": 1073741824,
            "xsu_used": 165937152
          }
        }
      },
      "hostname": "localhost.local",
      "name": "sysctl",
      "timestamp": 1528058096,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::sysctl d = j;

  auto c1 = wassail::check::misc::load_average_1min(1.0);
  auto r1 = c1.check(d);
  REQUIRE(r1->issue == wassail::result::issue_t::YES);
}

TEST_CASE("load_average sysinfo input") {
  auto j = R"(
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

  wassail::data::sysinfo d = j;

  auto c1 = wassail::check::misc::load_average_1min(1.0);
  auto r1 = c1.check(d);
  REQUIRE(r1->issue == wassail::result::issue_t::YES);
}
