/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DATA_NVML_HPP
#define _WASSAIL_DATA_NVML_HPP

#include <memory>
#include <string>
#include <wassail/data/data.hpp>

namespace wassail {
  namespace data {
    /*! \brief Data source building block for getting NVIDIA GPU information */
    class nvml final : public wassail::data::common {
    public:
      /*! constructor */
      nvml();
      /*! destructor */
      ~nvml();
      /*! move constructor */
      nvml(nvml &&);
      /*! move constructor */
      nvml &operator=(nvml &&);

      /*! Indicate whether the building block is enabled or not.  If not,
       *  evaluating the building block will throw an exception.
       *  \return true if required capabilities are available, false otherwise
       */
      bool enabled() const;

      /*! If the data has already been read, do nothing.
       *  Otherwise get the data.
       * \param[in] force Force reevaluation (i.e., ignore any cached data)
       * \throws std::runtime_error() if the data source is not available
       */
      void evaluate(bool force = false);

      /*! Unique name for this building block */
      std::string name() const { return "nvml"; };

      /*! JSON type conversion
       * \param[in] j JSON object
       * \param[in,out] d
       */
      friend void from_json(const json &j, nvml &d);

      /*! JSON type conversion
       *  \param[in] j JSON object
       */
      void from_json(const json &j) { *this = j; };

      /*! JSON type conversion
       * \param[in,out] j JSON object
       * \param[in] d
       *
       * \par JSON schema
       * \include nvml.json
       */
      friend void to_json(json &j, const nvml &d);

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
