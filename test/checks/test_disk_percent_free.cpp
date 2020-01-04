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
#include <wassail/checks/disk/percent_free.hpp>

using json = nlohmann::json;

TEST_CASE("percent_free unknown JSON") {
  json j = {{"name", "unknown"}, {"data", {{"avail", 4096}}}, {"timestamp", 0}};

  auto c = wassail::check::disk::percent_free("/mnt", 4096);

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("percent_free invalid JSON") {
  json j = {{"foo", "bar"}};

  auto c = wassail::check::disk::percent_free("/mnt", 4096);

  REQUIRE_THROWS(c.check(j));
}

TEST_CASE("percent_free basic JSON input (getfsstat)") {
  auto j = R"(
    {
      "data": {
        "file_systems": [
          {
            "bavail": 5771575,
            "bfree": 6722834,
            "blocks": 61228134,
            "bsize": 4096,
            "ffree": 9223372036853658231,
            "files": 9223372036854775807,
            "flags": 75550720,
            "fstypename": "apfs",
            "mntfromname": "/dev/disk1s1",
            "mntonname": "/",
            "owner": 0
          }
        ]
      },
      "hostname": "localhost.local",
      "name": "getfsstat",
      "timestamp": 1529033551,
      "uid": 99,
      "version": 100
    }
  )"_json;

  /* percent = 100 * 5771575 / 61228134 = 9.43% */
  auto c1 = wassail::check::disk::percent_free("/", 9.4);
  auto r1 = c1.check(j);
  REQUIRE(r1->issue == wassail::result::issue_t::NO);
  REQUIRE(r1->system_id.size() == 1);
  REQUIRE(r1->system_id[0] == "localhost.local");
  REQUIRE(r1->timestamp == std::chrono::system_clock::from_time_t(1529033551));

  auto c2 = wassail::check::disk::percent_free("/", 9.5);
  auto r2 = c2.check(j);
  REQUIRE(r2->issue == wassail::result::issue_t::YES);

  auto c3 = wassail::check::disk::percent_free("/invalid", 10.0);
  auto r3 = c3.check(j);
  REQUIRE(r3->issue == wassail::result::issue_t::MAYBE);
}

TEST_CASE("percent_free basic getfsstat input") {
  auto j = R"(
    {
      "data": {
        "file_systems": [
          {
            "bavail": 5771575,
            "bfree": 6722834,
            "blocks": 61228134,
            "bsize": 4096,
            "ffree": 9223372036853658231,
            "files": 9223372036854775807,
            "flags": 75550720,
            "fstypename": "apfs",
            "mntfromname": "/dev/disk1s1",
            "mntonname": "/",
            "owner": 0
          }
        ]
      },
      "hostname": "localhost.local",
      "name": "getfsstat",
      "timestamp": 1529033551,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::getfsstat d = j;

  /* percent = 100 * 5771575 / 61228134 = 9.43% */
  auto c1 = wassail::check::disk::percent_free("/");
  c1.fmt_str.brief = "Brief {0}";
  c1.fmt_str.detail_no = "{0:.1f} >= {1:.1f}";
  auto r1 = c1.check(d);

  REQUIRE(r1->issue == wassail::result::issue_t::NO);
  REQUIRE(r1->brief == "Brief /");
  REQUIRE(r1->detail == "9.4 >= 0.0");

  auto c2 = wassail::check::disk::percent_free("/", 100, "Something brief {0}",
                                               "{0:.1f} < {1:.1f}", "{0}",
                                               "{0} >= {1}");
  auto r2 = c2.check(d);

  REQUIRE(r2->issue == wassail::result::issue_t::YES);
  REQUIRE(r2->brief == "Something brief /");
  REQUIRE(r2->detail == "9.4 < 100.0");
}

TEST_CASE("percent_free basic JSON input (getmntent)") {
  auto j = R"(
    {
      "data": {
        "file_systems": [
          {
            "bavail": 76420,
            "bfree": 76420,
            "blocks": 1621504,
            "bsize": 4096,
            "dir": "/",
            "favail": 64768,
            "ffree": 611763,
            "files": 778784,
            "flag": 4096,
            "frsize": 4096,
            "fsid": 64768,
            "fsname": "/dev/mapper/centos_centos7-root",
            "type": "xfs"
          }
        ]
      },
      "hostname": "localhost.local",
      "name": "getmntent",
      "timestamp": 1529033551,
      "uid": 99,
      "version": 100
    }
  )"_json;

  /* percent = 100 * 76420 / 1621504 = 4.71% */
  auto c1 = wassail::check::disk::percent_free("/", 4.6);
  auto r1 = c1.check(j);
  REQUIRE(r1->issue == wassail::result::issue_t::NO);

  auto c2 = wassail::check::disk::percent_free("/", 4.8);
  auto r2 = c2.check(j);
  REQUIRE(r2->issue == wassail::result::issue_t::YES);

  auto c3 = wassail::check::disk::percent_free("/invalid", 10.0);
  auto r3 = c3.check(j);
  REQUIRE(r3->issue == wassail::result::issue_t::MAYBE);
}

TEST_CASE("percent_free basic getmntent input") {
  auto j = R"(
    {
      "data": {
        "file_systems": [
          {
            "bavail": 76420,
            "bfree": 76420,
            "blocks": 1621504,
            "bsize": 4096,
            "dir": "/",
            "favail": 64768,
            "ffree": 611763,
            "files": 778784,
            "flag": 4096,
            "frsize": 4096,
            "fsid": 64768,
            "fsname": "/dev/mapper/centos_centos7-root",
            "type": "xfs"
          }
        ]
      },
      "hostname": "localhost.local",
      "name": "getmntent",
      "timestamp": 1529033551,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::getmntent d = j;

  /* percent = 100 * 76420 / 1621504 = 4.71% */
  auto c1 = wassail::check::disk::percent_free("/");
  c1.fmt_str.brief = "Brief {0}";
  c1.fmt_str.detail_no = "{0:.1f} >= {1:.1f}";
  auto r1 = c1.check(d);

  REQUIRE(r1->issue == wassail::result::issue_t::NO);
  REQUIRE(r1->brief == "Brief /");
  REQUIRE(r1->detail == "4.7 >= 0.0");

  auto c2 = wassail::check::disk::percent_free("/", 100, "Something brief {0}",
                                               "{0:.1f} < {1:.1f}", "{0}",
                                               "{0} >= {1}");
  auto r2 = c2.check(d);

  REQUIRE(r2->issue == wassail::result::issue_t::YES);
  REQUIRE(r2->brief == "Something brief /");
  REQUIRE(r2->detail == "4.7 < 100.0");
}
