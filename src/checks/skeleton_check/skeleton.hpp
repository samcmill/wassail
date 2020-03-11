/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_CHECK_SKELETON_HPP
#define _WASSAIL_CHECK_SKELETON_HPP

#include <memory>
#include <string>
#include <wassail/checks/check.hpp>
#include <wassail/data/skeleton.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

namespace wassail {
  namespace check {
    namespace skeleton {
      /*! \brief Skeleton check building block class
       */
      class skeleton : public wassail::check::common {
      public:
        /*! \brief Configuration and thresholds */
        struct {
          /* configuration options */
        } config;

        /*! Construct an instance */
        skeleton()
            : common("Checking skeleton item", "Issue detected template",
                     "Issue unsure template", "Issue not detected template") {}

        /*! Construct an instance
         *  \param[in] brief result brief format template
         *  \param[in] brief result brief format template
         *  \param[in] detail_yes result detail format template for the case
         *             when issue::YES
         *  \param[in] detail_maybe result detail format template for the
         *             case when issue::MAYBE
         *  \param[in] detail_no result detail format template for the case
         *             when issue::NO
         */
        skeleton(std::string brief, std::string detail_yes,
                 std::string detail_maybe, std::string detail_no)
            : common(brief, detail_yes, detail_maybe, detail_no) {}

        /*! Check data (JSON)
         * \param[in] data JSON object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(const json &data);

        /*! Check data (building block)
         * \param[in] data skeleton data object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::skeleton &data);

      private:
        /*! Unique name for this building block */
        std::string name() const { return "skeleton"; };
      };
    } // namespace skeleton
  }   // namespace check
} // namespace wassail

#endif
