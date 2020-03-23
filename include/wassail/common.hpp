/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_COMMON_HPP
#define _WASSAIL_COMMON_HPP

#include <memory>
#include <string>
#include <wassail/fmt/format.h>

/* Generate Doxygen documentation */
/*! \file */

namespace wassail {
  /* duplicate of spdlog::level::level_enum */
  enum log_level {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    err = 4,
    critical = 5,
    off = 6
  };

  /*! \brief Initialize the wassail library
   *
   * Every program using the wassail library must call this function
   * before any other wassail calls
   *
   * \param[in] log_level log level (default: warning)
   */
  void initialize(log_level log_level = log_level::warn);

  /*! \brief Format args according to the format string fmt and return the
   * result as a string.
   *
   * Implementation of C++20 std::format using fmtlib.  The format template
   * syntax description can be found at http://fmtlib.net/latest/syntax.html.
   *
   * \param[in] fmt format template
   * \param[in] args argument list
   * \return formatted string
   */
  template <typename... T>
  std::string format(const std::string &fmt, const T &... args) {
    return fmt::vformat(fmt, {fmt::make_format_args(args...)});
  }

  /*! \brief Return the wassail library version */
  std::string version();

  /*! \brief Return the wassail library major version number */
  uint16_t version_major();

  /*! \brief Return the wassail library minor version number */
  uint16_t version_minor();

  /*! \brief Return the wassail library micro version number */
  uint16_t version_micro();
} // namespace wassail

#endif
