/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_CHECK_HPP
#define _WASSAIL_CHECK_HPP

#include <wassail/common.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

using json = nlohmann::json;

namespace wassail {
  namespace check {
    /*! \brief Parent class for all check building blocks */
    class common {
    public:
      virtual ~common() = default;

      /*! Constructor */
      common(){};

      /*! Constructor
       *  \param[in] brief result brief format template
       *  \param[in] detail_yes result detail format template for the case
       *             when issue_t::YES
       *  \param[in] detail_maybe result detail format template for the case
       *             when issue_t::MAYBE
       *  \param[in] detail_no result detail format template for the case
       *             when issue_t::NO
       */
      common(std::string brief, std::string detail_yes,
             std::string detail_maybe, std::string detail_no)
          : fmt_str{brief, detail_yes, detail_maybe, detail_no} {};

      struct {
        std::string brief =
            "Brief check description"; /*!< result brief format template */
        std::string detail_yes = "Issue detected"; /*!< result detail format
                                      template for the case when issue_t::YES */
        std::string detail_maybe =
            "Potential issue"; /*!< result detail format template for
                   the case when issue_t::MAYBE */
        std::string detail_no = "No issue"; /*!< result detail format template
                                    for the case when issue_t::NO */
      } fmt_str; /*!< result format templates (fmtlib) */

      /*! \brief Virtual check function */
      virtual std::shared_ptr<wassail::result> check(const json &j) = 0;

      /*! \brief Building block unique name */
      virtual std::string name() const = 0;
    };
  } // namespace check
} // namespace wassail

#endif
