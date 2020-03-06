/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_MISC_LOAD_AVERAGE_HPP
#define _WASSAIL_MISC_LOAD_AVERAGE_HPP

#include <memory>
#include <string>
#include <wassail/checks/rules_engine.hpp>
#include <wassail/data/getloadavg.hpp>
#include <wassail/data/sysctl.hpp>
#include <wassail/data/sysinfo.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

namespace wassail {
  namespace check {
    namespace misc {
      /*! \brief Check building block class for the system load average */
      class load_average : public wassail::check::rules_engine {
      public:
        enum minute_t {
          ONE = 1,     /*!< 1 minute load average */
          FIVE = 5,    /*!< 5 minute load average */
          FIFTEEN = 15 /*!< 15 minute load average */
        };

        struct {
          float load = 0; /*!< Reference threshold load average */
          minute_t minute = minute_t::ONE; /*!< Load average to check */
        } config; /*!< Check building block configuration */

        /*! Construct an instance
         *  \param[in] load Reference load average
         */
        load_average(float load) : load_average(load, minute_t::ONE){};

        /*! Construct an instance
         *  \param[in] load Reference load average
         *  \param[in] minute Load average value to check (1, 5, or 15 minute)
         *
         * Template field 0 is the time period of the load average (1, 5, or 15
         * minutes). In the case of error, field 0 contains the error message.
         * Template field 1 is the observed load average. Template field 2 is
         * the expected or reference load average threshold.
         */
        load_average(float load, minute_t minute)
            : rules_engine(
                  "Checking {0} minute load average",
                  "Observed load average {1:.2f} greater than reference "
                  "threshold {2:.2f}",
                  "Unable to check load average: '{0}'",
                  "Observed load average {1:.2f} less than or equal to "
                  "reference threshold {2:.2f}"),
              config{load, minute} {};

        /*! Construct an instance
         *  \param[in] load Reference load average
         *  \param[in] minute Load average value to check (1, 5, or 15 minute)
         *  \param[in] brief result brief format template
         *  \param[in] detail_yes result detail format template for the case
         *             when issue::YES
         *  \param[in] detail_maybe result detail format template for the
         *             case when issue::MAYBE
         *  \param[in] detail_no result detail format template for the case
         *             when issue::NO
         */
        load_average(float load, minute_t minute, std::string brief,
                     std::string detail_yes, std::string detail_maybe,
                     std::string detail_no)
            : rules_engine(brief, detail_yes, detail_maybe, detail_no),
              config{load, minute} {};

        /*! Check load average
         * \param[in] data JSON object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(const json &data);

        /*! Check load average
         * \param[in] data getloadavg object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::getloadavg &data);

        /*! Check load average
         * \param[in] data sysctl object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::sysctl &data);

        /*! Check load average
         * \param[in] data sysinfo object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::sysinfo &data);

      private:
        /*! Unique name for this building block */
        std::string name() const { return "misc/load_average"; };
      };
    } // namespace misc
  }   // namespace check
} // namespace wassail

#endif
