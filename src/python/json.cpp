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

using json = nlohmann::json;
namespace py = pybind11;

/* Inter-covert Python and C++ JSON objects
   Credit: Martin Renou, https://github.com/pybind/pybind11/issues/1627 */
namespace nlohmann {
  template <>
  struct adl_serializer<py::object> {
    static py::object from_json(const json &j) {
      py::module py_json = py::module::import("json");

      return py_json.attr("loads")(j.dump());
    }

    static void to_json(json &j, const py::object &obj) {
      py::module py_json = py::module::import("json");

      j = json::parse(
          static_cast<std::string>(py::str(py_json.attr("dumps")(obj))));
    }
  };
} // namespace nlohmann

void json_conversion(py::module &m) {
  /* Inter-convert Python and C++ JSON objects */
  py::class_<json>(m, "json").def(py::init<>()).def(py::init<py::object>());
  py::implicitly_convertible<py::object, json>();
}
