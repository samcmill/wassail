/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "internal.hpp"

namespace wassail {
  namespace internal {
    bool parallel() {
#if __cpp_lib_execution >= 201603L
      return true;
#elif HAVE_LIBDISPATCH
      return true;
#elif HAVE_TBB
      return true;
#else
      return false;
#endif
    }
  } // namespace internal
} // namespace wassail
