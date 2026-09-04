/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_CHECK_USER_EXISTS_HPP
#define _WASSAIL_CHECK_USER_EXISTS_HPP

#include <memory>
#include <string>
#include <wassail/checks/rules_engine.hpp>
#include <wassail/data/getpwent.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

namespace wassail {
  namespace check {
    namespace user {
      /*! \brief Check building block class for the existence of a user
       */
      class exists : public wassail::check::rules_engine {
      public:
        /*! \brief Configuration and thresholds */
        struct {
          /* configuration options */
          std::string username; /*!< user name */
        } config;               /*!< Check building block configuration */

        /*! Construct an instance
         *  \param[in] user name to check
         */
        exists(std::string username)
            : rules_engine("Checking user '{0}' exists",
                           "User '{0}' does not exist",
                           "Unable to check whether user '{0}' exists",
                           "User '{0}' exists"),
              config{username} {};

        /*! Construct an instance
         *  \param[in] user name to check
         *  \param[in] brief result brief format template
         *  \param[in] brief result brief format template
         *  \param[in] detail_yes result detail format template for the case
         *             when issue::YES
         *  \param[in] detail_maybe result detail format template for the
         *             case when issue::MAYBE
         *  \param[in] detail_no result detail format template for the case
         *             when issue::NO
         */
        exists(std::string username, std::string brief, std::string detail_yes,
               std::string detail_maybe, std::string detail_no)
            : rules_engine(brief, detail_yes, detail_maybe, detail_no),
              config{username} {}

        /*! Check data (JSON)
         * \param[in] data JSON object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(const json &data);

        /*! Check data (building block)
         * \param[in] data getpwent data object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::getpwent &data);

      private:
        /*! Unique name for this building block */
        std::string name() const { return "user/exists"; };
      };
    } // namespace user
  } // namespace check
} // namespace wassail

#endif
