/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DISK_AMOUNT_FREE_HPP
#define _WASSAIL_DISK_AMOUNT_FREE_HPP

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
      /*! \brief Check building block class for the amount of disk space free
       */
      class amount_free : public wassail::check::common {
      public:
        struct {
          std::string filesystem; /*!< Mount point to check */
          uint64_t amount = 0;    /*!< Reference amount free, in bytes */
        } config;                 /*!< Check building block configuration */

        /*! Construct an instance
         *  \param[in] fs Mount point to check
         */
        amount_free(std::string fs) : amount_free(fs, 0){};

        /*! Construct an instance
         *  \param[in] fs Mount point to check
         *  \param[in] amount Reference amount free
         *
         * Template field 0 is the name of the filesystem. Template field 1 is
         * the observed amount of free disk space. Template field 2 is the
         * expected or reference amount of free disk space. Template field 3 is
         * the units of free disk space.
         */
        amount_free(std::string fs, uint64_t amount)
            : common("Checking amount of free disk space on filesystem '{0}'",
                     "Observed amount of free disk space {1} {3} is less than "
                     "reference threshold value {2} {3}",
                     "Unable to check amount of free disk space on filesystem "
                     "'{0}'",
                     "Observed amount of free disk space {1} {3} is greater "
                     "than or "
                     "equal to the reference threshold value {2} {3}"),
              config{fs, amount} {};

        /*! \brief Construct an instance
         *  \param[in] fs Mount point to check
         *  \param[in] amount Reference amount free
         *  \param[in] brief result brief format template
         *  \param[in] detail_yes result detail format template for the case
         *             when issue::YES
         *  \param[in] detail_maybe result detail format template for the
         *             case when issue::MAYBE
         *  \param[in] detail_no result detail format template for the case
         *             when issue::NO
         */
        amount_free(std::string fs, uint64_t amount, std::string brief,
                    std::string detail_yes, std::string detail_maybe,
                    std::string detail_no)
            : common(brief, detail_yes, detail_maybe, detail_no),
              config{fs, amount} {};

        /*! Check amount of free disk space
         * \param[in] data JSON object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(const json &data);

        /*! Check amount of disk space
         * \param[in] data getfsstat object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::getfsstat &data);

        /*! Check amount of free disk space
         * \param[in] data getmntent object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::getmntent &data);

        /*! Unique name for this building block */
        std::string name() const { return "disk/amount_free"; };
      };
    } // namespace disk
  }   // namespace check
} // namespace wassail

#endif
