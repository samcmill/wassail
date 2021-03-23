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
#include <condition_variable>
#include <mutex>
#include <thread>
#include <wassail/data/osu_micro_benchmarks.hpp>

TEST_CASE("osu_init basic usage") {
  auto d = wassail::data::osu_micro_benchmarks();

  if (getuid() == 0 and d.allow_run_as_root) {
    REQUIRE(d.command ==
            "mpirun -n 2 --allow-run-as-root -x MPIEXEC_TIMEOUT=60 " +
                std::string(LIBEXECDIR) +
                "/osu-micro-benchmarks/mpi/startup/osu_init");
  }
  else {
    REQUIRE(d.command == "mpirun -n 2 -x MPIEXEC_TIMEOUT=60 " +
                             std::string(LIBEXECDIR) +
                             "/osu-micro-benchmarks/mpi/startup/osu_init");
  }
}

TEST_CASE("osu_hello basic usage") {
  auto d = wassail::data::osu_micro_benchmarks(
      4, wassail::data::osu_micro_benchmarks::osu_benchmark_t::HELLO);

  if (getuid() == 0 and d.allow_run_as_root) {
    REQUIRE(d.command ==
            "mpirun -n 4 --allow-run-as-root -x MPIEXEC_TIMEOUT=60 " +
                std::string(LIBEXECDIR) +
                "/osu-micro-benchmarks/mpi/startup/osu_hello");
  }
  else {
    REQUIRE(d.command == "mpirun -n 4 -x MPIEXEC_TIMEOUT=60 " +
                             std::string(LIBEXECDIR) +
                             "/osu-micro-benchmarks/mpi/startup/osu_hello");
  }
}

TEST_CASE("osu_bw basic usage") {
  auto d = wassail::data::osu_micro_benchmarks(
      2, 1, "hostfile", "",
      wassail::data::osu_micro_benchmarks::osu_benchmark_t::BW, 60);

  if (getuid() == 0 and d.allow_run_as_root) {
    REQUIRE(d.command == "mpirun -n 2 --npernode 1 -f hostfile "
                         "--allow-run-as-root -x MPIEXEC_TIMEOUT=60 " +
                             std::string(LIBEXECDIR) +
                             "/osu-micro-benchmarks/mpi/pt2pt/osu_bw");
  }
  else {
    REQUIRE(d.command ==
            "mpirun -n 2 --npernode 1 -f hostfile -x MPIEXEC_TIMEOUT=60 " +
                std::string(LIBEXECDIR) +
                "/osu-micro-benchmarks/mpi/pt2pt/osu_bw");
  }
}

TEST_CASE("osu_allreduce JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "benchmark": "osu_allreduce",
        "mpi_impl": "openmpi",
        "mpirun_args": "",
        "num_procs": 2,
        "per_node": 0,
        "timeout": 60
      },
      "data": {
        "command": "mpirun -n 2 osu_allreduce",
        "elapsed": 0.982017832,
        "returncode": 0,
        "stderr": "",
        "stdout": "# OSU MPI Allreduce Latency Test v5.6.2\n# Size       Avg Latency(us)\n4                      21.76\n8                      22.88\n16                     22.17\n32                     26.30\n64                     23.99\n128                    20.52\n256                    21.12\n512                    18.42\n1024                   14.83\n2048                   24.23\n4096                  368.40\n8192                  418.04\n16384                 639.20\n32768                 673.19\n65536                 763.51\n131072                853.01\n262144               1118.53\n524288               1897.82\n1048576              3291.21\n"
      },
      "hostname": "localhost.local",
      "name": "osu_micro_benchmarks",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::osu_micro_benchmarks d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  REQUIRE(jout["data"]["latency"].size() == 19);
  REQUIRE(jout["data"]["latency"][0]["size"] == 4);
  REQUIRE(jout["data"]["latency"][0]["latency"] == 21.76);
  REQUIRE(jout["data"]["latency"][18]["size"] == 1048576);
  REQUIRE(jout["data"]["latency"][18]["latency"] == 3291.21);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("latency");
  REQUIRE(jout == jin);
}

