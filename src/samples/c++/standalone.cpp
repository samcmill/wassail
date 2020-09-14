/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* This sample is a complete standalone program to check that 1) at least 5%
 * of the "/" file system is free and 2) the amount of physical memory
 * is 4GB.  Data building blocks are called to collect the raw data,
 * and the raw data is verified using check building blocks.
 *
 * The sample also demonstrates how to use results.  The individual
 * results are children of the top level result.
 */

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <wassail/wassail.hpp>

int main(int argc, char **argv) {
  wassail::initialize();

  /* overall result */
  auto overall_result = wassail::make_result();
  overall_result->brief = "Standalone wassail sample";

  /* checks */
  auto disk = wassail::check::disk::percent_free("/", 5.0);
  auto memory =
      wassail::check::memory::physical_size(4L * 1024 * 1024 * 1024, 1L * 1024);

  /* data sources */
  auto getfsstat = wassail::data::getfsstat();
  auto getmntent = wassail::data::getmntent();
  auto sysconf = wassail::data::sysconf();
  auto sysctl = wassail::data::sysctl();
  auto sysinfo = wassail::data::sysinfo();

  /* perform disk check */
  if (getfsstat.enabled()) {
    auto r = disk.check(getfsstat);
    overall_result->add_child(r);
  }
  else if (getmntent.enabled()) {
    auto r = disk.check(getmntent);
    overall_result->add_child(r);
  }
  else {
    std::cerr << "Unable to perform disk check - no data" << std::endl;
  }

  /* perform memory check */
  if (sysconf.enabled()) {
    auto r = memory.check(sysconf);
    overall_result->add_child(r);
  }
  else if (sysinfo.enabled()) {
    auto r = memory.check(sysinfo);
    overall_result->add_child(r);
  }
  else if (sysctl.enabled()) {
    auto r = memory.check(sysctl);
    overall_result->add_child(r);
  }
  else {
    std::cerr << "Unable to perform memory check - no data" << std::endl;
  }

  /* update the overall result based on the children */
  overall_result->priority = overall_result->max_priority();
  overall_result->issue = overall_result->max_issue();

  /* print the result */
  std::cout << overall_result;

  return 0;
}
