/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_CPU_CORE_COUNT_HPP
#define _WASSAIL_CPU_CORE_COUNT_HPP

#include <memory>
#include <string>
#include <wassail/checks/check.hpp>
#include <wassail/checks/rules_engine.hpp>
#include <wassail/data/sysconf.hpp>
#include <wassail/data/sysctl.hpp>
#include <wassail/data/sysinfo.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

namespace wassail {
  namespace check {
    namespace cpu {
      /*! \brief Check building block class for the number of cpu cores */
      class core_count : public wassail::check::rules_engine {
      public:
        struct {
          uint16_t num_cores = 0; /*!< Reference number of cores */
        } config;                 /*!< Check building block configuration */

        /*! Construct an instance */
        core_count() : core_count(0){};

        /*! Construct an instance
         *  \param[in] num_cores Reference number of cores
         *
         * Template field 0 is the observed number of CPU cores.  In the case
         * of error, field 0 contains the error message. Template field 1 is
         * the expected or reference number of CPU cores.
         */
        core_count(uint16_t num_cores)
            : rules_engine(
                  "Checking number of CPU cores",
                  "Observed number of cores {0} not equal to expected value "
                  "{1}",
                  "Unable to check number of cores: '{0}'",
                  "Observed number of cores {0} equal to expected value {1}"),
              config{num_cores} {};

        /*! \brief Construct an instance
         *  \param[in] num_cores Reference number of cores
         *  \param[in] brief result brief format template
         *  \param[in] detail_yes result detail format template for the case
         *             when issue::YES
         *  \param[in] detail_maybe result detail format template for the
         *             case when issue::MAYBE
         *  \param[in] detail_no result detail format template for the case
         *             when issue::NO
         */
        core_count(uint16_t num_cores, std::string brief,
                   std::string detail_yes, std::string detail_maybe,
                   std::string detail_no)
            : rules_engine(brief, detail_yes, detail_maybe, detail_no),
              config{num_cores} {};

        /*! Check number of cpu cores
         * \param[in] data JSON object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(const json &data);

        /*! Check number of cpu cores
         * \param[in] data sysconf object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::sysconf &data);

        /*! Check number of cpu cores
         * \param[in] data sysctl object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::sysctl &data);

        /*! Unique name for this building block */
        std::string name() const { return "cpu/core_count"; };
      };
    } // namespace cpu
  }   // namespace check
} // namespace wassail

#endif