TEST_CASE("osu_allreduce common pointer JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "benchmark": "osu_allreduce",
        "mpi_impl": "openmpi",
        "mpirun_args": "",
        "num_procs": 2,
        "per_node": 0,
        "timeout": 60
      },
      "data": {
        "command": "mpirun -n 2 osu_allreduce",
        "elapsed": 0.982017832,
        "returncode": 0,
        "stderr": "",
        "stdout": "# OSU MPI Allreduce Latency Test v5.6.2\n# Size       Avg Latency(us)\n4                      21.76\n8                      22.88\n16                     22.17\n32                     26.30\n64                     23.99\n128                    20.52\n256                    21.12\n512                    18.42\n1024                   14.83\n2048                   24.23\n4096                  368.40\n8192                  418.04\n16384                 639.20\n32768                 673.19\n65536                 763.51\n131072                853.01\n262144               1118.53\n524288               1897.82\n1048576              3291.21\n"
      },
      "hostname": "localhost.local",
      "name": "osu_micro_benchmarks",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  std::shared_ptr<wassail::data::common> d =
      std::make_shared<wassail::data::osu_micro_benchmarks>();

  d->from_json(jin);
  json jout = d->to_json();

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  REQUIRE(jout["data"]["latency"].size() == 19);
  REQUIRE(jout["data"]["latency"][0]["size"] == 4);
  REQUIRE(jout["data"]["latency"][0]["latency"] == 21.76);
  REQUIRE(jout["data"]["latency"][18]["size"] == 1048576);
  REQUIRE(jout["data"]["latency"][18]["latency"] == 3291.21);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("latency");
  REQUIRE(jout == jin);
}

TEST_CASE("osu_alltoall JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "benchmark": "osu_alltoall",
        "mpi_impl": "openmpi",
        "mpirun_args": "",
        "num_procs": 2,
        "per_node": 0,
        "timeout": 60
      },
      "data": {
        "command": "mpirun -n 2 osu_alltoall",
        "elapsed": 0.982017832,
        "returncode": 0,
        "stderr": "",
        "stdout": "# OSU MPI All-to-All Personalized Exchange Latency Test v5.6.2\n# Size       Avg Latency(us)\n1                      24.93\n2                      21.58\n4                      22.86\n8                      21.41\n16                     21.87\n32                     20.68\n64                     19.61\n128                    22.26\n256                    23.76\n512                    24.11\n1024                   23.62\n2048                   26.17\n4096                  349.53\n8192                  361.08\n16384                 356.21\n32768                 450.85\n65536                 468.52\n131072                506.96\n262144                713.89\n524288               1261.96\n1048576              2275.21\n"
      },
      "hostname": "localhost.local",
      "name": "osu_micro_benchmarks",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::osu_micro_benchmarks d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  REQUIRE(jout["data"]["latency"].size() == 21);
  REQUIRE(jout["data"]["latency"][0]["size"] == 1);
  REQUIRE(jout["data"]["latency"][0]["latency"] == 24.93);
  REQUIRE(jout["data"]["latency"][20]["size"] == 1048576);
  REQUIRE(jout["data"]["latency"][20]["latency"] == 2275.21);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("latency");
  REQUIRE(jout == jin);
}

TEST_CASE("osu_bw JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "benchmark": "osu_bw",
        "hostfile": "hostfile",
        "mpi_impl": "openmpi",
        "mpirun_args": "",
        "num_procs": 2,
        "per_node": 1,
        "timeout": 60
      },
      "data": {
        "command": "mpirun -n 2 -npernode 1 -f hostfile osu_bw",
        "elapsed": 0.982017832,
        "returncode": 0,
        "stderr": "",
        "stdout": "# OSU MPI Bandwidth Test v5.6.2\n# Size      Bandwidth (MB/s)\n1                       1.34\n2                       4.76\n4                      10.39\n8                      21.33\n16                     36.77\n32                     75.23\n64                    148.15\n128                   211.53\n256                   267.78\n512                   563.08\n1024                 1471.49\n2048                 2365.97\n4096                  305.83\n8192                 1209.53\n16384                1675.11\n32768                2355.63\n65536                3008.32\n131072               3616.71\n262144               3957.12\n524288               4584.51\n1048576              4827.21\n2097152              4979.35\n4194304              5152.25\n"
      },
      "hostname": "localhost.local",
      "name": "osu_micro_benchmarks",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::osu_micro_benchmarks d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  REQUIRE(jout["data"]["bandwidth"].size() == 23);
  REQUIRE(jout["data"]["bandwidth"][0]["size"] == 1);
  REQUIRE(jout["data"]["bandwidth"][0]["bandwidth"] == 1.34);
  REQUIRE(jout["data"]["bandwidth"][22]["size"] == 4194304);
  REQUIRE(jout["data"]["bandwidth"][22]["bandwidth"] == 5152.25);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("bandwidth");
  REQUIRE(jout == jin);
}

TEST_CASE("osu_hello JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "benchmark": "osu_hello",
        "hostfile": "hostfile",
        "mpi_impl": "openmpi",
        "mpirun_args": "",
        "num_procs": 4,
        "per_node": 1,
        "timeout": 60
      },
      "data": {
        "command": "mpirun -n 4 -npernode 1 -f hostfile osu_hello",
        "elapsed": 0.982017832,
        "returncode": 0,
        "stderr": "",
        "stdout": "# OSU MPI Hello World Test v5.6.2\nThis is a test with 4 processes\n"
      },
      "hostname": "localhost.local",
      "name": "osu_micro_benchmarks",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::osu_micro_benchmarks d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  REQUIRE(jout["data"]["nprocs"] == 4);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("nprocs");
  REQUIRE(jout == jin);
}

