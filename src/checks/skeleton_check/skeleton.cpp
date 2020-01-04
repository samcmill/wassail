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
      try {
        if (j.at("name").get<std::string>() == "skeleton") {
          auto r = wassail::make_result(j);
          /* check something */
          return r;
        }
        else {
          throw std::runtime_error("Unrecognized JSON object");
        }
      }
      catch (std::exception &e) {
        throw std::runtime_error(std::string("Unable to perform check: ") +
                                 std::string(e.what()));
      }
    }

    std::shared_ptr<wassail::result>
    skeleton::check(wassail::data::skeleton &d) {
      d.evaluate();
      return check(static_cast<json>(d));
    }
  } // namespace check
} // namespace wassail
