/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include <wassail/common.hpp>
#include <wassail/fmt/format.h>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace wassail {
  void initialize(wassail::log_level log_level) {
    auto logger = spdlog::stderr_color_mt(LOGGER);

    switch (log_level) {
    case wassail::log_level::trace:
      logger->set_level(spdlog::level::trace);
      break;
    case wassail::log_level::debug:
      logger->set_level(spdlog::level::debug);
      break;
    case wassail::log_level::info:
      logger->set_level(spdlog::level::info);
      break;
    case wassail::log_level::warn:
      logger->set_level(spdlog::level::warn);
      break;
    case wassail::log_level::err:
      logger->set_level(spdlog::level::err);
      break;
    case wassail::log_level::critical:
      logger->set_level(spdlog::level::critical);
      break;
    case wassail::log_level::off:
      logger->set_level(spdlog::level::off);
      break;
    }
  }
} // namespace wassail