TEST_CASE("osu_init JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "benchmark": "osu_init",
        "hostlist": ["node1", "node2"],
        "mpi_impl": "openmpi",
        "mpirun_args": "",
        "num_procs": 2,
        "per_node": 0,
        "timeout": 60
      },
      "data": {
        "command": "mpirun -n 2 -H node1,node2 osu_init",
        "elapsed": 0.982017832,
        "returncode": 0,
        "stderr": "",
        "stdout": "# OSU MPI Init Test v5.6.2\nnprocs: 2, min: 129 ms, max: 131 ms, avg: 130 ms\n"
      },
      "hostname": "localhost.local",
      "name": "osu_micro_benchmarks",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::osu_micro_benchmarks d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  REQUIRE(jout["data"]["nprocs"] == 2);
  REQUIRE(jout["data"]["min"] == 129);
  REQUIRE(jout["data"]["max"] == 131);
  REQUIRE(jout["data"]["avg"] == 130);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("nprocs");
  jout["data"].erase("min");
  jout["data"].erase("max");
  jout["data"].erase("avg");
  REQUIRE(jout == jin);
}

TEST_CASE("osu_latency JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "benchmark": "osu_latency",
        "hostfile": "hostfile",
        "mpi_impl": "openmpi",
        "mpirun_args": "",
        "num_procs": 2,
        "per_node": 1,
        "timeout": 60
      },
      "data": {
        "command": "mpirun -n 2 -npernode 1 -f hostfile osu_latency",
        "elapsed": 0.982017832,
        "returncode": 0,
        "stderr": "",
        "stdout": "# OSU MPI Latency Test v5.6.2\n# Size          Latency (us)\n0                       4.39\n1                       4.54\n2                       4.65\n4                       4.83\n8                       4.90\n16                      4.87\n32                      4.77\n64                      4.57\n128                     4.50\n256                     5.05\n512                     5.04\n1024                    5.43\n2048                    5.59\n4096                   75.28\n8192                   75.66\n16384                  82.91\n32768                  86.34\n65536                  94.24\n131072                103.97\n262144                129.43\n524288                191.83\n1048576               360.96\n2097152               665.89\n4194304              1221.65\n"
      },
      "hostname": "localhost.local",
      "name": "osu_micro_benchmarks",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::osu_micro_benchmarks d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  REQUIRE(jout["data"]["latency"].size() == 24);
  REQUIRE(jout["data"]["latency"][0]["size"] == 0);
  REQUIRE(jout["data"]["latency"][0]["latency"] == 4.39);
  REQUIRE(jout["data"]["latency"][23]["size"] == 4194304);
  REQUIRE(jout["data"]["latency"][23]["latency"] == 1221.65);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("latency");
  REQUIRE(jout == jin);
}

TEST_CASE("osu_reduce JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
	"benchmark": "osu_reduce",
	"mpi_impl": "openmpi",
	"mpirun_args": "",
	"num_procs": 2,
	"per_node": 0,
        "timeout": 60
      },
      "data": {
	"command": "mpirun -n 2 osu_reduce",
	"elapsed": 0.982017832,
	"returncode": 0,
	"stderr": "",
        "stdout": "# OSU MPI Reduce Latency Test v5.6.2\n# Size       Avg Latency(us)\n4                      18.93\n8                      18.16\n16                     17.87\n32                     16.61\n64                     17.35\n128                    16.34\n256                    19.53\n512                    18.39\n1024                   20.77\n2048                   22.90\n4096                  246.47\n8192                  257.09\n16384                 274.66\n32768                 276.97\n65536                 563.35\n131072                663.88\n262144               1389.99\n524288               2587.44\n1048576              5256.58\n"
      },
      "hostname": "localhost.local",
      "name": "osu_micro_benchmarks",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::osu_micro_benchmarks d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  REQUIRE(jout["data"]["latency"].size() == 19);
  REQUIRE(jout["data"]["latency"][0]["size"] == 4);
  REQUIRE(jout["data"]["latency"][0]["latency"] == 18.93);
  REQUIRE(jout["data"]["latency"][18]["size"] == 1048576);
  REQUIRE(jout["data"]["latency"][18]["latency"] == 5256.58);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("latency");
  REQUIRE(jout == jin);
}

TEST_CASE("osu_micro_benchmarks factory evaluate") {
  auto jin = R"(
    {
      "configuration": {
        "benchmark": "osu_bw",
        "mpi_impl": "openmpi",
        "num_procs": 2,
        "per_node": 2
      },
      "name": "osu_micro_benchmarks"
    }
  )"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "osu_micro_benchmarks");
    REQUIRE(jout.count("data") == 1);
    REQUIRE_THAT(
        jout["data"]["command"],
        Catch::Matches("mpirun -n 2 --npernode 2 (?:--allow-run-as-root )*-x "
                       "MPIEXEC_TIMEOUT=60 .*osu_bw"));
  }
}
