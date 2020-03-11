/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_CHECK_FILE_PERMISSIONS_HPP
#define _WASSAIL_CHECK_FILE_PERMISSIONS_HPP

#include <memory>
#include <string>
#include <wassail/checks/rules_engine.hpp>
#include <wassail/data/stat.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

namespace wassail {
  namespace check {
    namespace file {
      /*! \brief Check building block class for the permissions on a file
       */
      class permissions : public wassail::check::rules_engine {
      public:
        /*! \brief Configuration and thresholds */
        struct {
          /* configuration options */
          uint16_t mode; /*!< protection mode of file */
        } config;        /*!< Check building block configuration */

        /*! Construct an instance
         *  \param[in] mode octal protection mode to check
         *
         * Template field 0 is the name of the file. In the case of error, field
         * 0 contains the error message. Template field 1 is the observed octal
         * permissions. Template field 2 is the expected or reference octal
         * permissions.
         */
        permissions(uint16_t mode)
            : rules_engine(
                  "Checking permissions on '{0}'",
                  "Observed permissions of {1} do not match expected {2}",
                  "Unable to check permissions: '{0}'",
                  "Observed permissions of {1} match expected {2}"),
              config{mode} {};

        /*! Construct an instance
         *  \param[in] mode octal protection mode to check
         *  \param[in] brief result brief format template
         *  \param[in] brief result brief format template
         *  \param[in] detail_yes result detail format template for the case
         *             when issue::YES
         *  \param[in] detail_maybe result detail format template for the
         *             case when issue::MAYBE
         *  \param[in] detail_no result detail format template for the case
         *             when issue::NO
         */
        permissions(uint16_t mode, std::string brief, std::string detail_yes,
                    std::string detail_maybe, std::string detail_no)
            : rules_engine(brief, detail_yes, detail_maybe, detail_no),
              config{mode} {};

        /*! Check data (JSON)
         * \param[in] data JSON object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(const json &data);

        /*! Check data (building block)
         * \param[in] data stat data object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::stat &data);

      private:
        /*! Unique name for this building block */
        std::string name() const { return "file/permissions"; };
      };
    } // namespace file
  }   // namespace check
} // namespace wassail

#endif
