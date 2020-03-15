/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* This sample is a complete standalone program demonstrating several
 * approaches for using wassail.
 */

#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <unistd.h>
#include <wassail/wassail.hpp>

void usage() {
  std::cout << "wassail-environment --method N" << std::endl << std::endl;
  std::cout
      << "Sample program demonstrating several approaches for using wassail"
      << std::endl
      << std::endl;
  std::cout << "  -h, --help          Show this help message and exit"
            << std::endl;
  std::cout << "  -m N, --method N    Use method N (0-8)" << std::endl;
}

int parse_args(int argc, char **argv) {
  int method = 1;
  struct option opts[] = {{"help", no_argument, 0, 'h'},
                          {"method", required_argument, 0, 'm'},
                          {0, 0, 0, 0}};

  while (1) {
    int opt_index = 0;
    int c = getopt_long(argc, argv, "hm:", opts, &opt_index);

    if (c == -1) {
      break;
    }

    switch (c) {
    case 'h':
      usage();
      exit(1);

    case 'm':
      method = std::atoi(optarg);
      break;

    default:
      break;
    }
  }

  return method;
}

int main(int argc, char **argv) {
  int method = parse_args(argc, argv);

  /* only output errors */
  wassail::initialize(wassail::log_level::err);

  if (method == 0) {
    /* No check, just print the raw data */
    auto d = wassail::data::environment();
    d.evaluate();
    std::cout << static_cast<json>(d) << std::endl;
  }
  else if (method == 1) {
    /* Check that the environment variable FOO equals "bar" */
    auto d = wassail::data::environment();
    auto c = wassail::check::misc::environment("FOO", "bar");
    auto r = c.check(d); /* implicitly evalutes d */

    std::cout << static_cast<json>(r) << std::endl;
  }
  else if (method == 2) {
    /* Check that the environment variable FOO equals "bar" */
    auto d = wassail::data::environment();
    d.evaluate(); /* explicitly evaluate d */

    auto c = wassail::check::misc::environment("FOO", "bar");
    auto r = c.check(d); /* does not re-evaluate d */

    std::cout << r << std::endl;
  }
  else if (method == 3) {
    /* Check that the environment variable FOO equals "bar" */
    auto d = wassail::data::environment();
    d.evaluate();
    json j = d; /* convert to JSON */

    auto c = wassail::check::misc::environment("FOO", "bar");
    auto r = c.check(j); /* use JSON */

    std::cout << r << std::endl;
  }
  else if (method == 4) {
    /* Check that the environment variable PATH contains "/usr/bin" */
    auto d = wassail::data::environment();
    d.evaluate();

    auto c = wassail::check::misc::environment("PATH", "/usr/bin", true);
    auto r = c.check(d);

    std::cout << r << std::endl;
  }
  else if (method == 5) {
    /* Check that the environment variable FOO equals "bar" and
     * the environment variable PATH contains "/usr/bin"
     */
    auto d = wassail::data::environment();
    d.evaluate();

    auto c1 = wassail::check::misc::environment("FOO", "bar");
    auto r1 = c1.check(d);
    std::cout << r1 << std::endl;

    auto c2 = wassail::check::misc::environment("PATH", "/usr/bin", true);
    auto r2 = c2.check(d);
    std::cout << r2 << std::endl;
  }
  else if (method == 6) {
    /* Check that the environment variable FOO equals "bar" and
     * the environment variable PATH contains "/usr/bin"
     */
    auto d = wassail::data::environment();
    d.evaluate();
    json j = d;

    /* Create an overarching result */
    auto r = wassail::make_result(j);
    r->brief = "Checking environment";

    auto c1 = wassail::check::misc::environment("FOO", "bar");
    auto r1 = c1.check(d);
    r->add_child(r1); /* add the FOO check */

    auto c2 = wassail::check::misc::environment("PATH", "/usr/bin", true);
    auto r2 = c2.check(d);
    r->add_child(r2); /* add the PATH check */

    /* Set the overarching result priority and issue levels based on
     * the FOO and PATH checks.
     */
    r->issue = r->max_issue();
    r->priority = r->max_priority();

    std::cout << r << std::endl;
  }
  else if (method == 7) {
    /* Check that the environment variable FOO equals "bar" */
    auto d = wassail::data::environment();
    d.evaluate();
    json j = d; /* convert to JSON */

    auto c = wassail::check::rules_engine("Checking environment variable 'FOO'",
                                          "Value '{0}' does not match 'bar'",
                                          "Unable to perform comparison: '{0}'",
                                          "Value '{0}' matches 'bar'");
    c.add_rule(
        [](json j) { return j.contains(json::json_pointer("/data/FOO")); });
    c.add_rule([](json j) { return j["data"]["FOO"] == "bar"; });
    auto r = c.check(j);

    /* apply actual value to the detail format strings */
    if (r->issue == wassail::result::issue_t::YES or
        r->issue == wassail::result::issue_t::NO) {
      r->detail = wassail::format(r->detail,
                                  j.value(json::json_pointer("/data/FOO"), ""));
    }

    std::cout << r << std::endl;
  }
  else if (method == 8) {
    /* Check that environment variable FOO equals "bar" */
    auto d = wassail::data::shell_command("printenv FOO");

    auto c = wassail::check::misc::shell_output("bar\n");
    auto r = c.check(d);

    std::cout << r << std::endl;
  }
  else {
    std::cerr << "Please choose a valid method" << std::endl;
    usage();
  }

  return 0;
}
