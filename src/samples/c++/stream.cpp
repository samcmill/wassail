/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* This sample is a standalone program to demonstrate some of the possible
 * ways to perform a check, including use of the rules engine.
 *
 * The sample demonstrates comparing a value in a list of data to
 * a reference value.
 */

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <unistd.h>
#include <wassail/wassail.hpp>

const std::string green("\033[1;32m");
const std::string red("\033[0;31m");
const std::string yellow("\033[1;33m");
const std::string reset("\033[0m");

void print_result(std::ostream &os, std::shared_ptr<wassail::result> const &r,
                  int level) {
  if (not r->brief.empty()) {
    os << std::setw(level * 2) << " " << r->brief << ": " << r->issue
       << std::endl;
  }

  if (not r->detail.empty()) {
    os << std::setw(level * 2 + 2) << " " << r->detail << std::endl;
  }

  for (auto rr : r->children) {
    print_result(os, rr, level + 1);
  }
}

namespace wassail {
  std::ostream &operator<<(std::ostream &os,
                           std::shared_ptr<wassail::result> const &r) {
    print_result(os, r, 0);
    return os;
  }

  std::ostream &operator<<(std::ostream &os,
                           enum wassail::result::issue_t const &i) {
    switch (i) {
    case wassail::result::issue_t::YES:
      os << red << "NOT OK" << reset;
      break;
    case wassail::result::issue_t::NO:
      os << green << "OK" << reset;
      break;
    default:
      os << yellow << "???" << reset;
    }

    return os;
  }
} // namespace wassail

int main(int argc, char **argv) {
  wassail::initialize();

  /* Collect some reference data.  Run STREAM 3 times and save the results to a
   * list. */
  auto stream = wassail::data::stream();
  std::vector<json> streamvec;
  for (int i = 0; i < 3; i++) {
    stream.evaluate(true);
    std::cout << "Sample " << i << " is: " << std::endl
              << static_cast<json>(stream).dump(2) << std::endl
              << std::endl;
    streamvec.push_back(static_cast<json>(stream));
  }

  /* Check that all STREAM triad bandwidth values are greater than 10000 MB/s */
  auto r1 = wassail::make_result();
  r1->brief = "Checking STREAM Triad bandwidth";

  auto c1 = wassail::check::rules_engine(
      "Checking STREAM Triad bandwidth is greater than 10000 MB/s",
      "Measured STREAM Triad bandwidth of {0} MB/s is less than bandwidth "
      "threshold of 10000 MB/s",
      "Error performing check: {0}",
      "Measured STREAM Triad bandwidth of {0} MB/s is greater than or equal to "
      "bandwidth threshold of 10000 MB/s");
  c1.add_rule([](json j) {
    return j.value(json::json_pointer("/data/triad"), 0.0) >= 10000.0;
  });

  for (auto &s : streamvec) {
    auto r1i = c1.check(s, s.value(json::json_pointer("/data/triad"), 0.0));
    r1->add_child(r1i);
  }

  r1->propagate();
  std::cout << r1;

  /* Check that each STREAM triad bandwidth value is in the range 12000 +/- 500
   * MB/s */
  float tolerance = 500;
  auto outlier = [&](json j) {
    return j.value(json::json_pointer("/data/triad"), 0.0) <=
               12000.0 + tolerance &&
           j.value(json::json_pointer("/data/triad"), 0.0) >=
               12000.0 - tolerance;
  };

  auto r2 = wassail::make_result();
  r2->brief = "Checking STREAM Triad bandwidth";

  auto c2 = wassail::check::rules_engine(
      "Checking STREAM Triad bandwidth is in the range 12000 +/- 500 MB/s",
      "STREAM Triad bandwidth of {0} MB/s is outside the acceptable range",
      "Error during comparison: {0}",
      "STREAM Triad bandwidth of {0} MB/s is in the acceptable range");
  c2.add_rule(outlier);

  for (auto &s : streamvec) {
    auto r2i =
        c2.check(s, s.value(json::json_pointer("/data/triad"), 0.0), tolerance);
    r2->add_child(r2i);
  }

  r2->propagate();
  std::cout << r2;

  return 0;
}
