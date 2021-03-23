/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DATA_PS_HPP
#define _WASSAIL_DATA_PS_HPP

#include <string>
#include <wassail/data/shell_command.hpp>

namespace wassail {
  namespace data {
    /*! \brief Data source building block class for listing process
     *  information from ps(1)
     */
    class ps final : public wassail::data::shell_command {
    public:
      /* Equivalent to "ps aux" */
      ps()
          : shell_command(
                "ps -eo user,pid,pcpu,pmem,vsz,rss,tt,state,start,time,command",
                1) {
        exclusive = true;
      };

      /*! Unique name for this building block */
      std::string name() const { return "ps"; };

      /*! JSON type conversion
       * \param[in] j JSON object
       * \param[in,out] d
       */
      friend void from_json(const json &j, ps &d);

      /*! JSON type conversion
       *  \param[in] j JSON object
       */
      void from_json(const json &j) { *this = j; };

      /*! JSON type conversion
       * \param[in,out] j JSON object
       * \param[in] d
       *
       * \par JSON schema
       * \include ps.json
       */
      friend void to_json(json &j, const ps &d);

      /*! JSON type conversion */
      json to_json() { return static_cast<json>(*this); };

    private:
      /*! Interface version for this building block */
      uint16_t version() const { return 100; };
    };
  } // namespace data
} // namespace wassail

#endif
