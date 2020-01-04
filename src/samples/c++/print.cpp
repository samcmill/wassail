/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iomanip>
#include <iostream>
#include <memory>
#include <wassail/result.hpp>

const std::string green("\033[1;32m");
const std::string red("\033[0;31m");
const std::string yellow("\033[1;33m");
const std::string reset("\033[0m");

void print_result(std::ostream &os, std::shared_ptr<wassail::result> const &r,
                  int level) {
  os << std::setw(level * 2) << " " << r->brief << " " << r->priority << " "
     << r->issue << std::endl;

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
    print_result(os, r, 1);
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
    case wassail::result::issue_t::MAYBE:
      os << yellow << "UNKNOWN" << reset;
      break;
    default:
      os << "UNKNOWN";
    }

    return os;
  }

  std::ostream &operator<<(std::ostream &os,
                           enum wassail::result::priority_t const &p) {
    switch (p) {
    case wassail::result::priority_t::EMERGENCY:
      os << red << "EMERGENCY" << reset;
      break;
    case wassail::result::priority_t::ALERT:
      os << red << "ALERT" << reset;
      break;
    case wassail::result::priority_t::ERROR:
      os << red << "ERROR" << reset;
      break;
    case wassail::result::priority_t::WARNING:
      os << yellow << "WARNING" << reset;
      break;
    case wassail::result::priority_t::NOTICE:
      os << green << "NOTICE" << reset;
      break;
    case wassail::result::priority_t::INFO:
      os << green << "INFO" << reset;
      break;
    case wassail::result::priority_t::DEBUG:
      os << green << "DEBUG" << reset;
      break;
    default:
      os << yellow << "UNKNOWN" << reset;
    }

    return os;
  }

} // namespace wassail
