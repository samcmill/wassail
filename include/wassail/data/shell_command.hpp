/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DATA_SHELL_COMMAND_HPP
#define _WASSAIL_DATA_SHELL_COMMAND_HPP

#include <memory>
#include <string>
#include <wassail/data/data.hpp>

namespace wassail {
  namespace data {
    /*! \brief Data source building block class for running
     *  UNIX shell commands
     */
    class shell_command : public wassail::data::common {
    public:
      /*! constructor */
      shell_command();
      /*! destructor */
      ~shell_command();
      /*! move constructor */
      shell_command(shell_command &&);
      /*! move constructor */
      shell_command &operator=(shell_command &&);

      /*! Indicate whether the building block is enabled or not.  If not,
       *  evaluating the building block will throw an exception.
       *  \return true if the required capabilities are available, false
       * otherwise
       */
      bool enabled() const;

      /*! Construct an instance.  Note that the shell command is not
       *  evaluated during construction.
       *  \see evaluate().
       *  \param[in] command Shell command line
       */
      shell_command(std::string command);

      /*! Construct an instance.  Note that the shell command is not
       *  evaluated during construction.
       *  \see evaluate()
       *  \param[in] command Shell command line
       *  \param[in] timeout Number of seconds to wait before timing out
       */
      shell_command(std::string command, uint8_t timeout);

      std::string command; /*!< Shell command line */

      bool exclusive =
          false; /*!< Block until the command can be executed exclusively */

      uint8_t timeout = 60; /*!< Number of seconds to wait before
                                 timing out */

      /*! If shell command has already been executed, do nothing.
       *  Otherwise, execute the shell command.
       * \param[in] force Force reevaluation (i.e., ignore any cached data)
       * \throws std::runtime_error() if required capabilities are not
       * available
       */
      void evaluate(bool force = false);

      /*! JSON type conversion
       * \param[in] j JSON object
       * \param[in,out] d
       */
      friend void from_json(const json &j, shell_command &d);

      /*! JSON type conversion
       * \param[in,out] j JSON object
       * \param[in] d
       *
       * \par JSON schema
       * \include shell_command.json
       */
      friend void to_json(json &j, const shell_command &d);

    private:
      /*! Unique name for this building block */
      std::string name() const { return "shell_command"; };

      /*! Interface version for this building block */
      uint16_t version() const { return 100; };

      class impl; /*! forward declaration of the implementation class */
      std::unique_ptr<impl> pimpl; /*! private implmentation */
    };
  } // namespace data
} // namespace wassail

#endif
