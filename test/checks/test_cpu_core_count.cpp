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
#include <wassail/checks/cpu/core_count.hpp>

using json = nlohmann::json;

TEST_CASE("core_count unknown JSON") {
  json j = {{"name", "unknown"},
            {"data", {{"core_count", 4}}},
            {"hostname", "localhost"},
            {"timestamp", 0}};

  auto c = wassail::check::cpu::core_count(4);

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("core_count invalid JSON") {
  json j = {{"foo", "bar"}};

  auto c = wassail::check::cpu::core_count(4);

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("core_count basic JSON input (sysconf)") {
  json j = {{"name", "sysconf"},
            {"data", {{"nprocessors_onln", 4}}},
            {"hostname", "localhost"},
            {"timestamp", 0}};

  auto c = wassail::check::cpu::core_count();
  c.config.num_cores = 8;

  auto r = c.check(j);

  REQUIRE(r->issue == wassail::result::issue_t::YES);
}

TEST_CASE("core_count basic JSON input (sysctl)") {
  json j = {{"name", "sysctl"},
            {"data", {{"machdep", {{"cpu", {{"core_count", 8}}}}}}},
            {"hostname", "localhost"},
            {"timestamp", 0}};

  auto c = wassail::check::cpu::core_count();
  c.config.num_cores = 4;

  auto r = c.check(j);

  REQUIRE(r->issue == wassail::result::issue_t::YES);
}

TEST_CASE("core_count sysconf input") {
  auto j = R"(
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

  wassail::data::sysconf d = j;

  auto c = wassail::check::cpu::core_count(4);
  c.fmt_str.brief = "Brief";
  c.fmt_str.detail_no = "{0} == {1}";

  auto r = c.check(d);

  REQUIRE(r->issue == wassail::result::issue_t::NO);
  REQUIRE(r->brief == "Brief");
  REQUIRE(r->detail == "4 == 4");
  REQUIRE(r->system_id.size() == 1);
  REQUIRE(r->system_id[0] == "localhost.local");
  REQUIRE(r->timestamp == std::chrono::system_clock::from_time_t(1528057219));

  auto c2 = wassail::check::cpu::core_count(8, "Something brief", "{0} != {1}",
                                            ":shrug:", "{0} == {1}");
  auto r2 = c2.check(d);

  REQUIRE(r2->issue == wassail::result::issue_t::YES);
  REQUIRE(r2->brief == "Something brief");
  REQUIRE(r2->detail == "4 != 8");
  REQUIRE(r2->system_id.size() == 1);
  REQUIRE(r2->system_id[0] == "localhost.local");
  REQUIRE(r2->timestamp == std::chrono::system_clock::from_time_t(1528057219));
}

TEST_CASE("core_count sysctl input") {
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

  auto c = wassail::check::cpu::core_count(2);

  auto r = c.check(d);

  REQUIRE(r->issue == wassail::result::issue_t::NO);
}
