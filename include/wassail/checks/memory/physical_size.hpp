/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_CHECK_MEMORY_PHYSICAL_SIZE_HPP
#define _WASSAIL_CHECK_MEMORY_PHYSICAL_SIZE_HPP

#include <memory>
#include <string>
#include <wassail/checks/rules_engine.hpp>
#include <wassail/data/sysconf.hpp>
#include <wassail/data/sysctl.hpp>
#include <wassail/data/sysinfo.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

namespace wassail {
  namespace check {
    namespace memory {
      /*! \brief Check building block class for the amount of physical memory */
      class physical_size : public wassail::check::rules_engine {
      public:
        struct {
          uint64_t mem_size = 0;  /*!< Physical memory size in bytes */
          uint64_t tolerance = 0; /*!< Tolerance in bytes */
        } config;                 /*!< Check building block configuration */

        /*! Construct an instance */
        physical_size() : physical_size(0, 0){};

        /*! Construct an instance
         *  \param[in] mem_size Reference physical memory size
         */
        physical_size(uint64_t mem_size) : physical_size(mem_size, 0){};

        /*! Construct an instance
         *  \param[in] mem_size Reference physical memory size in bytes
         *  \param[in] tolerance Tolerance in bytes
         *
         * Template field 0 is the observed amount of memory. In the case of
         * error, field 0 contains the error message. Template field 1 is
         * expected amount of memory. Template field 2 is the tolerance.
         * Template field 3 is units of memory.
         */
        physical_size(uint64_t mem_size, uint64_t tolerance)
            : rules_engine(
                  "Checking physical memory size",
                  "Observed memory size of {0} {3:s} not within {1} +/- {2} "
                  "{3:s}",
                  "Unable to check memory size: '{0}'",
                  "Observed memory size of {0} {3:s} within {1} +/- {2} {3:s}"),
              config{mem_size, tolerance} {};

        /*! Construct an instance
         *  \param[in] mem_size Reference physical memory size in bytes
         *  \param[in] tolerance Tolerance in bytes
         *  \param[in] brief result brief format template
         *  \param[in] detail_yes result detail format template for the case
         *             when issue::YES
         *  \param[in] detail_maybe result detail format template for the
         *             case when issue::MAYBE
         *  \param[in] detail_no result detail format template for the case
         *             when issue::NO
         */
        physical_size(uint64_t mem_size, uint64_t tolerance, std::string brief,
                      std::string detail_yes, std::string detail_maybe,
                      std::string detail_no)
            : rules_engine(brief, detail_yes, detail_maybe, detail_no),
              config{mem_size, tolerance} {};

        /*! Check physical memory size
         * \param[in] data JSON object
         * \throws std::runtime_error() if input is invalid or unrecognized
         * \return result object
         */
        std::shared_ptr<wassail::result> check(const json &data);

        /*! Check memory size
         * \param[in] data sysconf object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::sysconf &data);

        /*! Check memory size
         * \param[in] data sysctl object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::sysctl &data);

        /*! Check memory size
         * \param[in] data sysinfo object
         * \return result object
         */
        std::shared_ptr<wassail::result> check(wassail::data::sysinfo &data);

      private:
        /*! Unique name for this building block */
        std::string name() const { return "memory/physical_size"; };
      };
    } // namespace memory
  }   // namespace check
} // namespace wassail

#endif
