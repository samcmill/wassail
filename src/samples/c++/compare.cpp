/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* This sample is a standalone program to demonstrate some of the
 * possible ways to perform a check, including use of the generic
 * compare check.
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
  std::cout << "The observed data is: " << std::endl
            << static_cast<json>(sysconf).dump(2) << std::endl
            << std::endl;

  /* Set the reference value to the observed number of cores */
  int ref_value = static_cast<json>(sysconf)
                      .at(json::json_pointer("/data/nprocessors_onln"))
                      .get<int>();

  /* Purpose built check */
  auto c1 = wassail::check::cpu::core_count(ref_value);
  auto r1 = c1.check(sysconf);
  std::cout << "Example 1: " << r1;

  /* Purpose built check, bad value */
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

  /* Simple generic comparison */
  auto c4 = wassail::check::compare();
  auto r4 = c4.check(sysconf, json::json_pointer("/data/nprocessors_onln"),
                     std::equal_to<int>{}, ref_value);
  std::cout << "Example 4: " << r4;

  /* Simple comparison, override default result brief */
  auto c5 = wassail::check::compare();
  auto r5 = c5.check(static_cast<json>(sysconf),
                     json::json_pointer("/data/nprocessors_onln"),
                     std::not_equal_to<int>{}, 7);
  r5->format_brief("Checking number of cores is not {0}", 7);
  std::cout << "Example 5: " << r5;

  /* Simple comparison with custom result format templates */
  auto c6 = wassail::check::compare(
      "Checking number of cores is at least {1}",
      "Obseved number of cores {0} is less than {1}",
      "Error during comparision: {0}",
      "Observed number of cores {0} is greater than or equal to {1}");
  auto r6 = c6.check(static_cast<json>(sysconf),
                     json::json_pointer("/data/nprocessors_onln"),
                     std::greater_equal<int>{}, 1);
  std::cout << "Example 6: " << r6;

  /* Use a lambda to do a regular expression comparison.  This case is
   * contrived, but it illustrates what is possible. */
  auto c7 = wassail::check::compare();
  auto r7 = c7.check(
      static_cast<json>(sysconf), json::json_pointer("/data/nprocessors_onln"),
      [](auto a, auto b) {
        return std::regex_match(std::to_string(a),
                                std::regex("^" + std::to_string(b) + "$"));
      },
      ref_value);
  r7->format_brief("Checking number of cores matches '^{0}$'", 4);
  std::cout << "Example 7: " << r7;

  /* Use a lambda to check whether the number of processors is within a range.
   * In this case, the detail message is a bit misleading, so override it. */
  int tolerance = 2;
  auto c8 = wassail::check::compare();
  c8.fmt_str.detail_yes = "Observed value {0} is not equal to {1} +/- 2";
  c8.fmt_str.detail_no = "Observed value {0} is equal to {1} +/- 2";
  auto r8 = c8.check(
      static_cast<json>(sysconf), json::json_pointer("/data/nprocessors_onln"),
      [&](auto a, auto b) { return a > tolerance - b && a <= tolerance + b; },
      4);
  r8->format_brief("Checking number of cores is between {0} and {1}",
                   4 - tolerance, 4 + tolerance);
  std::cout << "Example 8: " << r8;

  return 0;
}
