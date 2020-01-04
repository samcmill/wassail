/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* This sample reads standard input for matching JSON strings. A match
 * is based on the "name" field. When a match is detected, the corresponding
 * check is performed. Checks verify 1) at least 5% of the "/" filesystem
 * is free, 2) at least 4 MB of the "/" filesystem is free, and 3) the amount
 * of physical memory is 4 GB.
 *
 * Pipe the output from the "dump" sample to this sample to demonstate
 * a potential workflow.
 */

#include <exception>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <wassail/wassail.hpp>

int main(int argc, char **argv) {
  wassail::initialize();

  /* map the data source name to the check */
  std::map<std::string, std::list<std::unique_ptr<wassail::check::common>>>
      checks;

  /* memory checks */
  /* sysconf data source */
  auto memory1 = std::make_unique<wassail::check::memory::physical_size>(
      4L * 1024 * 1024 * 1024, 1L * 1024);
  checks["sysconf"].push_back(std::move(memory1));

  /* sysctl data source */
  auto memory2 = std::make_unique<wassail::check::memory::physical_size>(
      4L * 1024 * 1024 * 1024, 1L * 1024);
  checks["sysctl"].push_back(std::move(memory2));

  /* sysinfo data source */
  auto memory3 = std::make_unique<wassail::check::memory::physical_size>(
      4L * 1024 * 1024 * 1024, 1L * 1024);
  checks["sysinfo"].push_back(std::move(memory3));

  /* disk checks */
  /* getfsstat data source */
  auto disk1a = std::make_unique<wassail::check::disk::percent_free>("/", 5.0);
  checks["getfsstat"].push_back(std::move(disk1a));

  auto disk1b = std::make_unique<wassail::check::disk::amount_free>(
      "/", 4L * 1024 * 1024);
  checks["getfsstat"].push_back(std::move(disk1b));

  /* getmntent data source */
  auto disk2a = std::make_unique<wassail::check::disk::percent_free>("/", 5.0);
  checks["getmntent"].push_back(std::move(disk2a));

  auto disk2b = std::make_unique<wassail::check::disk::amount_free>(
      "/", 4L * 1024 * 1024);
  checks["getmntent"].push_back(std::move(disk2b));

  /* read JSON from standard input */
  for (std::string json_string; std::getline(std::cin, json_string);) {
    try {
      auto j = json::parse(json_string);

      /* look for a match */
      auto match = checks.find(j.at("name"));
      if (match != checks.end()) {
        for (auto &c : match->second) {
          /* perform check */
          auto r = c->check(j);
          std::cout << r;
        }
      }
    }
    catch (std::exception &e) {
      std::cerr << "Not a valid JSON string '" << json_string
                << "': " << e.what() << std::endl;
    }
  }

  exit(0);
}
