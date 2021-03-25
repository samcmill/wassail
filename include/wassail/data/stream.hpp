/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DATA_STREAM_HPP
#define _WASSAIL_DATA_STREAM_HPP

#include <string>
#include <wassail/data/shell_command.hpp>

#ifndef WASSAIL_LIBEXECDIR
#define WASSAIL_LIBEXECDIR "/usr/libexec/wassail"
#endif

namespace wassail {
  namespace data {
    /*! \brief Data source building block class for the STREAM memory
     *  benchmark
     */
    class stream final : public wassail::data::shell_command {
    public:
      /* STREAM memory benchmark */
      stream() : shell_command(std::string(WASSAIL_LIBEXECDIR) + "/stream", 5) {
        exclusive = true;
      };

      /*! Unique name for this building block */
      std::string name() const { return "stream"; };

      /*! JSON type conversion
       * \param[in] j JSON object
       * \param[in,out] d
       */
      friend void from_json(const json &j, stream &d);

      /*! JSON type conversion
       *  \param[in] j JSON object
       */
      void from_json(const json &j) { *this = j; };

      /*! JSON type conversion
       * \param[in,out] j JSON object
       * \param[in] d
       *
       * \par JSON schema
       * \include stream.json
       */
      friend void to_json(json &j, const stream &d);

      /*! JSON type conversion */
      json to_json() { return static_cast<json>(*this); };

    private:
      /*! Interface version for this building block */
      uint16_t version() const { return 100; };
    };
  } // namespace data
} // namespace wassail

#endif
