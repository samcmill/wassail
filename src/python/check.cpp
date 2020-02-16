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
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#pragma GCC diagnostic pop

#include <pybind11_json/pybind11_json.hpp>

#include <wassail/json/json.hpp>
#include <wassail/wassail.hpp>

using json = nlohmann::json;
namespace py = pybind11;

void py_check(py::module &m) {
  /* wassail.check.compare() is templated, so no Python binding */

  py::module check = m.def_submodule("check", "Check building blocks");

  py::class_<wassail::check::rules_engine>(check, "rules_engine")
      .def(py::init<>())
      .def(py::init<std::string, std::string, std::string, std::string>())
      .def("add_rule", &wassail::check::rules_engine::add_rule)
      .def("check", &wassail::check::rules_engine::check);

  py::module check_cpu =
      check.def_submodule("cpu", "CPU check building blocks");

  py::class_<wassail::check::cpu::core_count>(check_cpu, "core_count")
      .def(py::init<>())
      .def(py::init<uint16_t>())
      .def(py::init<uint16_t, std::string, std::string, std::string,
                    std::string>())
      .def("check", py::overload_cast<const json &>(
                        &wassail::check::cpu::core_count::check))
      .def("check", py::overload_cast<wassail::data::sysconf &>(
                        &wassail::check::cpu::core_count::check))
      .def("check", py::overload_cast<wassail::data::sysctl &>(
                        &wassail::check::cpu::core_count::check));

  py::module check_disk =
      check.def_submodule("disk", "Disk check building blocks");

  py::class_<wassail::check::disk::amount_free>(check_disk, "amount_free")
      .def(py::init<std::string>())
      .def(py::init<std::string, uint64_t>())
      .def(py::init<std::string, uint64_t, std::string, std::string,
                    std::string, std::string>())
      .def("check", py::overload_cast<const json &>(
                        &wassail::check::disk::amount_free::check))
      .def("check", py::overload_cast<wassail::data::getfsstat &>(
                        &wassail::check::disk::amount_free::check))
      .def("check", py::overload_cast<wassail::data::getmntent &>(
                        &wassail::check::disk::amount_free::check));

  py::class_<wassail::check::disk::percent_free>(check_disk, "percent_free")
      .def(py::init<std::string>())
      .def(py::init<std::string, float>())
      .def(py::init<std::string, float, std::string, std::string, std::string,
                    std::string>())
      .def("check", py::overload_cast<const json &>(
                        &wassail::check::disk::percent_free::check))
      .def("check", py::overload_cast<wassail::data::getfsstat &>(
                        &wassail::check::disk::percent_free::check))
      .def("check", py::overload_cast<wassail::data::getmntent &>(
                        &wassail::check::disk::percent_free::check));

  py::module check_memory =
      check.def_submodule("memory", "Memory check building blocks");

  py::class_<wassail::check::memory::physical_size>(check_memory,
                                                    "physical_size")
      .def(py::init<>())
      .def(py::init<uint64_t>())
      .def(py::init<uint64_t, uint64_t>())
      .def(py::init<uint64_t, uint64_t, std::string, std::string, std::string,
                    std::string>())
      .def("check", py::overload_cast<const json &>(
                        &wassail::check::memory::physical_size::check))
      .def("check", py::overload_cast<wassail::data::sysconf &>(
                        &wassail::check::memory::physical_size::check))
      .def("check", py::overload_cast<wassail::data::sysctl &>(
                        &wassail::check::memory::physical_size::check))
      .def("check", py::overload_cast<wassail::data::sysinfo &>(
                        &wassail::check::memory::physical_size::check));

  py::module check_misc =
      check.def_submodule("misc", "Miscellaneous check building blocks");

  py::class_<wassail::check::misc::environment>(check_misc, "environment")
      .def(py::init<std::string, std::string>())
      .def(py::init<std::string, std::string, bool>())
      .def(py::init<std::string, std::string, bool, std::string, std::string,
                    std::string, std::string>())
      .def("check", py::overload_cast<const json &>(
                        &wassail::check::misc::environment::check))
      .def("check", py::overload_cast<wassail::data::environment &>(
                        &wassail::check::misc::environment::check));

  py::class_<wassail::check::misc::load_average_1min>(check_misc,
                                                      "load_average_1min")
      .def(py::init<>())
      .def(py::init<float>())
      .def(
          py::init<float, std::string, std::string, std::string, std::string>())
      .def("check", py::overload_cast<const json &>(
                        &wassail::check::misc::load_average_1min::check))
      .def("check", py::overload_cast<wassail::data::getloadavg &>(
                        &wassail::check::misc::load_average_1min::check))
      .def("check", py::overload_cast<wassail::data::sysctl &>(
                        &wassail::check::misc::load_average_1min::check))
      .def("check", py::overload_cast<wassail::data::sysinfo &>(
                        &wassail::check::misc::load_average_1min::check));

  py::class_<wassail::check::misc::load_average_5min>(check_misc,
                                                      "load_average_5min")
      .def(py::init<>())
      .def(py::init<float>())
      .def(
          py::init<float, std::string, std::string, std::string, std::string>())
      .def("check", py::overload_cast<const json &>(
                        &wassail::check::misc::load_average_5min::check))
      .def("check", py::overload_cast<wassail::data::getloadavg &>(
                        &wassail::check::misc::load_average_5min::check))
      .def("check", py::overload_cast<wassail::data::sysctl &>(
                        &wassail::check::misc::load_average_5min::check))
      .def("check", py::overload_cast<wassail::data::sysinfo &>(
                        &wassail::check::misc::load_average_5min::check));

  py::class_<wassail::check::misc::load_average_15min>(check_misc,
                                                       "load_average_15min")
      .def(py::init<>())
      .def(py::init<float>())
      .def(
          py::init<float, std::string, std::string, std::string, std::string>())
      .def("check", py::overload_cast<const json &>(
                        &wassail::check::misc::load_average_15min::check))
      .def("check", py::overload_cast<wassail::data::getloadavg &>(
                        &wassail::check::misc::load_average_15min::check))
      .def("check", py::overload_cast<wassail::data::sysctl &>(
                        &wassail::check::misc::load_average_15min::check))
      .def("check", py::overload_cast<wassail::data::sysinfo &>(
                        &wassail::check::misc::load_average_15min::check));
}
