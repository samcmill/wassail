/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DATA_GETFSSTAT_HPP
#define _WASSAIL_DATA_GETFSSTAT_HPP

#include <memory>
#include <string>
#include <wassail/data/data.hpp>

namespace wassail {
  namespace data {
    /*! \brief Data source building block class for getting
     *  information about mounted filesystems from %getfsstat()
     */
    class getfsstat final : public wassail::data::common {
    public:
      /*! constructor */
      getfsstat();
      /*! destructor */
      ~getfsstat();
      /*! move constructor */
      getfsstat(getfsstat &&);
      /*! move constructor */
      getfsstat &operator=(getfsstat &&);

      /*! Indicate whether the building block is enabled or not.  If not,
       *  evaluating the building block will throw an exception.
       *  \return true if %getfsstat() is available, false otherwise
       */
      bool enabled() const;

      /*! If the filesystem information has already been read, do nothing.
       *  Otherwise get the filesystem information.
       * \param[in] force Force reevaluation (i.e., ignore any cached data)
       * \throws std::runtime_error() if %getfsstat() is not available
       */
      void evaluate(bool force = false);

      /*! JSON type conversion
       * \param[in] j JSON object
       * \param[in,out] d
       */
      friend void from_json(const json &j, getfsstat &d);

      /*! JSON type conversion
       * \param[in,out] j JSON object
       * \param[in] d
       *
       * \par JSON schema
       * \include getfsstat.json
       */
      friend void to_json(json &j, const getfsstat &d);

    private:
      /*! Unique name for this building block */
      std::string name() const { return "getfsstat"; };

      /*! Interface version for this building block */
      uint16_t version() const { return 100; };

      class impl; /*! forward declaration of the implementation class */
      std::unique_ptr<impl> pimpl; /*! private implementation */
    };
  } // namespace data
} // namespace wassail

#endif
