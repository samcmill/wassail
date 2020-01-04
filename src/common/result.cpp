/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "internal.hpp"

#include <algorithm>
#include <config.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
#include <wassail/fmt/format.h>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

using json = nlohmann::json;

static std::shared_timed_mutex mutex;

namespace wassail {
  void result::_flatten(std::vector<std::shared_ptr<result>> &v,
                        std::shared_ptr<result> r) {
    mutex.lock();
    v.insert(v.end(), r->children.begin(), r->children.end());
    mutex.unlock();
    for (auto child : r->children) {
      _flatten(v, child);
    }
  }

  std::vector<std::shared_ptr<result>> result::flat_children() {
    std::vector<std::shared_ptr<result>> v;
    _flatten(v, std::make_shared<result>(*this));
    return v;
  }

  std::string result::format(const std::string fmt, fmt::format_args args) {
    return fmt::vformat(fmt, args);
  }

  void result::add_child(std::shared_ptr<result> child) {
    std::lock_guard<std::shared_timed_mutex> lock(mutex);
    children.push_back(child);
  }

  enum wassail::result::issue_t result::max_issue() {
    auto l = flat_children();
    std::shared_lock<std::shared_timed_mutex> lock(mutex);
    auto it = std::max_element(
        l.begin(), l.end(),
        [](std::shared_ptr<result> r1, std::shared_ptr<result> r2) {
          return r1->issue < r2->issue;
        });
    if (it == l.end()) {
      return issue_t::MAYBE;
    }
    return (*it)->issue;
  }

  bool result::match_issue(enum wassail::result::issue_t _issue) {
    auto l = flat_children();
    std::shared_lock<std::shared_timed_mutex> lock(mutex);
    return std::any_of(
        l.begin(), l.end(),
        [&_issue](std::shared_ptr<result> r) { return r->issue == _issue; });
  }

  enum wassail::result::priority_t result::max_priority() {
    auto l = flat_children();
    std::shared_lock<std::shared_timed_mutex> lock(mutex);
    auto it = std::max_element(
        l.begin(), l.end(),
        [](std::shared_ptr<result> r1, std::shared_ptr<result> r2) {
          return r1->priority < r2->priority;
        });
    if (it == l.end()) {
      return priority_t::NOTICE;
    }

    return (*it)->priority;
  }

  bool result::match_priority(enum wassail::result::priority_t _priority) {
    auto l = flat_children();
    std::shared_lock<std::shared_timed_mutex> lock(mutex);
    return std::any_of(l.begin(), l.end(),
                       [&_priority](std::shared_ptr<result> r) {
                         return r->priority == _priority;
                       });
  }

  std::shared_ptr<wassail::result> make_result(const json &j) {
    auto r = std::make_shared<wassail::result>();

    /* set the result system id if the json contains a hostname entry */
    try {
      r->system_id = {j.at("hostname").get<std::string>()};
    }
    catch (std::exception &e) {
      wassail::internal::logger()->warn("Unable to set system id");
    }

    /* set the result timestamp if the json contains a timestamp entry */
    try {
      r->timestamp = std::chrono::system_clock::from_time_t(
          j.at("timestamp").get<time_t>());
    }
    catch (std::exception &e) {
      wassail::internal::logger()->warn("Unable to set timestamp");
    }

    return r;
  }

  void to_json(json &j, const std::shared_ptr<result> &r) {
    j["action"] = r->action;
    j["brief"] = r->brief;
    j["children"] = r->children;
    j["detail"] = r->detail;
    j["issue"] = r->issue;
    j["priority"] = r->priority;
    j["system_id"] = r->system_id;
    j["timestamp"] = std::chrono::system_clock::to_time_t(r->timestamp);
  }
} // namespace wassail
