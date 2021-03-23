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
#include <sys/stat.h>
#include <wassail/data/umad.hpp>

TEST_CASE("umad basic usage") {
  auto d = wassail::data::umad();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    struct stat s;
    if (stat("/sys/class/infiniband_mad/abi_version", &s) == 0) {
      /* will only pass on machines that actually have a device */
      REQUIRE(j["data"]["devices"].size() > 0);
    }
    else {
      REQUIRE(j["data"]["devices"].size() == 0);
    }
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("umad JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "devices": [
          {
            "ca_type": "MT4113",
            "fw_ver": "10.16.1006",
            "hw_ver": "0",
            "name": "mlx5_0",
            "node_guid": 1000000000000000000,
            "node_type": 1,
            "numports": 2,
            "ports": [
              {
                "base_lid": 14,
                "ca_name": "mlx5_0",
                "capmask": 1214796070,
                "gid_prefix": 33022,
                "link_layer": "InfiniBand",
                "lmc": 0,
                "phys_state": 5,
                "port_guid": 4660669261498097124,
                "portnum": 1,
                "rate": 56,
                "sm_lid": 1,
                "sm_sl": 0,
                "state": 4
              },
              {
                "base_lid": 65535,
                "ca_name": "mlx5_0",
                "capmask": 1214796070,
                "gid_prefix": 33022,
                "link_layer": "InfiniBand",
                "lmc": 0,
                "phys_state": 3,
                "port_guid": 5237130013801520612,
                "portnum": 2,
                "rate": 10,
                "sm_lid": 0,
                "sm_sl": 0,
                "state": 1
              }
            ],
            "system_guid": 1000000000000000000
          }
        ]
      },
      "hostname": "localhost.local",
      "name": "umad",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::umad d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("umad common pointer JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "devices": [
          {
            "ca_type": "MT4113",
            "fw_ver": "10.16.1006",
            "hw_ver": "0",
            "name": "mlx5_0",
            "node_guid": 1000000000000000000,
            "node_type": 1,
            "numports": 2,
            "ports": [
              {
                "base_lid": 14,
                "ca_name": "mlx5_0",
                "capmask": 1214796070,
                "gid_prefix": 33022,
                "link_layer": "InfiniBand",
                "lmc": 0,
                "phys_state": 5,
                "port_guid": 4660669261498097124,
                "portnum": 1,
                "rate": 56,
                "sm_lid": 1,
                "sm_sl": 0,
                "state": 4
              },
              {
                "base_lid": 65535,
                "ca_name": "mlx5_0",
                "capmask": 1214796070,
                "gid_prefix": 33022,
                "link_layer": "InfiniBand",
                "lmc": 0,
                "phys_state": 3,
                "port_guid": 5237130013801520612,
                "portnum": 2,
                "rate": 10,
                "sm_lid": 0,
                "sm_sl": 0,
                "state": 1
              }
            ],
            "system_guid": 1000000000000000000
          }
        ]
      },
      "hostname": "localhost.local",
      "name": "umad",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  std::shared_ptr<wassail::data::common> d =
      std::make_shared<wassail::data::umad>();

  d->from_json(jin);
  json jout = d->to_json();

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("umad invalid JSON conversion") {
  auto jin = R"({ "name": "invalid" })"_json;
  wassail::data::umad d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("umad incomplete JSON conversion") {
  auto jin = R"({ "name": "umad", "timestamp": 0, "version": 100})"_json;

  wassail::data::umad d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "umad");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("devices") == 1);
  REQUIRE(jout["data"]["devices"].size() == 0);
}

TEST_CASE("umad factory evaluate") {
  auto jin = R"({ "name": "umad" })"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "umad");
    REQUIRE(jout.count("data") == 1);

    struct stat s;
    if (stat("/sys/class/infiniband_mad/abi_version", &s) == 0) {
      /* will only pass on machines that actually have a device */
      REQUIRE(jout["data"]["devices"].size() > 0);
    }
    else {
      REQUIRE(jout["data"]["devices"].size() == 0);
    }
  }
}
