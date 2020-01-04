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

void json_conversion(py::module &);
void py_check(py::module &);
void py_data(py::module &);
void py_enum(py::module &);
void py_result(py::module &);

PYBIND11_MODULE(wassail, m) {
  m.def("initialize", &wassail::initialize);

  /* Inter-convert Python and C++ JSON objects */
  json_conversion(m);

  /* Version functions */
  m.def("version", &wassail::version);
  m.def("version_major", &wassail::version_major);
  m.def("version_minor", &wassail::version_minor);
  m.def("version_micro", &wassail::version_micro);

  /* result class */
  py_result(m);

  /* Enums */
  py_enum(m);

  /* Check building blocks */
  py_check(m);

  /* Data building blocks */
  py_data(m);
}
