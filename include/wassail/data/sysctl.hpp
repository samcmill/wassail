/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DATA_SYSCTL_HPP
#define _WASSAIL_DATA_SYSCTL_HPP

#include <memory>
#include <string>
#include <wassail/data/data.hpp>

namespace wassail {
  namespace data {
    /*! \brief Data source building block class for getting
        system information from %sysctlbyname() */
    class sysctl final : public wassail::data::common {
    public:
      /*! constructor */
      sysctl();
      /*! destructor */
      ~sysctl();
      /*! move constructor */
      sysctl(sysctl &&);
      /*! move constructor */
      sysctl &operator=(sysctl &&);

      /*! Indicate whether the building block is enabled or not.  If not,
       *  evaluating the building block will throw an exception.
       *  \return True if %sysctlbyname() is available, false otherwise
       */
      bool enabled() const;

      /*! If the system information has already been read, do nothing.
       *  Otherwise get the system information.
       * \param[in] force Force reevaluation (i.e., ignore any cached data)
       * \throws std::runtime_error() if %sysctlbyname() is not available
       */
      void evaluate(bool force = false);

      /*! Unique name for this building block */
      std::string name() const { return "sysctl"; };

      /*! JSON type conversion
       * \param[in] j JSON object
       * \param[in,out] d
       */
      friend void from_json(const json &j, sysctl &d);

      /*! JSON type conversion
       *  \param[in] j JSON object
       */
      void from_json(const json &j) { *this = j; };

      /*! JSON type conversion
       * \param[in,out] j JSON object
       * \param[in] d
       *
       * \par JSON schema
       * \include sysctl.json
       */
      friend void to_json(json &j, const sysctl &d);

      /*! JSON type conversion */
      json to_json() { return static_cast<json>(*this); };

    private:
      /*! Interface version for this building block */
      uint16_t version() const { return 100; };

      class impl; /*! forward declaration of the implementation class */
      std::unique_ptr<impl> pimpl; /*! private implementation */
    };
  } // namespace data
} // namespace wassail

#endif
