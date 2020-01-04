/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DATA_UNAME_HPP
#define _WASSAIL_DATA_UNAME_HPP

#include <memory>
#include <string>
#include <wassail/data/data.hpp>

namespace wassail {
  namespace data {
    /*! \brief Data source building block class for getting system
     *  identification from %uname()
     */
    class uname final : public wassail::data::common {
    public:
      /*! constructor */
      uname();
      /*! destructor */
      ~uname();
      /*! move constructor */
      uname(uname &&);
      /*! move constructor */
      uname &operator=(uname &&);

      /*! Indicate whether the building block is enabled or not.  If not,
       *  evaluating the building block will throw an exception.
       *  \return true if %uname() is available, false otherwise
       */
      bool enabled() const;

      /*! If the data has already been read, do nothing.
       *  Otherwise get the data.
       * \param[in] force Force reevaluation (i.e., ignore any cached data)
       * \throws std::runtime_error() if %uname() is not available
       */
      void evaluate(bool force = false);

      /*! JSON type conversion
       * \param[in] j JSON object
       * \param[in,out] d
       */
      friend void from_json(const json &j, uname &d);

      /*! JSON type conversion
       * \param[in,out] j JSON object
       * \param[in] d
       *
       * \par JSON schema
       * \include uname.json
       */
      friend void to_json(json &j, const uname &d);

    private:
      /*! Unique name for this building block */
      std::string name() const { return "uname"; };

      /*! Interface version for this building block */
      uint16_t version() const { return 100; };

      class impl; /*! forward declaration of the implementation class */
      std::unique_ptr<impl> pimpl; /*! private implementation */
    };
  } // namespace data
} // namespace wassail

#endif
