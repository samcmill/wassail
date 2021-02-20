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
#include <cstdlib>
#include <string>
#include <wassail/data/udev.hpp>

TEST_CASE("udev basic usage") {
  auto d = wassail::data::udev();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"].size() > 0);
    REQUIRE(j["data"]["sys"]["devices"].size() > 0);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("udev JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "sys": {
          "devices": {
            "virtual": {
              "net": {
                "eth0": {
                  "addr_assign_type": "3",
                  "addr_len": "6",
                  "address": "02:42:ac:11:00:02",
                  "broadcast": "ff:ff:ff:ff:ff:ff",
                  "carrier": "1",
                  "carrier_changes": "2",
                  "carrier_down_count": "1",
                  "carrier_up_count": "1",
                  "dev_id": "0x0",
                  "dev_port": "0",
                  "dormant": "0",
                  "duplex": "full",
                  "flags": "0x1003",
                  "gro_flush_timeout": "0",
                  "ifalias": "",
                  "ifindex": "34",
                  "iflink": "35",
                  "link_mode": "0",
                  "mtu": "1500",
                  "name_assign_type": "4",
                  "netdev_group": "0",
                  "operstate": "up",
                  "phys_port_id": null,
                  "phys_port_name": null,
                  "phys_switch_id": null,
                  "proto_down": "0",
                  "speed": "10000",
                  "subsystem": "net",
                  "tx_queue_len": "0",
                  "type": "1",
                  "uevent": "INTERFACE=eth0\nIFINDEX=34"
                },
                "lo": {
                  "addr_assign_type": "0",
                  "addr_len": "6",
                  "address": "00:00:00:00:00:00",
                  "broadcast": "00:00:00:00:00:00",
                  "carrier": "1",
                  "carrier_changes": "0",
                  "carrier_down_count": "0",
                  "carrier_up_count": "0",
                  "dev_id": "0x0",
                  "dev_port": "0",
                  "dormant": "0",
                  "duplex": null,
                  "flags": "0x9",
                  "gro_flush_timeout": "0",
                  "ifalias": "",
                  "ifindex": "1",
                  "iflink": "1",
                  "link_mode": "0",
                  "mtu": "65536",
                  "name_assign_type": null,
                  "netdev_group": "0",
                  "operstate": "unknown",
                  "phys_port_id": null,
                  "phys_port_name": null,
                  "phys_switch_id": null,
                  "proto_down": "0",
                  "speed": null,
                  "subsystem": "net",
                  "tx_queue_len": "1000",
                  "type": "772",
                  "uevent": "INTERFACE=lo\nIFINDEX=1"
                }
              }
            }
          }
        }
      },
      "hostname": "localhost.local",
      "name": "udev",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::udev d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("udev invalid JSON conversion") {
  auto jin = R"({ "name": "invalid" })"_json;
  wassail::data::udev d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("udev incomplete JSON conversion") {
  auto jin = R"({ "name": "udev", "timestamp": 0, "version": 100})"_json;

  wassail::data::udev d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "udev");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].size() == 0);
}

TEST_CASE("udev factory evaluate") {
  auto jin = R"({ "name": "udev" })"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "udev");
    REQUIRE(jout.count("data") == 1);
    REQUIRE(jout["data"]["sys"]["devices"].size() > 0);
  }
}
