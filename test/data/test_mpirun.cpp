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
  auto d1 = wassail::data::mpirun(2, "echo 'foo'");

  if (getuid() == 0 and d1.allow_run_as_root) {
    REQUIRE(d1.command ==
            "mpirun -n 2 --allow-run-as-root -x MPIEXEC_TIMEOUT=60 echo 'foo'");
  }
  else {
    REQUIRE(d1.command == "mpirun -n 2 -x MPIEXEC_TIMEOUT=60 echo 'foo'");
  }

  auto d2 = wassail::data::mpirun(2, "echo 'foo'",
                                  wassail::data::mpirun::mpi_impl_t::MPICH);
  REQUIRE(d2.command == "MPIEXEC_TIMEOUT=60 mpirun -n 2 echo 'foo'");
}

TEST_CASE("mpirun hostfile usage") {
  auto d1 = wassail::data::mpirun(8, "hostfile", "a.out");

  if (getuid() == 0 and d1.allow_run_as_root) {
    REQUIRE(d1.command == "mpirun -n 8 -f hostfile --allow-run-as-root -x "
                          "MPIEXEC_TIMEOUT=60 a.out");
  }
  else {
    REQUIRE(d1.command ==
            "mpirun -n 8 -f hostfile -x MPIEXEC_TIMEOUT=60 a.out");
  }

  auto d2 = wassail::data::mpirun(8, "hostfile", "a.out",
                                  wassail::data::mpirun::mpi_impl_t::MPICH);
  REQUIRE(d2.command == "MPIEXEC_TIMEOUT=60 mpirun -n 8 -f hostfile a.out");
}

TEST_CASE("mpirun hostlist usage") {
  auto d1 = wassail::data::mpirun(
      2, std::vector<std::string>({"node1", "node2"}), "a.out");

  if (getuid() == 0 and d1.allow_run_as_root) {
    REQUIRE(d1.command == "mpirun -n 2 -H node1,node2 --allow-run-as-root -x "
                          "MPIEXEC_TIMEOUT=60 a.out");
  }
  else {
    REQUIRE(d1.command ==
            "mpirun -n 2 -H node1,node2 -x MPIEXEC_TIMEOUT=60 a.out");
  }

  auto d2 =
      wassail::data::mpirun(2, std::vector<std::string>({"node1", "node2"}),
                            "a.out", wassail::data::mpirun::mpi_impl_t::MPICH);
  REQUIRE(d2.command ==
          "MPIEXEC_TIMEOUT=60 mpirun -n 2 -hosts node1,node2 a.out");
}

TEST_CASE("mpirun additional args") {
  auto d1 = wassail::data::mpirun(4, 2, "", "--bind-to core", "a.out",
                                  "-i foo.in", 10);

  if (getuid() == 0 and d1.allow_run_as_root) {
    REQUIRE(
        d1.command ==
        "mpirun -n 4 --npernode 2 --allow-run-as-root -x MPIEXEC_TIMEOUT=10 "
        "--bind-to core a.out -i foo.in");
  }
  else {
    REQUIRE(d1.command == "mpirun -n 4 --npernode 2 -x MPIEXEC_TIMEOUT=10 "
                          "--bind-to core a.out -i foo.in");
  }

  auto d2 =
      wassail::data::mpirun(4, 2, "", "-bind-to core", "a.out", "-i foo.in", 10,
                            wassail::data::mpirun::mpi_impl_t::MPICH);

  REQUIRE(
      d2.command ==
      "MPIEXEC_TIMEOUT=10 mpirun -n 4 -ppn 2 -bind-to core a.out -i foo.in");
}

TEST_CASE("mpirun JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "mpirun_args": "",
        "mpi_impl": "openmpi",
        "num_procs": 2,
        "per_node": 0,
        "program": "osu_hello",
        "program_args": "",
        "timeout": 10
      },
      "data": {
        "command": "mpirun -n 2 osu_hello",
        "elapsed": 0.982017832,
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

TEST_CASE("mpirun common pointer JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "mpirun_args": "",
        "mpi_impl": "openmpi",
        "num_procs": 2,
        "per_node": 0,
        "program": "osu_hello",
        "program_args": "",
        "timeout": 10
      },
      "data": {
        "command": "mpirun -n 2 osu_hello",
        "elapsed": 0.982017832,
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

  std::shared_ptr<wassail::data::common> d =
      std::make_shared<wassail::data::mpirun>();

  d->from_json(jin);
  json jout = d->to_json();

  REQUIRE(jout == jin);
}

TEST_CASE("mpirun factory evaluate") {
  auto jin = R"(
    {
      "configuration": {
        "mpirun_args": "-l",
        "mpi_impl": "mpich",
        "num_procs": 4,
        "per_node": 4,
        "program": "hostname",
        "program_args": "-s",
        "timeout": 10
      },
      "name": "mpirun"
    }
  )"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "mpirun");
    REQUIRE(jout.count("data") == 1);
    REQUIRE(jout["data"]["command"].get<std::string>() ==
            "MPIEXEC_TIMEOUT=10 mpirun -n 4 -ppn 4 -l hostname -s");
  }
}
