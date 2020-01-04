/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* The operator<< overloads must be included before the catch header */
#include "tostring.h"

#define CATCH_CONFIG_MAIN
#include "3rdparty/catch/catch.hpp"
#include "3rdparty/catch/catch_reporter_automake.hpp"

#include "config.h"
#include "internal.hpp"

#include <chrono>
#include <list>
#include <thread>
#include <unistd.h>

TEST_CASE("for_each") {
  std::list<std::string> l = std::list<std::string>({"a", "b"});

  auto start = std::chrono::steady_clock::now();
  wassail::internal::for_each(l.begin(), l.end(),
                              [](std::string i) { sleep(1); });
  auto end = std::chrono::steady_clock::now();

  std::chrono::duration<double> elapsed = end - start;

  if (wassail::internal::parallel() &&
      std::thread::hardware_concurrency() > 1) {
    REQUIRE(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() ==
            Approx(1).epsilon(0.01));
  }
  else {
    REQUIRE(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() ==
            Approx(2).epsilon(0.01));
  }
}
