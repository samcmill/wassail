/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_DATA_OSU_MICRO_BENCHMARKS_HPP
#define _WASSAIL_DATA_OSU_MICRO_BENCHMARKS_HPP

#include <string>
#include <wassail/data/mpirun.hpp>

#ifndef LIBEXECDIR
#define LIBEXECDIR "/usr/libexec/wassail"
#endif

namespace wassail {
  namespace data {
    /*! \brief Data source building block class for the OSU Micro-Benchmarks
     */
    class osu_micro_benchmarks final : public wassail::data::mpirun {
    public:
      /*! OSU benchmark */
      enum class osu_benchmark_t {
        ALLREDUCE = 0,                         /*!< osu_allreduce */
        ALLTOALL,                              /*!< osu_alltoall */
        BW,                                    /*!< osu_bw */
        HELLO,                                 /*!< osu_hello */
        INIT,                                  /*!< osu_init */
        LATENCY,                               /*!< osu_latency */
        REDUCE                                 /*!< osu_reduce */
      } osu_benchmark = osu_benchmark_t::INIT; /*!< Benchmark choice */

      /*! Construct an instance
       */
      osu_micro_benchmarks() : osu_micro_benchmarks(2, osu_benchmark_t::INIT){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       */
      osu_micro_benchmarks(uint32_t num_procs)
          : osu_micro_benchmarks(num_procs, 0, "", "", osu_benchmark_t::INIT,
                                 60, mpi_impl_t::OPENMPI){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] osu_benchmark OSU micro-benchmark to launch
       * \param[in] mpi_impl MPI implementation
       */
      osu_micro_benchmarks(uint32_t num_procs, osu_benchmark_t osu_benchmark,
                           mpi_impl_t mpi_impl = mpi_impl_t::OPENMPI)
          : osu_micro_benchmarks(num_procs, 0, "", "", osu_benchmark, 60,
                                 mpi_impl){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] hostfile Path to file containing list of hosts
       * \param[in] osu_benchmark OSU micro-benchmark to launch
       * \param[in] mpi_impl MPI implementation
       */
      osu_micro_benchmarks(uint32_t num_procs, std::string hostfile,
                           osu_benchmark_t osu_benchmark,
                           mpi_impl_t mpi_impl = mpi_impl_t::OPENMPI)
          : osu_micro_benchmarks(num_procs, 0, hostfile, "", osu_benchmark, 60,
                                 mpi_impl){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] hostlist List of hosts
       * \param[in] osu_benchmark OSU micro-benchmark to launch
       * \param[in] mpi_impl MPI implementation
       */
      osu_micro_benchmarks(uint32_t num_procs,
                           std::vector<std::string> hostlist,
                           osu_benchmark_t osu_benchmark,
                           mpi_impl_t mpi_impl = mpi_impl_t::OPENMPI)
          : osu_micro_benchmarks(num_procs, 0, hostlist, "", osu_benchmark, 60,
                                 mpi_impl){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] per_node Number of MPI processes to start per node
       * \param[in] hostfile Path to file containing list of hosts
       * \param[in] mpirun_args Addition mpirun arguments
       * \param[in] osu_benchmark OSU micro-benchmark to launch
       * \param[in] timeout Number of seconds to wait before timing out
       * \param[in] mpi_impl MPI implementation
       */
      osu_micro_benchmarks(uint32_t num_procs, uint32_t per_node,
                           std::string hostfile, std::string mpirun_args,
                           osu_benchmark_t osu_benchmark, uint8_t timeout,
                           mpi_impl_t mpi_impl = mpi_impl_t::OPENMPI)
          : mpirun(num_procs, per_node, hostfile, mpirun_args,
                   osu_program(osu_benchmark), "", timeout, mpi_impl),
            osu_benchmark(osu_benchmark){};

      /*! Construct an instance.
       * \param[in] num_procs Number of MPI processes to start
       * \param[in] per_node Number of MPI processes to start per node
       * \param[in] hostlist List of hosts
       * \param[in] mpirun_args Addition mpirun arguments
       * \param[in] osu_benchmark OSU micro-benchmark to launch
       * \param[in] timeout Number of seconds to wait before timing out
       * \param[in] mpi_impl MPI implementation
       */
      osu_micro_benchmarks(uint32_t num_procs, uint32_t per_node,
                           std::vector<std::string> hostlist,
                           std::string mpirun_args,
                           osu_benchmark_t osu_benchmark, uint8_t timeout,
                           mpi_impl_t mpi_impl = mpi_impl_t::OPENMPI)
          : mpirun(num_procs, per_node, hostlist, mpirun_args,
                   osu_program(osu_benchmark), "", timeout, mpi_impl),
            osu_benchmark(osu_benchmark){};

      /*! Unique name for this building block */
      std::string name() const { return "osu_micro_benchmarks"; };

      /*! JSON type conversion
       * \param[in] j JSON object
       * \param[in,out] d
       */
      friend void from_json(const json &j, osu_micro_benchmarks &d);

      /*! JSON type conversion
       * \param[in,out] j JSON object
       * \param[in] d
       *
       * \par JSON schema
       * \include osu_micro_benchmarks.json
       */
      friend void to_json(json &j, const osu_micro_benchmarks &d);

    private:
      /*! Interface version for this building block */
      uint16_t version() const { return 100; };

      /*! Table lookup to map osu_benchmark_t to program string */
      std::string osu_program(osu_benchmark_t osu_benchmark,
                              std::string libexecdir = std::string(LIBEXECDIR));
    };
  } // namespace data
} // namespace wassail

#endif
