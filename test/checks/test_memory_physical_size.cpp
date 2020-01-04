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
#include <wassail/checks/memory/physical_size.hpp>

using json = nlohmann::json;

TEST_CASE("physical_size unknown JSON") {
  json j = {
      {"name", "unknown"}, {"data", {{"memory", 4096}}}, {"timestamp", 0}};

  auto c = wassail::check::memory::physical_size(4096);

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("physical_size invalid JSON") {
  json j = {{"foo", "bar"}};

  auto c = wassail::check::memory::physical_size(4096);

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("physical_size basic JSON input (sysconf)") {
  json j = {{"name", "sysconf"},
            {"data", {{"page_size", 4096}, {"phys_pages", 2097152}}},
            {"timestamp", 0}};

  auto c = wassail::check::memory::physical_size(8589934592, 0);

  auto r = c.check(j);

  REQUIRE(r->issue == wassail::result::issue_t::NO);
}

TEST_CASE("physical_size basic JSON input (sysctl)") {
  json j = {{"name", "sysctl"},
            {"data", {{"hw", {{"memsize", 8589934592}}}}},
            {"timestamp", 0}};

  auto c = wassail::check::memory::physical_size();
  c.config.mem_size = 4L * 1024 * 1024 * 1024;

  auto r = c.check(j);

  REQUIRE(r->issue == wassail::result::issue_t::YES);
}

TEST_CASE("physical_size basic JSON input (sysinfo)") {
  json j = {{"name", "sysinfo"},
            {"data", {{"totalram", 1040621568}, {"mem_unit", 1}}},
            {"timestamp", 0}};

  auto c = wassail::check::memory::physical_size(1L * 1024 * 1024 * 1024,
                                                 64L * 1024 * 1024);

  auto r = c.check(j);

  REQUIRE(r->issue == wassail::result::issue_t::NO);
}

TEST_CASE("physical_size sysconf input") {
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

  auto c = wassail::check::memory::physical_size(8589934592, 0);
  c.fmt_str.brief = "Brief";
  c.fmt_str.detail_no = "{0} {3} == {1} +/- {2} {3}";

  auto r = c.check(d);

  REQUIRE(r->issue == wassail::result::issue_t::NO);
  REQUIRE(r->brief == "Brief");
  REQUIRE(r->detail == "8589934592 bytes == 8589934592 +/- 0 bytes");
  REQUIRE(r->system_id.size() == 1);
  REQUIRE(r->system_id[0] == "localhost.local");
  REQUIRE(r->timestamp == std::chrono::system_clock::from_time_t(1528057219));

  auto c2 = wassail::check::memory::physical_size(
      8589934500, 64, "Something brief", "{0} {3} != {1} +/- {2} {3}",
      ":shrug:", "{0} {1} {2} {3}");
  auto r2 = c2.check(d);

  REQUIRE(r2->issue == wassail::result::issue_t::YES);
  REQUIRE(r2->brief == "Something brief");
  REQUIRE(r2->detail == "8589934592 bytes != 8589934500 +/- 64 bytes");
}

TEST_CASE("physical_size sysctl input") {
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

  auto c = wassail::check::memory::physical_size(8L * 1024 * 1024 * 1024);

  auto r = c.check(d);

  REQUIRE(r->issue == wassail::result::issue_t::NO);
}

TEST_CASE("physical_size sysinfo input") {
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

  auto c = wassail::check::memory::physical_size(8L * 1024 * 1024 * 1024);

  auto r = c.check(d);

  REQUIRE(r->issue == wassail::result::issue_t::YES);
}
