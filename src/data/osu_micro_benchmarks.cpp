/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <string>
#include <wassail/data/osu_micro_benchmarks.hpp>

namespace wassail {
  namespace data {
    std::string osu_micro_benchmarks::osu_program(
        osu_micro_benchmarks::osu_benchmark_t osu_benchmark,
        std::string libexecdir) {
      switch (osu_benchmark) {
      case osu_micro_benchmarks::osu_benchmark_t::ALLREDUCE: {
        return libexecdir +
               "/osu-micro-benchmarks/mpi/collective/osu_allreduce";
        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::ALLTOALL: {
        return libexecdir + "/osu-micro-benchmarks/mpi/collective/osu_alltoall";
        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::BW: {
        return libexecdir + "/osu-micro-benchmarks/mpi/pt2pt/osu_bw";
        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::HELLO: {
        return libexecdir + "/osu-micro-benchmarks/mpi/startup/osu_hello";
        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::INIT: {
        return libexecdir + "/osu-micro-benchmarks/mpi/startup/osu_init";
        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::LATENCY: {
        return libexecdir + "/osu-micro-benchmarks/mpi/pt2pt/osu_latency";
        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::REDUCE: {
        return libexecdir + "/osu-micro-benchmarks/mpi/collective/osu_reduce";
        break;
      }
      default: { throw std::runtime_error("unknown OSU Micro-Benchmark"); }
      }
    }

    void from_json(const json &j, osu_micro_benchmarks &d) {
      if (j.value("version", 0) != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<mpirun &>(d));

      std::string benchmark =
          j.value(json::json_pointer("/data/benchmark"), "");

      if (benchmark == "osu_allreduce") {
        d.osu_benchmark = osu_micro_benchmarks::osu_benchmark_t::ALLREDUCE;
      }
      else if (benchmark == "osu_alltoall") {
        d.osu_benchmark = osu_micro_benchmarks::osu_benchmark_t::ALLTOALL;
      }
      else if (benchmark == "osu_bw") {
        d.osu_benchmark = osu_micro_benchmarks::osu_benchmark_t::BW;
      }
      else if (benchmark == "osu_hello") {
        d.osu_benchmark = osu_micro_benchmarks::osu_benchmark_t::HELLO;
      }
      else if (benchmark == "osu_init") {
        d.osu_benchmark = osu_micro_benchmarks::osu_benchmark_t::INIT;
      }
      else if (benchmark == "osu_latency") {
        d.osu_benchmark = osu_micro_benchmarks::osu_benchmark_t::LATENCY;
      }
      else if (benchmark == "osu_reduce") {
        d.osu_benchmark = osu_micro_benchmarks::osu_benchmark_t::REDUCE;
      }
      else {
        throw std::runtime_error(std::string("unknown OSU Micro-Benchmark: ") +
                                 benchmark);
      }
    }

    void to_json(json &j, const osu_micro_benchmarks &d) {
      j = dynamic_cast<const mpirun &>(d);

      std::string stdout = j.value(json::json_pointer("/data/stdout"), "");

      using iter = std::regex_iterator<std::string::const_iterator>;
      auto apply_regex = [](std::string s, std::regex re,
                            std::function<void(iter it)> f) {
        for (auto it = std::sregex_iterator(s.begin(), s.end(), re);
             it != std::sregex_iterator(); ++it) {
          f(it);
        }
      };

      switch (d.osu_benchmark) {
      case osu_micro_benchmarks::osu_benchmark_t::ALLREDUCE: {
        j["data"]["benchmark"] = "osu_allreduce";

        std::regex re("(\\d+)\\s+(\\d+\\.\\d+)");

        j["data"]["latency"] = json::array();

        apply_regex(stdout, re, [&j](iter it) {
          j["data"]["latency"].push_back(
              {{"size", std::atoi(it->str(1).c_str())},
               {"latency", std::atof(it->str(2).c_str())}});
        });

        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::ALLTOALL: {
        j["data"]["benchmark"] = "osu_alltoall";

        std::regex re("(\\d+)\\s+(\\d+\\.\\d+)");

        j["data"]["latency"] = json::array();

        apply_regex(stdout, re, [&j](iter it) {
          j["data"]["latency"].push_back(
              {{"size", std::atoi(it->str(1).c_str())},
               {"latency", std::atof(it->str(2).c_str())}});
        });

        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::BW: {
        j["data"]["benchmark"] = "osu_bw";

        std::regex re("(\\d+)\\s+(\\d+\\.\\d+)");

        j["data"]["bandwidth"] = json::array();

        apply_regex(stdout, re, [&j](iter it) {
          j["data"]["bandwidth"].push_back(
              {{"size", std::atoi(it->str(1).c_str())},
               {"bandwidth", std::atof(it->str(2).c_str())}});
        });

        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::HELLO: {
        j["data"]["benchmark"] = "osu_hello";

        std::regex re("This is a test with (\\d+) processes");

        apply_regex(stdout, re, [&j](iter it) {
          j["data"]["nprocs"] = std::atoi(it->str(1).c_str());
        });

        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::INIT: {
        j["data"]["benchmark"] = "osu_init";

        std::regex re("nprocs: (\\d+), min: (\\d+) ms, max: (\\d+) ms, avg: "
                      "(\\d+) ms\\n");

        apply_regex(stdout, re, [&j](iter it) {
          j["data"]["nprocs"] = std::atoi(it->str(1).c_str());
          j["data"]["min"] = std::atoi(it->str(2).c_str());
          j["data"]["max"] = std::atoi(it->str(3).c_str());
          j["data"]["avg"] = std::atoi(it->str(4).c_str());
        });

        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::LATENCY: {
        j["data"]["benchmark"] = "osu_latency";

        std::regex re("(\\d+)\\s+(\\d+\\.\\d+)");

        j["data"]["latency"] = json::array();

        apply_regex(stdout, re, [&j](iter it) {
          j["data"]["latency"].push_back(
              {{"size", std::atoi(it->str(1).c_str())},
               {"latency", std::atof(it->str(2).c_str())}});
        });

        break;
      }
      case osu_micro_benchmarks::osu_benchmark_t::REDUCE: {
        j["data"]["benchmark"] = "osu_reduce";

        std::regex re("(\\d+)\\s+(\\d+\\.\\d+)");

        j["data"]["latency"] = json::array();

        apply_regex(stdout, re, [&j](iter it) {
          j["data"]["latency"].push_back(
              {{"size", std::atoi(it->str(1).c_str())},
               {"latency", std::atof(it->str(2).c_str())}});
        });

        break;
      }
      default: { throw std::runtime_error("unknown OSU Micro-Benchmark"); }
      }
    }
  } // namespace data
} // namespace wassail
