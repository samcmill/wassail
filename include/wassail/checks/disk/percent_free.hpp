/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DISK_PERCENT_FREE_HPP
#define _WASSAIL_DISK_PERCENT_FREE_HPP

#include <memory>
#include <string>
#include <wassail/checks/check.hpp>
#include <wassail/data/getfsstat.hpp>
#include <wassail/data/getmntent.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

namespace wassail {
  namespace check {
    namespace disk {
      /*! \brief Check building block class for the percent of disk space free
       */
      class percent_free : public wassail::check::common {
      public:
        struct {
          std::string filesystem; /*!< Mount point to check */
          float percent = 0;      /*!< Reference percent free */
        } config;                 /*!< Check building block configuration */

        /*! Construct an instance
         *  \param[in] fs Mount point to check
         */
        percent_free(std::string fs) : percent_free(fs, 0.0){};

        /*! Construct an instance
         *  \param[in] fs Mount point to check
         *  \param[in] percent Reference percent free
         *
         * Template field 0 is the name of the filesystem. Template field 1 is
         * the observed free disk percentage. Template field 2 is the expected
         * or reference free disk percentage.
         */
        percent_free(std::string fs, float percent)
            : common(
                  "Checking percent free disk space of filesystem '{0}'",
                  "Observed percent free disk space {1}% is less than "
                  "reference threshold value {2}%",
                  "Unable to check percent free disk space of filesystem '{0}'",
                  "Observed percent free disk space {1}% is greater than or "
                  "equal to the reference threshold value {2}%"),
              config{fs, percent} {};

        /*! \brief Construct an instance
         *  \param[in] fs Mount point to check
         *  \param[in] percent Reference percent free
         *  \param[in] brief result brief format template
         *  \param[in] detail_yes result detail format template for the case
         *             when issue::YES
         *  \param[in] detail_maybe result detail format template for the
         *             case when issue::MAYBE
         *  \param[in] detail_no result detail format template for the case
         *             when issue::NO
         */
        percent_free(std::string fs, float percent, std::string brief,
                     std::string detail_yes, std::string detail_maybe,
                     std::string detail_no)
            : common(brief, detail_yes, detail_maybe, detail_no),
              config{fs, percent} {};

        /*! Check percent free disk space
         * \param[in] data JSON object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(const json &data);

        /*! Check percent free disk space
         * \param[in] data getfsstat object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::getfsstat &data);

        /*! Check percent free disk space
         * \param[in] data getmntent object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::getmntent &data);

      private:
        /*! Unique name for this building block */
        std::string name() const { return "disk/percent_free"; };
      };
    } // namespace disk
  }   // namespace check
} // namespace wassail

#endif
