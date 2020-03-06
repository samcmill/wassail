/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* This sample is a standalone program to demonstrate some of the
 * possible ways to perform a check, including use of the rules engine.
 *
 * The sample also demonstrates overriding the default format strings
 * to customize the output messages.
 */

#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>
#include <unistd.h>
#include <wassail/wassail.hpp>

const std::string green("\033[1;32m");
const std::string red("\033[0;31m");
const std::string yellow("\033[1;33m");
const std::string reset("\033[0m");

namespace wassail {
  std::ostream &operator<<(std::ostream &os,
                           std::shared_ptr<wassail::result> const &r) {
    os << r->brief << ": " << r->issue << std::endl;
    if (!r->detail.empty()) {
      os << "  " << r->detail << std::endl;
    }
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

  /* Collect some reference data */
  auto sysconf = wassail::data::sysconf();
  sysconf.evaluate();
  json jdata = static_cast<json>(sysconf);
  std::cout << "The observed data is: " << std::endl
            << jdata.dump(2) << std::endl
            << std::endl;

  /* Set the reference value to the observed number of cores */
  int ref_value = jdata.value(json::json_pointer("/data/nprocessors_onln"), 0);

  /* Purpose built check */
  auto c1 = wassail::check::cpu::core_count(ref_value);
  auto r1 = c1.check(sysconf);
  std::cout << "Example 1: " << r1;

  /* Purpose built check, purposefully bad value */
  auto c2 = wassail::check::cpu::core_count(ref_value * 2 - 1);
  auto r2 = c2.check(sysconf);
  std::cout << "Example 2: " << r2;

  /* Purpose built check, override default format templates */
  auto c3 = wassail::check::cpu::core_count(
      ref_value, "Purpose built core count check",
      "Observed {0} cores, expected {1} cores",
      "Issue checking number of cores: '{0}'",
      "Observed and expected {0} cores");
  auto r3 = c3.check(sysconf);
  std::cout << "Example 3: " << r3;

  /* Simple custom rule */
  auto c4 = wassail::check::rules_engine();
  c4.add_rule([&](json j) {
    return j.value(json::json_pointer("/data/nprocessors_onln"), 0) ==
           ref_value;
  });
  auto r4 = c4.check(sysconf);
  std::cout << "Example 4: " << r4;

  /* Simple custom rule, manually set result brief */
  auto c5 = wassail::check::rules_engine();
  c5.add_rule([&](json j) {
    return j.value(json::json_pointer("/data/nprocessors_onln"), 0) != 7;
  });
  auto r5 = c5.check(sysconf);
  r5->brief = wassail::format("Checking number of cores is not {0}", 7);
  std::cout << "Example 5: " << r5;

  /* Simple custom rule with custom result format templates */
  auto c6 = wassail::check::rules_engine(
      "Checking number of cores is at least {1}",
      "Obseved number of cores {0} is less than {1}",
      "Error during comparision: {0}",
      "Observed number of cores {0} is greater than or equal to {1}");
  c6.add_rule([&](json j) {
    return j.value(json::json_pointer("/data/nprocessors_onln"), 0) >=
           ref_value;
  });
  auto r6 = c6.check(
      sysconf, jdata.value(json::json_pointer("/data/nprocessors_onln"), 0),
      ref_value);
  std::cout << "Example 6: " << r6;

  /* Use a rule to do a regular expression comparison.  This case is
   * contrived, but it illustrates what is possible. */
  auto c7 = wassail::check::rules_engine();
  c7.add_rule([&](json j) {
    return std::regex_match(
        std::to_string(
            j.value(json::json_pointer("/data/nprocessors_onln"), 0)),
        std::regex("^" + std::to_string(ref_value) + "$"));
  });
  auto r7 = c7.check(sysconf);
  r7->brief =
      wassail::format("Checking number of cores matches '^{0}$'", ref_value);
  std::cout << "Example 7: " << r7;

  /* Use rules to check whether the number of processors is within a range. */
  int tolerance = 2;
  auto c8 = wassail::check::rules_engine(
      "Checking range", "Observed value {0} is not equal to {1} +/- {2}",
      "Error: '{0}'", "Observed value {0} is equal to {1} +/- {2}");
  c8.add_rule([&](json j) {
    return j.value(json::json_pointer("/data/nprocessors_onln"), 0) <=
           ref_value + tolerance;
  });
  c8.add_rule([&](json j) {
    return j.value(json::json_pointer("/data/nprocessors_onln"), 0) >=
           ref_value - tolerance;
  });
  auto r8 = c8.check(
      sysconf, jdata.value(json::json_pointer("/data/nprocessors_onln"), 0),
      ref_value, tolerance);
  std::cout << "Example 8: " << r8;

  return 0;
}
