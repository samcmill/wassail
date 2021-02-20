/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DATA_MPIRUN_HPP
#define _WASSAIL_DATA_MPIRUN_HPP

#include <string>
#include <vector>
#include <wassail/data/shell_command.hpp>

namespace wassail {
  namespace data {
    /*! \brief Data source building block class for MPI programs */
    class mpirun : public wassail::data::shell_command {
    public:
      /*! \brief MPI implementation
       */
      enum class mpi_impl_t {
        MPICH = 1,                      /*!< MPICH */
        OPENMPI = 0                     /*!< OpenMPI */
      } mpi_impl = mpi_impl_t::OPENMPI; /*!< MPI implementation */

      /*! For OpenMPI only, if running as root, add the --allow-run-as-root
       * flag.
       */
      bool allow_run_as_root = true;

      std::string hostfile; /*!< Path to file containing list of hosts */

      std::vector<std::string> hostlist; /*!< List of hosts */

      std::string mpirun_args; /*!< extra mpirun arguments */

      uint32_t num_procs = 1; /*!< Number of MPI processes to start */

      /*! Number of MPI processes per node. 0 means do not specify the
       *  number on the command line.
       */
      uint32_t per_node = 0;

      std::string program; /*!< MPI program to launch */

      std::string program_args; /*!< Arguments to pass to the MPI program */

      /*! Construct an instance */
      mpirun() : shell_command(""){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] program MPI program to launch
       * \param[in] mpi_impl MPI implementation
       */
      mpirun(uint32_t num_procs, std::string program)
          : mpirun(num_procs, 0, "", "", program, "", 60,
                   mpi_impl_t::OPENMPI){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] program MPI program to launch
       * \param[in] mpi_impl MPI implementation
       */
      mpirun(uint32_t num_procs, std::string program, mpi_impl_t mpi_impl)
          : mpirun(num_procs, 0, "", "", program, "", 60, mpi_impl){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] hostfile Path to file containing list of hosts
       * \param[in] program MPI program to launch
       * \param[in] mpi_impl MPI implementation
       */
      mpirun(uint32_t num_procs, std::string hostfile, std::string program,
             mpi_impl_t mpi_impl = mpi_impl_t::OPENMPI)
          : mpirun(num_procs, 0, hostfile, "", program, "", 60, mpi_impl){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] hostlist List of hosts
       * \param[in] program MPI program to launch
       * \param[in] mpi_impl MPI implementation
       */
      mpirun(uint32_t num_procs, std::vector<std::string> hostlist,
             std::string program, mpi_impl_t mpi_impl = mpi_impl_t::OPENMPI)
          : mpirun(num_procs, 0, hostlist, "", program, "", 60, mpi_impl){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] per_node Number of MPI processes to start per node
       * \param[in] hostfile Path to file containing list of hosts
       * \param[in] mpirun_args Addition mpirun arguments
       * \param[in] program MPI program to launch
       * \param[in] program_args MPI program arguments
       * \param[in] timeout Number of seconds to wait before timing out
       * \param[in] mpi_impl MPI implementation
       */
      mpirun(uint32_t num_procs, uint32_t per_node, std::string hostfile,
             std::string mpirun_args, std::string program,
             std::string program_args, uint8_t timeout,
             mpi_impl_t mpi_impl = mpi_impl_t::OPENMPI);

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] per_node Number of MPI processes to start per node
       * \param[in] hostlist List of hosts
       * \param[in] mpirun_args Addition mpirun arguments
       * \param[in] program MPI program to launch
       * \param[in] program_args MPI program arguments
       * \param[in] timeout Number of seconds to wait before timing out
       * \param[in] mpi_impl MPI implementation
       */
      mpirun(uint32_t num_procs, uint32_t per_node,
             std::vector<std::string> hostlist, std::string mpirun_args,
             std::string program, std::string program_args, uint8_t timeout,
             mpi_impl_t mpi_impl = mpi_impl_t::OPENMPI);

      /*! Unique name for this building block */
      std::string name() const { return "mpirun"; };

      /*! JSON type conversion
       * \param[in] j JSON object
       * \param[in,out] d
       */
      friend void from_json(const json &j, mpirun &d);

      /*! JSON type conversion
       * \param[in,out] j JSON object
       * \param[in] d
       *
       * \par JSON schema
       * \include mpirun.json
       */
      friend void to_json(json &j, const mpirun &d);

    protected:
      /*! Helper to build the command line */
      void set_cmdline();

    private:
      /*! Number of seconds to wait before timing out */
      uint8_t mpirun_timeout = 60;

      /*! Interface version for this building block */
      uint16_t version() const { return 100; };
    };
  } // namespace data
} // namespace wassail

#endif
