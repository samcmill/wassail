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
#include <wassail/data/nvml.hpp>

TEST_CASE("nvml basic usage") {
  auto d = wassail::data::nvml();

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    struct stat s;
    if (stat("/dev/nvidia0", &s) == 0) {
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

TEST_CASE("nvml JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "cuda_driver_version": 10010,
        "devices": [
          {
            "bar1": {
              "free": 17177178112,
              "total": 17179869184,
              "used": 2691072
            },
            "board_part_number": "000-00000-0000-000",
            "brand": 2,
            "clock": {
              "current": {
                "graphics": 135,
                "memory": 877,
                "sm": 135
              },
              "default": {
                "graphics": 1245,
                "memory": 877,
                "sm": 1245
              },
              "target": {
                "graphics": 1245,
                "memory": 877,
                "sm": 1245
              }
            },
            "compute_mode": 0,
            "cuda_compute_capability": {
              "major": 7,
              "minor": 0
            },
            "ecc": {
              "current": false,
              "errors": {
                "aggregate": {
                  "corrected": 47499235723664,
                  "uncorrected": 3251653408
                },
                "volatile": {
                  "corrected": 47499235723664,
                  "uncorrected": 2314885530279477248
                }
              },
              "pending": true
            },
            "index": 0,
            "inforom": {
              "checksum": 0,
              "ecc_version": "5.0",
              "image_version": "G500.0200.00.02",
              "oem_version": "1.1",
              "power_version": ""
            },
            "memory": {
              "free": 16945446912,
              "total": 16945512448,
              "used": 65536
            },
            "minor_number": 0,
            "name": "Tesla V100-PCIE-16GB",
            "nvlinks": [],
            "pcie": {
              "bus": 4,
              "bus_id": "00000000:04:00.0",
              "device": 0,
              "device_id": 498340062,
              "domain": 0,
              "generation": 3,
              "subsystem_id": 695649184,
              "width": 16
            },
            "pstate": 0,
            "retired_pages": {
              "double_bit": 0,
              "pending": false,
              "single_bit": 0
            },
            "serial": "0000000000000",
            "temperature": 31,
            "uuid": "GPU-00000000-0000-0000-0000-000000000000",
            "vbios": "88.00.1A.00.03"
          },
          {
            "bar1": {
              "free": 34357047296,
              "total": 34359738368,
              "used": 2691072
            },
            "board_part_number": "111-11111-1111-111",
            "brand": 2,
            "clock": {
              "current": {
                "graphics": 135,
                "memory": 877,
                "sm": 135
              },
              "default": {
                "graphics": 1230,
                "memory": 877,
                "sm": 1230
              },
              "target": {
                "graphics": 1230,
                "memory": 877,
                "sm": 1230
              }
            },
            "compute_mode": 0,
            "cuda_compute_capability": {
              "major": 7,
              "minor": 0
            },
            "ecc": {
              "current": true,
              "errors": {
                "aggregate": {
                  "corrected": 0,
                  "uncorrected": 0
                },
                "volatile": {
                  "corrected": 0,
                  "uncorrected": 0
                }
              },
              "pending": true
            },
            "index": 0,
            "inforom": {
              "checksum": 0,
              "ecc_version": "5.0",
              "image_version": "G500.0200.00.02",
              "oem_version": "1.1",
              "power_version": ""
            },
            "memory": {
              "free": 34089664512,
              "total": 34089730048,
              "used": 65536
            },
            "minor_number": 0,
            "name": "Tesla V100-PCIE-32GB",
            "nvlinks": [
              {
                "active": true,
                "version": 2
              },
              {
                "active": false,
                "version": 0
              },
              {
                "active": true,
                "version": 2
              },
              {
                "active": true,
                "version": 2
              },
              {
                "active": true,
                "version": 2
              },
              {
                "active": true,
                "version": 2
              }
            ],
            "pcie": {
              "bus": 5,
              "bus_id": "00000000:05:00.0",
              "device": 0,
              "device_id": 498471134,
              "domain": 0,
              "generation": 3,
              "subsystem_id": 2289891536,
              "width": 16
            },
            "pstate": 0,
            "retired_pages": {
              "double_bit": 0,
              "pending": false,
              "single_bit": 0
            },
            "serial": "1111111111111",
            "temperature": 33,
            "uuid": "GPU-11111111-1111-1111-1111-111111111111",
            "vbios": "88.00.48.00.02"
          }
        ],
        "driver_version": "418.67",
        "nvml_version": "10.418.67"
      },
      "hostname": "localhost.local",
      "name": "nvml",
      "timestamp": 1530420039,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::nvml d = jin;
  json jout = d;

  REQUIRE(jout.size() > 0);
  REQUIRE(jout == jin);
}

TEST_CASE("nvml invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::nvml d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("nvml incomplete JSON conversion") {
  auto jin = R"({ "name": "nvml", "timestamp": 0, "version": 100})"_json;

  wassail::data::nvml d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "nvml");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("devices") == 1);
  REQUIRE(jout["data"]["devices"].size() == 0);
}
