/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_RULES_ENGINE_HPP
#define _WASSAIL_RULES_ENGINE_HPP

#include <functional>
#include <vector>
#include <wassail/checks/check.hpp>
#include <wassail/common.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

using json = nlohmann::json;

namespace wassail {
  namespace check {
    using rules_t = std::function<bool(const json)>;

    /*! \brief Simple rules engine
     *
     *  Define one or more rules using lambda functions and then check
     *  that all rules are obeyed.
     *
     *  Since the rules are arbitrary, the result format strings will
     *  typically need to be set by the caller.
     *
     *  \par Examples
     *  \code{.cpp}
     *  json jin = {{"data", {{"v_int", 4096}, {"v_float", 3.1415},
     *                        {"v_string", "foo"}}}};
     *
     *  auto c = wassail::check::rules_engine();
     *  c.add_rule([](json j) {
     *    return j.at(json::json_pointer("/data/v_int")).get<int>() == 4096;
     *  });
     *  c.add_rule([](json j) {
     *    return j.at(json::json_pointer("/data/v_float")).get<float>() >= 3;
     *  });
     *  auto r = c.check(jin);
     *  \endcode
     */
    class rules_engine : public wassail::check::common {
    private:
      /*! \brief List of rules
       */
      std::vector<rules_t> rules;

    public:
      /*! \brief Constructor */
      rules_engine()
          : common("Checking data against rules", "Rule criteria not met",
                   "Unable to perform comparison: '{0}'",
                   "Rule criteria met"){};

      /*! \brief Constructor
       *  \param[in] brief format template for result brief
       *  \param[in] detail_yes format template for result detail case when
       *             issue::YES
       *  \param[in] detail_maybe format template for result detail
       *             case when issue::MAYBE
       *  \param[in] detail_no format template for result detail case when
       *             issue::NO
       */
      rules_engine(std::string brief, std::string detail_yes,
                   std::string detail_maybe, std::string detail_no)
          : common(brief, detail_yes, detail_maybe, detail_no){};

      /*! \brief Add new rule
       *
       *  The rule signature must be bool(json).
       *
       *  \param[in] rule Rule expression (lambda) to add
       */
      void add_rule(const rules_t &rule);

      /*! \brief Apply all rules
       *  \param[in] j JSON object
       *  \return result object
       */
      std::shared_ptr<wassail::result> check(const json &j);

      /*! \brief Apply all rules
       *  Also set the result brief and detail strings based on the arguments.
       *
       *  \param[in] j JSON object
       *  \param[in] args argument list
       *  \return result object
       */
      template <typename... T>
      std::shared_ptr<wassail::result> check(const json &j, const T &... args) {
        auto r = rules_engine::check(j);

        r->brief = wassail::format(fmt_str.brief, args...);

        if (r->issue == wassail::result::issue_t::YES) {
          r->detail = wassail::format(fmt_str.detail_yes, args...);
        }
        else if (r->issue == wassail::result::issue_t::NO) {
          r->detail = wassail::format(fmt_str.detail_no, args...);
        }

        return r;
      }
    };
  } // namespace check
} // namespace wassail

#endif
