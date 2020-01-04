/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include <string>
#include <wassail/common.hpp>

namespace wassail {
  std::string version() { return std::string(PACKAGE_VERSION); }

  uint16_t version_major() { return VERSION_MAJOR; }

  uint16_t version_minor() { return VERSION_MINOR; }

  uint16_t version_micro() { return VERSION_MICRO; }
} // namespace wassail
