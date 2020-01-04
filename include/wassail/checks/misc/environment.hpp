/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_CHECK_ENVIRONMENT_H
#define _WASSAIL_CHECK_ENVIRONMENT_H

#include <memory>
#include <string>
#include <wassail/checks/check.hpp>
#include <wassail/data/environment.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

namespace wassail {
  namespace check {
    namespace misc {
      /*! \brief Check building block class for environment variables */
      class environment : public wassail::check::common {
      public:
        /*! \brief Configuration and thresholds */
        struct {
          bool regex =
              false;         /*!< The reference value is a regular expression */
          std::string value; /*!< Environment variable reference value */
          std::string variable; /*!< Environment variable name */
        } config;               /*!< Check building block configuration */

        /*! Construct an instance
         *  \param[in] variable Environment variable name
         *  \param[in] value Reference environment variable value
         *  \param[in] regex Consider the reference value to be a regular
         *                   expression
         */
        environment(std::string variable, std::string value, bool regex = false)
            : common("Checking environment variable '{0}'",
                     "Value '{0}' does not match '{1}'",
                     "Unable to check value '{0}'",
                     "Value '{0}' matches '{1}'"),
              config{regex, value, variable} {}

        /*! Construct an instance
         *  \param[in] variable Environment variable name
         *  \param[in] value Reference environment variable value
         *  \param[in] regex Consider the reference value to be a regular
         *                   expression
         *  \param[in] brief result brief format template
         *  \param[in] detail_yes result detail format template for the case
         *             when issue::YES
         *  \param[in] detail_maybe result detail format template for the
         *             case when issue::MAYBE
         *  \param[in] detail_no result detail format template for the case
         *             when issue::NO
         */
        environment(std::string variable, std::string value, bool regex,
                    std::string brief, std::string detail_yes,
                    std::string detail_maybe, std::string detail_no)
            : common(brief, detail_yes, detail_maybe, detail_no),
              config{regex, value, variable} {}

        /*! Check data (JSON)
         * \param[in] data JSON object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(const json &data);

        /*! Check data (building block)
         * \param[in] data environment data object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result>
        check(wassail::data::environment &data);

      private:
        /*! Unique name for this building block */
        std::string name() const { return "environment"; };
      };
    } // namespace misc
  }   // namespace check
} // namespace wassail

#endif
