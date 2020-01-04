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
#include <wassail/checks/check.hpp>
#include <wassail/checks/compare.hpp>
#include <wassail/data/getloadavg.hpp>
#include <wassail/data/sysctl.hpp>
#include <wassail/data/sysinfo.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

namespace wassail {
  namespace check {
    namespace misc {
      /*! \brief Check building block class for the system load average */
      class load_average_generic : public wassail::check::compare {
      public:
        struct {
          float load = 0; /*!< Reference load average */
        } config;         /*!< Check building block configuration */

        /*! Construct an instance */
        load_average_generic() : load_average_generic(0.0){};

        /*! Construct an instance
         *  \param[in] load Reference load average
         */
        load_average_generic(float load)
            : compare("Checking {0} minute load average",
                      "Observed load average {0:.2f} greater than reference "
                      "threshold {1:.2f}",
                      "Unable to check load average: '{0}'",
                      "Observed load average {0:.2f} less than or equal to "
                      "reference threshold {1:.2f}"),
              config{load} {};

        /*! Construct an instance
         *  \param[in] load Reference load average
         *  \param[in] brief result brief format template
         *  \param[in] detail_yes result detail format template for the case
         *             when issue::YES
         *  \param[in] detail_maybe result detail format template for the
         *             case when issue::MAYBE
         *  \param[in] detail_no result detail format template for the case
         *             when issue::NO
         */
        load_average_generic(float load, std::string brief,
                             std::string detail_yes, std::string detail_maybe,
                             std::string detail_no)
            : compare(brief, detail_yes, detail_maybe, detail_no), config{
                                                                       load} {};

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
        virtual uint16_t get_minute() = 0;

        /*! Unique name for this building block */
        std::string name() const { return "misc/load_average"; };
      };

      /*! \brief Check building block class for the system 1 minute load
       * average */
      class load_average_1min : public load_average_generic {
        using load_average_generic::load_average_generic;

      private:
        uint16_t get_minute() { return 1; }

        /*! Unique name for this building block */
        std::string name() const { return "misc/load_average_1min"; };
      };

      /*! \brief Check building block class for the system 5 minute load
       * average */
      class load_average_5min : public load_average_generic {
        using load_average_generic::load_average_generic;

      private:
        uint16_t get_minute() { return 5; }

        /*! Unique name for this building block */
        std::string name() const { return "misc/load_average_5min"; };
      };

      /*! \brief Check building block class for the system 15 minute load
       * average */
      class load_average_15min : public load_average_generic {
        using load_average_generic::load_average_generic;

      private:
        uint16_t get_minute() { return 15; }

        /*! Unique name for this building block */
        std::string name() const { return "misc/load_average_15min"; };
      };
    } // namespace misc
  }   // namespace check
} // namespace wassail

#endif
