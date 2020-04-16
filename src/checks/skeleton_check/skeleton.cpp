/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "internal.hpp"

#include <exception>
#include <string>
#include <wassail/checks/skeleton/skeleton.hpp>

namespace wassail {
  namespace check {
    std::shared_ptr<wassail::result> skeleton::check(const json &j) {
      if (j.value("name", "") == "skeleton") {
        auto r = wassail::make_result(j);
        /* check something */
        return r;
      }
      else {
        throw std::runtime_error("Unrecognized JSON object");
      }
    }

    std::shared_ptr<wassail::result>
    skeleton::check(wassail::data::skeleton &d) {
      d.evaluate();
      return check(static_cast<json>(d));
    }
  } // namespace check
} // namespace wassail
