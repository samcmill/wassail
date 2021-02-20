/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* warning: ISO C++1z does not allow ‘register’ storage class specifier
   [-Wregister] */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wregister"
#include <pybind11/pybind11.h>

#include <pybind11/cast.h>
#include <pybind11/stl.h>
#pragma GCC diagnostic pop

#include <wassail/json/json.hpp>
#include <wassail/wassail.hpp>

using json = nlohmann::json;
namespace py = pybind11;

#define MAKE_DATA_CLASS(MODULE, NAME)                                          \
  py::class_<wassail::data::NAME>(MODULE, #NAME)                               \
      .def(py::init<>())                                                       \
      .def("__str__",                                                          \
           [](const wassail::data::NAME &d) {                                  \
             return static_cast<json>(d).dump(-1, ' ', false,                  \
                                              json::error_handler_t::replace); \
           })                                                                  \
      .def("enabled", &wassail::data::NAME ::enabled)                          \
      .def("evaluate", &wassail::data::NAME ::evaluate,                        \
           py::arg("force") = false);

void py_data(py::module &m) {
  py::module data = m.def_submodule("data", "Data building blocks");

  /* factory method */
  data.def("evaluate",
           py::overload_cast<const json &>(&wassail::data::evaluate));

  MAKE_DATA_CLASS(data, environment)
  MAKE_DATA_CLASS(data, getcpuid)
  MAKE_DATA_CLASS(data, getfsstat)
  MAKE_DATA_CLASS(data, getloadavg)
  MAKE_DATA_CLASS(data, getmntent)
  MAKE_DATA_CLASS(data, getrlimit)
  MAKE_DATA_CLASS(data, nvml)
  MAKE_DATA_CLASS(data, pciaccess)
  MAKE_DATA_CLASS(data, pciutils)
  MAKE_DATA_CLASS(data, ps)
  MAKE_DATA_CLASS(data, stream)
  MAKE_DATA_CLASS(data, sysconf)
  MAKE_DATA_CLASS(data, sysctl)
  MAKE_DATA_CLASS(data, sysinfo)
  MAKE_DATA_CLASS(data, udev)
  MAKE_DATA_CLASS(data, umad)
  MAKE_DATA_CLASS(data, uname)

  /* special case, unique constructor */
  py::enum_<wassail::data::mpirun::mpi_impl_t>(data, "mpi_impl_t",
                                               py::arithmetic())
      .value("MPICH", wassail::data::mpirun::mpi_impl_t::MPICH)
      .value("OPENMPI", wassail::data::mpirun::mpi_impl_t::OPENMPI);

  py::class_<wassail::data::mpirun>(data, "mpirun")
      .def(py::init<uint32_t, std::string>())
      .def(py::init<uint32_t, std::string, wassail::data::mpirun::mpi_impl_t>())
      .def(py::init<uint32_t, std::string, std::string>())
      .def(py::init<uint32_t, std::vector<std::string>, std::string>())
      .def(py::init<uint32_t, uint32_t, std::string, std::string, std::string,
                    std::string, uint8_t, wassail::data::mpirun::mpi_impl_t>())
      .def(py::init<uint32_t, uint32_t, std::vector<std::string>, std::string,
                    std::string, std::string, uint8_t,
                    wassail::data::mpirun::mpi_impl_t>())
      .def("__str__",
           [](const wassail::data::mpirun &d) {
             return static_cast<json>(d).dump();
           })
      .def("enabled", &wassail::data::mpirun::enabled)
      .def("evaluate", &wassail::data::mpirun::evaluate,
           py::arg("force") = false);

  /* special case, unique constructor */
  py::enum_<wassail::data::osu_micro_benchmarks::osu_benchmark_t>(
      data, "osu_benchmark_t", py::arithmetic())
      .value("ALLTOALL",
             wassail::data::osu_micro_benchmarks::osu_benchmark_t::ALLTOALL)
      .value("BW", wassail::data::osu_micro_benchmarks::osu_benchmark_t::BW)
      .value("HELLO",
             wassail::data::osu_micro_benchmarks::osu_benchmark_t::HELLO)
      .value("INIT", wassail::data::osu_micro_benchmarks::osu_benchmark_t::INIT)
      .value("LATENCY",
             wassail::data::osu_micro_benchmarks::osu_benchmark_t::LATENCY);

  py::class_<wassail::data::osu_micro_benchmarks>(data, "osu_micro_benchmarks")
      .def(py::init<>())
      .def(py::init<uint32_t>())
      .def(py::init<uint32_t,
                    wassail::data::osu_micro_benchmarks::osu_benchmark_t,
                    wassail::data::mpirun::mpi_impl_t>())
      .def(py::init<uint32_t, std::string,
                    wassail::data::osu_micro_benchmarks::osu_benchmark_t,
                    wassail::data::mpirun::mpi_impl_t>())
      .def(py::init<uint32_t, std::vector<std::string>,
                    wassail::data::osu_micro_benchmarks::osu_benchmark_t,
                    wassail::data::mpirun::mpi_impl_t>())
      .def(py::init<uint32_t, uint32_t, std::string, std::string,
                    wassail::data::osu_micro_benchmarks::osu_benchmark_t,
                    uint8_t, wassail::data::mpirun::mpi_impl_t>())
      .def(py::init<uint32_t, uint32_t, std::vector<std::string>, std::string,
                    wassail::data::osu_micro_benchmarks::osu_benchmark_t,
                    uint8_t, wassail::data::mpirun::mpi_impl_t>())
      .def("__str__",
           [](const wassail::data::osu_micro_benchmarks &d) {
             return static_cast<json>(d).dump();
           })
      .def("enabled", &wassail::data::osu_micro_benchmarks::enabled)
      .def("evaluate", &wassail::data::osu_micro_benchmarks::evaluate,
           py::arg("force") = false);

  /* special case, unique constructor */
  py::class_<wassail::data::remote_shell_command>(data, "remote_shell_command")
      .def(py::init<std::string, std::string>())
      .def(py::init<std::list<std::string>, std::string>())
      .def(py::init<std::list<std::string>, std::string, uint8_t>())
      .def("__str__",
           [](const wassail::data::remote_shell_command &d) {
             return static_cast<json>(d).dump();
           })
      .def("enabled", &wassail::data::remote_shell_command::enabled)
      .def("evaluate", &wassail::data::remote_shell_command::evaluate,
           py::arg("force") = false);

  /* special case, unique constructor */
  py::class_<wassail::data::shell_command>(data, "shell_command")
      .def(py::init<std::string>())
      .def(py::init<std::string, uint8_t>())
      .def("__str__",
           [](const wassail::data::shell_command &d) {
             return static_cast<json>(d).dump();
           })
      .def("enabled", &wassail::data::shell_command::enabled)
      .def("evaluate", &wassail::data::shell_command::evaluate,
           py::arg("force") = false);

  /* special case, unique constructor */
  py::class_<wassail::data::stat>(data, "stat")
      .def(py::init<std::string>())
      .def("__str__",
           [](const wassail::data::stat &d) {
             return static_cast<json>(d).dump();
           })
      .def("enabled", &wassail::data::stat::enabled)
      .def("evaluate", &wassail::data::stat::evaluate,
           py::arg("force") = false);
}
