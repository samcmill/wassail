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
#include <pybind11/chrono.h>
#include <pybind11/stl.h>
#pragma GCC diagnostic pop

#include <wassail/json/json.hpp>
#include <wassail/wassail.hpp>

using json = nlohmann::json;
namespace py = pybind11;

void py_result(py::module &m) {
  m.def("make_result", py::overload_cast<>(&wassail::make_result))
      .def("make_result",
           py::overload_cast<const json &>(&wassail::make_result));

  py::class_<wassail::result, std::shared_ptr<wassail::result>>(m, "result")
      .def(py::init<>())
      .def(py::init<const std::string &>())
      .def("__str__",
           [](const std::shared_ptr<wassail::result> &r) {
             return static_cast<json>(r).dump();
           })
      .def("add_child", &wassail::result::add_child)
      .def("max_issue", &wassail::result::max_issue)
      .def("max_priority", &wassail::result::max_priority)
      .def("match_issue", &wassail::result::match_issue)
      .def("match_priority", &wassail::result::match_priority)
      .def("propagate", &wassail::result::propagate)
      .def_readwrite("action", &wassail::result::action)
      .def_readwrite("brief", &wassail::result::brief)
      .def_readwrite("children", &wassail::result::children)
      .def_readwrite("detail", &wassail::result::detail)
      .def_readwrite("issue", &wassail::result::issue)
      .def_readwrite("priority", &wassail::result::priority)
      .def_readwrite("system_id", &wassail::result::system_id)
      .def_readwrite("timestamp", &wassail::result::timestamp);
}
