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
#include <wassail/data/pciaccess.hpp>

TEST_CASE("pciaccess basic usage") {
  auto d = wassail::data::pciaccess();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["devices"].size() > 0);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("pciaccess JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "devices": [
          {
            "bus":0,
            "class_id":4096,
            "dev":7,
            "device_id":4101,
            "device_name":"Virtio RNG",
            "domain":0,
            "func":0,
            "irq":0,
            "revision":0,
            "slot":"00:07.0",
            "subdevice_id":4,
            "subdevice_name":"",
            "subvendor_id":6900,
            "subvendor_name":"Red Hat, Inc.",
            "vendor_id":6900,
            "vendor_name":"Red Hat, Inc."
          },
          {
            "bus":0,
            "class_id":262,
            "dev":4,
            "device_id":10273,
            "device_name":"82801HR/HO/HH (ICH8R/DO/DH) 6 port SATA Controller [AHCI mode]",
            "domain":0,
            "func":0,
            "irq":0,
            "revision":0,
            "slot":"00:04.0",
            "subdevice_id":0,
            "subdevice_name":"",
            "subvendor_id":0,
            "subvendor_name":"",
            "vendor_id":32902,
            "vendor_name":"Intel Corporation"
          }
        ]
      },
      "hostname": "localhost.local",
      "name": "pciaccess",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::pciaccess d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("pciaccess common pointer JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "devices": [
          {
            "bus":0,
            "class_id":4096,
            "dev":7,
            "device_id":4101,
            "device_name":"Virtio RNG",
            "domain":0,
            "func":0,
            "irq":0,
            "revision":0,
            "slot":"00:07.0",
            "subdevice_id":4,
            "subdevice_name":"",
            "subvendor_id":6900,
            "subvendor_name":"Red Hat, Inc.",
            "vendor_id":6900,
            "vendor_name":"Red Hat, Inc."
          },
          {
            "bus":0,
            "class_id":262,
            "dev":4,
            "device_id":10273,
            "device_name":"82801HR/HO/HH (ICH8R/DO/DH) 6 port SATA Controller [AHCI mode]",
            "domain":0,
            "func":0,
            "irq":0,
            "revision":0,
            "slot":"00:04.0",
            "subdevice_id":0,
            "subdevice_name":"",
            "subvendor_id":0,
            "subvendor_name":"",
            "vendor_id":32902,
            "vendor_name":"Intel Corporation"
          }
        ]
      },
      "hostname": "localhost.local",
      "name": "pciaccess",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  std::shared_ptr<wassail::data::common> d =
      std::make_shared<wassail::data::pciaccess>();

  d->from_json(jin);
  json jout = d->to_json();

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("pciaccess invalid JSON conversion") {
  auto jin = R"({ "name": "invalid" })"_json;
  wassail::data::pciaccess d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("pciaccess incomplete JSON conversion") {
  auto jin = R"({ "name": "pciaccess", "timestamp": 0, "version": 100})"_json;

  wassail::data::pciaccess d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "pciaccess");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("devices") == 1);
  REQUIRE(jout["data"]["devices"].size() == 0);
}

TEST_CASE("pciaccess factory evaluate") {
  auto jin = R"({ "name": "pciaccess" })"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "pciaccess");
    REQUIRE(jout.count("data") == 1);
    REQUIRE(jout["data"]["devices"].size() > 0);
  }
}
