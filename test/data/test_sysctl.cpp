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
#include <string>
#include <wassail/data/sysctl.hpp>

TEST_CASE("sysctl basic usage") {
  auto d = wassail::data::sysctl();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    /* This is a subset of the data fields */
    REQUIRE(j["name"] == "sysctl");
    REQUIRE(j["data"]["hw"]["cpufrequency_max"].get<int64_t>() > 0);
    REQUIRE(j["data"]["hw"]["memsize"].get<int64_t>() > 0);
    REQUIRE(j["data"]["hw"]["packages"].get<int32_t>() > 0);
    REQUIRE(j["data"]["kern"]["version"].size() > 0);
    REQUIRE(j["data"]["machdep"]["cpu"]["core_count"].get<int32_t>() > 0);
    REQUIRE(j["data"]["vm"]["loadavg"]["load1"].get<uint32_t>() >= 0);
    REQUIRE(j["data"]["vm"]["swapusage"]["xsu_total"].get<uint64_t>() >= 0);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("sysctl JSON conversions") {
  auto jin = R"(
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

  wassail::data::sysctl d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);
  REQUIRE(jout == jin);
}

TEST_CASE("sysctl invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::sysctl d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("sysctl incomplete JSON conversion") {
  auto jin = R"({ "name": "sysctl", "timestamp": 0, "version": 100})"_json;
  wassail::data::sysctl d;
  REQUIRE_THROWS(d = jin);
}
