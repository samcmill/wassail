/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_CHECK_MISC_SHELL_OUTPUT_HPP
#define _WASSAIL_CHECK_MISC_SHELL_OUTPUT_HPP

#include <memory>
#include <string>
#include <wassail/checks/rules_engine.hpp>
#include <wassail/data/remote_shell_command.hpp>
#include <wassail/data/shell_command.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

namespace wassail {
  namespace check {
    namespace misc {
      /*! \brief Check building block class for shell output */
      class shell_output : public wassail::check::rules_engine {
      public:
        /*! \brief Configuration and thresholds */
        struct {
          bool regex =
              false; /*!< The reference value is a regular expression */
          std::string output; /*!< Reference shell output */
        } config;             /*!< Check building block configuration */

        /*! Construct an instance
         *  \param[in] output Reference shell output
         *  \param[in] regex Consider the reference value to be a regular
         *                   expression
         *
         * Template field 0 is the observed shell output.  In the case of
         * error, field 0 contains the error message. Template field 1 is the
         * expected or reference shell output.
         */
        shell_output(std::string output, bool regex = false)
            : rules_engine(
                  "Checking shell output",
                  "Observed output '{0}' does not match expected output "
                  "'{1}'",
                  "Unable to check output: '{0}'",
                  "Observed output '{0}' matches expected output '{1}'"),
              config{regex, output} {}

        /*! Construct an instance
         *  \param[in] output Reference shell output
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
        shell_output(std::string output, bool regex, std::string brief,
                     std::string detail_yes, std::string detail_maybe,
                     std::string detail_no)
            : rules_engine(brief, detail_yes, detail_maybe, detail_no),
              config{regex, output} {}

        /*! Check data (JSON)
         * \param[in] data JSON object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(const json &data);

        /*! Check data (building block)
         * \param[in] data remote_shell_command data object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result>
        check(wassail::data::remote_shell_command &data);

        /*! Check data (building block)
         * \param[in] data shell_command data object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result>
        check(wassail::data::shell_command &data);

        /*! Unique name for this building block */
        std::string name() const { return "misc/shell_output"; };
      };
    } // namespace misc
  }   // namespace check
} // namespace wassail

#endif
