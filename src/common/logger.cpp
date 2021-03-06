/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include <cassert>
#include <memory>
#include <wassail/common.hpp>
#include <wassail/fmt/format.h>

#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace wassail {
  namespace internal {
    std::shared_ptr<spdlog::logger> logger() {
      auto logger = spdlog::get(LOGGER);

      if (not logger) {
        /* The standard logger was not setup, so create a null logger. */
        auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        logger = std::make_shared<spdlog::logger>("null_logger", null_sink);
      }
      assert(logger);

      return (logger);
    }
  } // namespace internal
} // namespace wassail
