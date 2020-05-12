/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "internal.hpp"

#include <cstdlib>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
#include <wassail/data/mpirun.hpp>

namespace wassail {
  namespace data {
    mpirun::mpirun(uint32_t num_procs, uint32_t per_node, std::string hostfile,
                   std::string mpirun_args, std::string program,
                   std::string program_args, uint8_t _timeout,
                   mpi_impl_t mpi_impl)
        : mpi_impl(mpi_impl), hostfile(hostfile), mpirun_args(mpirun_args),
          num_procs(num_procs), per_node(per_node), program(program),
          program_args(program_args) {
      set_cmdline(_timeout);
    }

    mpirun::mpirun(uint32_t num_procs, uint32_t per_node,
                   std::vector<std::string> hostlist, std::string mpirun_args,
                   std::string program, std::string program_args,
                   uint8_t _timeout, mpi_impl_t mpi_impl)
        : mpi_impl(mpi_impl), hostlist(hostlist), mpirun_args(mpirun_args),
          num_procs(num_procs), per_node(per_node), program(program),
          program_args(program_args) {
      set_cmdline(_timeout);
    }

    void mpirun::set_cmdline(uint8_t _timeout) {
      /* Build the mpirun command line */
      switch (mpi_impl) {
      case mpi_impl_t::MPICH: {
        if (_timeout > 0) {
          command = wassail::format("MPIEXEC_TIMEOUT={0} ", _timeout);

          /* add a cushion to allow the mpirun timeout a chance to work before
           * invoking the shell_command timeout */
          timeout = _timeout + 10;
        }

        command += "mpirun";

        if (num_procs) {
          command += wassail::format(" -n {0}", num_procs);
        }

        if (per_node) {
          command += wassail::format(" -ppn {0}", per_node);
        }

        if (not hostfile.empty()) {
          command += wassail::format(" -f {0}", hostfile);
        }
        else if (hostlist.size() > 0) {
          /* create comma separated list */
          command += wassail::format(
              " -hosts {0}",
              std::accumulate(hostlist.begin() + 1, hostlist.end(), hostlist[0],
                              [](const std::string &a, std::string b) {
                                return a + "," + b;
                              }));
        }

        if (not mpirun_args.empty()) {
          command += wassail::format(" {0}", mpirun_args);
        }

        if (not program.empty()) {
          if (not program_args.empty()) {
            command += wassail::format(" {0} {1}", program, program_args);
          }
          else {
            command += wassail::format(" {0}", program);
          }
        }

        break;
      }

      case mpi_impl_t::OPENMPI: {
        command = "mpirun";

        if (num_procs) {
          command += wassail::format(" -n {0}", num_procs);
        }

        if (per_node) {
          command += wassail::format(" --npernode {0}", per_node);
        }

        if (not hostfile.empty()) {
          command += wassail::format(" -f {0}", hostfile);
        }
        else if (hostlist.size() > 0) {
          /* create comma separated list */
          command += wassail::format(
              " -H {0}",
              std::accumulate(hostlist.begin() + 1, hostlist.end(), hostlist[0],
                              [](const std::string &a, std::string b) {
                                return a + "," + b;
                              }));
        }

        if (getuid() == 0 and allow_run_as_root) {
          command += " --allow-run-as-root";
        }

        if (_timeout > 0) {
          /* OpenMPI 2.x+ has a --timeout command line parameter, but
           * for compatibility with 1.x, set the environment variable
           * instead */
          command += wassail::format(" -x MPIEXEC_TIMEOUT={0}", _timeout);

          /* add a cushion to allow the mpirun timeout a chance to work before
           * invoking the shell_command timeout */
          timeout = _timeout + 10;
        }

        if (not mpirun_args.empty()) {
          command += wassail::format(" {0}", mpirun_args);
        }

        if (not program.empty()) {
          if (not program_args.empty()) {
            command += wassail::format(" {0} {1}", program, program_args);
          }
          else {
            command += wassail::format(" {0}", program);
          }
        }

        break;
      }
      default: {
        wassail::internal::logger()->warn("unknown MPI implementation");
        command = wassail::format("mpirun -n {0} {1}", num_procs, program);
      }
      }
    }

    void from_json(const json &j, mpirun &d) {
      if (j.value("version", 0) != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<shell_command &>(d));

      d.hostfile = j.value(json::json_pointer("/data/hostfile"), "");
      d.hostlist = j.value(json::json_pointer("/data/hostlist"),
                           std::vector<std::string>({}));

      std::string mpi_impl = j.value(json::json_pointer("/data/mpi_impl"), "");
      if (mpi_impl == "mpich") {
        d.mpi_impl = mpirun::mpi_impl_t::MPICH;
      }
      else if (mpi_impl == "openmpi") {
        d.mpi_impl = mpirun::mpi_impl_t::OPENMPI;
      }
      else {
        throw std::runtime_error(std::string("unknown MPI implementation: ") +
                                 mpi_impl);
      }

      d.mpirun_args = j.value(json::json_pointer("/data/mpirun_args"), "");
      d.num_procs = j.value(json::json_pointer("/data/num_procs"), 0);
      d.per_node = j.value(json::json_pointer("/data/per_node"), 0);
      d.program = j.value(json::json_pointer("/data/program"), "");
      d.program_args = j.value(json::json_pointer("/data/program_args"), "");
    }

    void to_json(json &j, const mpirun &d) {
      j = dynamic_cast<const shell_command &>(d);

      if (not d.hostfile.empty()) {
        j["data"]["hostfile"] = d.hostfile;
      }
      else if (d.hostlist.size() > 0) {
        j["data"]["hostlist"] = d.hostlist;
      }

      j["data"]["mpi_impl"] = d.mpi_impl;
      switch (d.mpi_impl) {
      case mpirun::mpi_impl_t::MPICH: {
        j["data"]["mpi_impl"] = "mpich";
        break;
      }
      case mpirun::mpi_impl_t::OPENMPI: {
        j["data"]["mpi_impl"] = "openmpi";
        break;
      }
      default: { throw std::runtime_error("unknown MPI implementation"); }
      }

      j["data"]["mpirun_args"] = d.mpirun_args;
      j["data"]["num_procs"] = d.num_procs;
      j["data"]["per_node"] = d.per_node;
      j["data"]["program"] = d.program;
      j["data"]["program_args"] = d.program_args;
    }
  } // namespace data
} // namespace wassail
