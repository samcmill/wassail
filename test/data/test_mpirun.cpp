/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define CATCH_CONFIG_MAIN
#include "3rdparty/catch/catch.hpp"
#include "3rdparty/catch/catch_reporter_automake.hpp"

#include <string>
#include <unistd.h>
#include <vector>
#include <wassail/data/mpirun.hpp>

/* Some tests may fail if mpi is not setup */

TEST_CASE("mpirun basic usage") {
  auto d = wassail::data::mpirun(2, "echo 'foo'");

  if (getuid() == 0 and d.allow_run_as_root) {
    REQUIRE(d.command ==
            "mpirun -n 2 --allow-run-as-root -x MPIEXEC_TIMEOUT=60 echo 'foo'");
  }
  else {
    REQUIRE(d.command == "mpirun -n 2 -x MPIEXEC_TIMEOUT=60 echo 'foo'");
  }

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    if (j.value(json::json_pointer("/data/returncode"), 0) != 0) {
      REQUIRE(j.value(json::json_pointer("/data/stderr"), "") ==
              "/bin/sh: mpirun: command not found\n");
    }
    else {
      REQUIRE(j.value(json::json_pointer("/data/stdout"), "") == "foo\nfoo\n");
    }
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("mpirun hostfile usage") {
  auto d = wassail::data::mpirun(8, "hostfile", "a.out");

  if (getuid() == 0 and d.allow_run_as_root) {
    REQUIRE(d.command == "mpirun -n 8 -f hostfile --allow-run-as-root -x "
                         "MPIEXEC_TIMEOUT=60 a.out");
  }
  else {
    REQUIRE(d.command == "mpirun -n 8 -f hostfile -x MPIEXEC_TIMEOUT=60 a.out");
  }
}

TEST_CASE("mpirun hostlist usage") {
  auto d = wassail::data::mpirun(
      2, std::vector<std::string>({"node1", "node2"}), "a.out");

  if (getuid() == 0 and d.allow_run_as_root) {
    REQUIRE(d.command == "mpirun -n 2 -H node1,node2 --allow-run-as-root -x "
                         "MPIEXEC_TIMEOUT=60 a.out");
  }
  else {
    REQUIRE(d.command ==
            "mpirun -n 2 -H node1,node2 -x MPIEXEC_TIMEOUT=60 a.out");
  }
}

TEST_CASE("mpirun additional args") {
  auto d = wassail::data::mpirun(4, 0, "", "--bind-to core", "a.out",
                                 "-i foo.in", 10);

  if (getuid() == 0 and d.allow_run_as_root) {
    REQUIRE(d.command ==
            "mpirun -n 4 --allow-run-as-root -x MPIEXEC_TIMEOUT=10 "
            "--bind-to core a.out -i foo.in");
  }
  else {
    REQUIRE(d.command ==
            "mpirun -n 4 -x MPIEXEC_TIMEOUT=10 --bind-to core a.out -i foo.in");
  }
}

TEST_CASE("mpirun JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "command": "mpirun -n 2 osu_hello",
        "elapsed": 0.982017832,
        "mpirun_args": "",
        "mpi_impl": 0,
        "num_procs": 2,
        "per_node": 0,
        "program": "osu_hello",
        "program_args": "",
        "returncode": 0,
        "stderr": "",
        "stdout": "# OSU MPI Init Test v5.6.2\nnprocs: 2, min: 129 ms, max: 131 ms, avg: 130 ms\n"
      },
      "hostname": "localhost.local",
      "name": "mpirun",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::mpirun d = jin;
  json jout = d;

  REQUIRE(jout == jin);
}
