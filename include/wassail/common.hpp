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

  /*! \brief Basic interface to the internal logger
   *
   * This function is provided so that templated code and other
   * public interfaces can use the internal logger without exposing
   * the details of the internal logger interface.  This should not
   * be used by user code.
   *
   * \param[in] log_level Log level
   * \param[in] msg Log message
   */
  void logger(const log_level log_level, const std::string msg);

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
