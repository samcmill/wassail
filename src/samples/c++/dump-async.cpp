/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* This sample calls the data building blocks in parallel.  Some
 * building blocks may need exclusive access to the system - the
 * data building block mutex will ensure this. If the building block
 * is enabled, then the result is printed as JSON to stdout.  Otherwise,
 * an error message is printed to stderr.
 *
 * Contrast this to the "dump" sample where the data building blocks
 * are run serially.
 */

#include <algorithm>
#include <future>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>
#include <wassail/wassail.hpp>

int main(int argc, char **argv) {
  wassail::initialize();

  std::vector<std::future<json>> futures;

  auto dump = [&](auto d) {
    d.evaluate();
    return static_cast<json>(d);
  };

  auto ready = [](auto &future) {
    return future.wait_for(std::chrono::milliseconds(0)) ==
           std::future_status::ready;
  };

  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::environment()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::getcpuid()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::getfsstat()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::getloadavg()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::getmntent()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::getrlimit()));
  futures.emplace_back(std::async(std::launch::async, dump,
                                  wassail::data::mpirun(2, "hostname")));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::nvml()));
  futures.emplace_back(std::async(std::launch::async, dump,
                                  wassail::data::osu_micro_benchmarks()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::pciaccess()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::pciutils()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::ps()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::stat("/tmp")));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::stream()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::sysconf()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::sysctl()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::sysinfo()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::udev()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::umad()));
  futures.emplace_back(
      std::async(std::launch::async, dump, wassail::data::uname()));

  while (futures.size() > 0) {
    auto it = std::find_if(futures.begin(), futures.end(), ready);

    if (it != futures.cend()) {
      auto future = std::move(*it);
      futures.erase(it);

      try {
        auto j = future.get();
        std::cout << j.dump(-1, ' ', false, json::error_handler_t::replace)
                  << std::endl;
      }
      catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
      }
    }
  }

  exit(0);
}
