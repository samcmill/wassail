/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ostream>
#include <wassail/result.hpp>

/* Define operation<< overloads for testing.  See
 * https://github.com/catchorg/Catch2/blob/master/docs/tostring.md */

std::ostream &operator<<(std::ostream &os,
                         enum wassail::result::issue_t const &i) {
  switch (i) {
  case wassail::result::issue_t::YES:
    os << "YES";
    break;
  case wassail::result::issue_t::NO:
    os << "NO";
    break;
  case wassail::result::issue_t::MAYBE:
    os << "MAYBE";
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
    os << "EMERGENCY";
    break;
  case wassail::result::priority_t::ALERT:
    os << "ALERT";
    break;
  case wassail::result::priority_t::ERROR:
    os << "ERROR";
    break;
  case wassail::result::priority_t::WARNING:
    os << "WARNING";
    break;
  case wassail::result::priority_t::NOTICE:
    os << "NOTICE";
    break;
  case wassail::result::priority_t::INFO:
    os << "INFO";
    break;
  case wassail::result::priority_t::DEBUG:
    os << "DEBUG";
    break;
  default:
    os << "UNKNOWN";
  }

  return os;
}
