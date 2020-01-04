/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_RESULT_HPP
#define _WASSAIL_RESULT_HPP

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <wassail/fmt/format.h>
#include <wassail/json/json.hpp>

using json = nlohmann::json;

namespace wassail {
  /*! \brief Result of a check */
  class result {
  private:
    /*! \brief Recusively add children to the list */
    void _flatten(std::vector<std::shared_ptr<result>> &l,
                  std::shared_ptr<result> r);

    /*! \brief Construct a list of all children, grandchildren, etc.
     *  \return List of children
     */
    std::vector<std::shared_ptr<result>> flat_children();

    /*! \brief Return a formatted string */
    std::string format(const std::string fmt, fmt::format_args args);

  public:
    std::string action; /*!< Action to take in response to an issue */

    std::string brief; /*!< Brief (a few words) description */

    std::vector<std::shared_ptr<result>> children; /*!< Child results */

    std::string detail; /*!< Detailed description, may be several sentences */

    /*! \brief Issue state of the result
     */
    enum class issue_t {
      YES = 2,                /*!< Is an issue */
      MAYBE = 1,              /*!< May or may not be an issue */
      NO = 0                  /*!< Is not an issue */
    } issue = issue_t::MAYBE; /*!< Issue state of the result */

    /*! \brief Priority of the result
     *
     *  Corresponds to the priority levels defined in RFC 5424.
     */
    enum class priority_t {
      EMERGENCY = 7, /*!< Emergency: system is unusable */
      ALERT = 6,     /*!< Alert: action must be taken immediately */
      CRITICAL = 5,  /*!< Critical: critical conditions */
      ERROR = 4,     /*!< Error: error conditions */
      WARNING = 3,   /*!< Warning: warning conditions */
      NOTICE = 2,    /*!< Notice: normal but significant condition */
      INFO = 1,      /*!< Informational: informational messages */
      DEBUG = 0      /*!< Debug: debug-level messages */
    } priority = priority_t::NOTICE; /*!< Priority of the result */

    /*! System ID
     *  The system ID is a vector to handle the multiple node case.
     *  In most circumstances only a single system id will be
     *  used.
     */
    std::vector<std::string> system_id;

    /*! Timestamp of the result */
    std::chrono::time_point<std::chrono::system_clock> timestamp;

    /*! Constructor */
    result(){};

    /*! Constructor */
    result(std::string _brief) { brief = _brief; };

    /*! \brief Add child result
     *  \param[in] child Child result object
     */
    void add_child(std::shared_ptr<result> child);

    /*! \brief Set the action string based on a format template and an
     *   argument list.  The format template syntax description can be
     *   found at http://fmtlib.net/latest/syntax.html.
     *
     * \param[in] fmt format template
     * \param[in] args argument list
     */
    template <typename... T>
    void format_action(const std::string fmt, const T &... args) {
      action = format(fmt, fmt::make_format_args(args...));
    }

    /*! \brief Set the brief string based on a format template and an
     *   argument list.  The format template syntax description can be
     *   found at http://fmtlib.net/latest/syntax.html.
     *
     * \param[in] fmt format template
     * \param[in] args argument list
     */
    template <typename... T>
    void format_brief(const std::string fmt, const T &... args) {
      brief = format(fmt, fmt::make_format_args(args...));
    }

    /*! \brief Set the detail string based on a format template and an
     *   argument list.  The format template syntax description can be
     *   found at http://fmtlib.net/latest/syntax.html.
     *
     * \param[in] fmt format template
     * \param[in] args argument list
     */
    template <typename... T>
    void format_detail(const std::string fmt, const T &... args) {
      detail = format(fmt, fmt::make_format_args(args...));
    }

    /*! \brief Return the maximum (i.e., most serious) issue state
     *  among all children, grandchildren, etc.
     * \return issue
     */
    enum issue_t max_issue();

    /*! \brief Return true if any children, grandchildren, etc. match
     *  the specified issue state
     *  \param[in] _issue issue state
     *  \return True if matched, false otherwise
     */
    bool match_issue(enum issue_t _issue);

    /*! \brief Return the maximum (i.e., most serious) priority
     *  among all children, grandchildren, etc.
     * \return priority
     */
    enum priority_t max_priority();

    /*! \brief Return true if any children, grandchildren, etc. match
     *  the specified priority state
     *  \param[in] _priority priority state
     *  \return True if matched, false otherwise
     */
    bool match_priority(enum priority_t _priority);

    /*! Output stream operator overload */
    friend std::ostream &operator<<(std::ostream &,
                                    std::shared_ptr<wassail::result> const &);

    /*! Output stream operator overload */
    friend std::ostream &operator<<(std::ostream &,
                                    enum wassail::result::issue_t const &);

    /*! Output stream operator overload */
    friend std::ostream &operator<<(std::ostream &,
                                    enum wassail::result::priority_t const &);

    /*! JSON type conversion
     * \param[in,out] j JSON object
     * \param[in] r
     */
    friend void to_json(json &j, const std::shared_ptr<result> &r);
  };

  /*! Convenience function for making result instances */
  inline std::shared_ptr<wassail::result> make_result() {
    return std::make_shared<wassail::result>();
  };

  /*! Convenience function for making result instances
   *
   * Also populate result system_id and timestamp from data source building
   * block.
   *
   * \param[in] j JSON object
   */
  std::shared_ptr<wassail::result> make_result(const json &j);
} // namespace wassail

#endif
