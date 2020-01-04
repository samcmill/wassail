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

void py_enum(py::module &m) {
  /* Enums */

  py::enum_<wassail::log_level>(m, "log_level", py::arithmetic())
      .value("trace", wassail::log_level::trace)
      .value("debug", wassail::log_level::debug)
      .value("info", wassail::log_level::info)
      .value("warn", wassail::log_level::warn)
      .value("err", wassail::log_level::err)
      .value("critical", wassail::log_level::critical)
      .value("off", wassail::log_level::off);

  py::enum_<wassail::result::issue_t>(m, "issue_t", py::arithmetic())
      .value("YES", wassail::result::issue_t::YES)
      .value("MAYBE", wassail::result::issue_t::MAYBE)
      .value("NO", wassail::result::issue_t::NO);

  py::enum_<wassail::result::priority_t>(m, "priority_t", py::arithmetic())
      .value("EMERGENCY", wassail::result::priority_t::EMERGENCY)
      .value("ALERT", wassail::result::priority_t::ALERT)
      .value("CRITICAL", wassail::result::priority_t::CRITICAL)
      .value("ERROR", wassail::result::priority_t::ERROR)
      .value("WARNING", wassail::result::priority_t::WARNING)
      .value("NOTICE", wassail::result::priority_t::NOTICE)
      .value("INFO", wassail::result::priority_t::INFO)
      .value("DEBUG", wassail::result::priority_t::DEBUG);
}
