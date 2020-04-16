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
#include <wassail/data/pciutils.hpp>

TEST_CASE("pciutils basic usage") {
  auto d = wassail::data::pciutils();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["devices"].size() > 0);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("pciutils JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "devices": [
          {
            "bus":0,
            "class_id":4096,
            "class_name":"Network and computing encryption device",
            "dev":7,
            "device_id":4101,
            "device_name":"Virtio RNG",
            "domain":0,
            "func":0,
            "slot":"00:07.0",
            "vendor_id":6900,
            "vendor_name":"Red Hat, Inc."
          },
          {
            "bus":0,
            "class_id":262,
            "class_name":"SATA controller",
            "dev":4,
            "device_id":10273,
            "device_name":"82801HR/HO/HH (ICH8R/DO/DH) 6 port SATA Controller [AHCI mode]",
            "domain":0,
            "func":0,
            "slot":"00:04.0",
            "vendor_id":32902,
            "vendor_name":"Intel Corporation"
          }
        ]
      },
      "hostname": "localhost.local",
      "name": "pciutils",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::pciutils d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("pciutils invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::pciutils d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("pciutils incomplete JSON conversion") {
  auto jin = R"({ "name": "pciutils", "timestamp": 0, "version": 100})"_json;

  wassail::data::pciutils d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "pciutils");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("devices") == 1);
  REQUIRE(jout["data"]["devices"].size() == 0);
}
