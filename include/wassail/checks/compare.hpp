/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_COMPARE_HPP
#define _WASSAIL_COMPARE_HPP

#include <chrono>
#include <exception>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <wassail/checks/check.hpp>
#include <wassail/common.hpp>
#include <wassail/data/data.hpp>
#include <wassail/json/json.hpp>
#include <wassail/result.hpp>

using json = nlohmann::json;

namespace wassail {
  namespace check {
    /*! \brief Generic comparison check
     *
     *  Compare a value to a reference value and return a result object.
     */
    class compare : public wassail::check::common {
    private:
      /*! \brief Return the input value
       *  \return input value
       */
      template <typename T>
      T value(const T &val) {
        return val;
      }

      /*! \brief Specialization for json
       *  \return string representation of the input json
       */
      decltype(auto) value(const json &j) { return j.dump(); }

    public:
      /*! \brief Constructor */
      compare()
          : common("Comparing value at '{0}' to reference value of '{1}'",
                   "Comparison of observed value '{0}' and reference value "
                   "'{1}' returned false",
                   "Unable to perform comparison: '{0}'",
                   "Comparison of observed value '{0}' and reference value "
                   "'{1}' returned true"){};

      /*! \brief Constructor
       *  \param[in] brief format template for result brief
       *  \param[in] detail_yes format template for result detail case when
       *             issue::YES
       *  \param[in] detail_maybe format template for result detail
       *             case when issue::MAYBE
       *  \param[in] detail_no format template for result detail case when
       *             issue::NO
       */
      compare(std::string brief, std::string detail_yes,
              std::string detail_maybe, std::string detail_no)
          : common(brief, detail_yes, detail_maybe, detail_no){};

      /*! \brief Dummy implementation
       *  \throws std::runtime_error()
       */
      std::shared_ptr<wassail::result> check(const json &j) {
        throw std::runtime_error("Unimplemented function");
      }

      /*! \brief Simple generic comparison check
       *
       *  Compare a value in a JSON object to a reference
       *  value.  The value in the JSON object is specified
       *  by a JSON pointer.
       *
       *  \param[in] j JSON object
       *  \param[in] key JSON pointer to specific key in JSON object
       *  \param[in] cmp comparison functor or lambda function
       *  \param[in] reference_value reference value to compare to the
       *             specified value in the JSON object
       *  \param[in] transform function to apply to the value prior to
       *             performing the check \return result object
       */
      template <typename T, typename Tcompare, typename Ttransform>
      std::shared_ptr<wassail::result>
      check(const json &j, const json::json_pointer &key, Tcompare cmp,
            const T &reference_value, Ttransform transform) {
        std::shared_ptr<wassail::result> r = make_result(j);
        r->format_brief(fmt_str.brief, static_cast<std::string>(key),
                        value(reference_value));

        try {
          T val = transform(j.at(key).get<T>());
          if (cmp(val, reference_value)) {
            r->issue = result::issue_t::NO;
            r->priority = result::priority_t::INFO;
            r->format_detail(fmt_str.detail_no, value(val),
                             value(reference_value));
          }
          else {
            r->issue = result::issue_t::YES;
            r->priority = result::priority_t::WARNING;
            r->format_detail(fmt_str.detail_yes, value(val),
                             value(reference_value));
          }
        }
        catch (std::exception &e) {
          logger(log_level::err, e.what() + std::string(": ") + j.dump());
          r->issue = result::issue_t::MAYBE;
          r->format_detail(fmt_str.detail_maybe, e.what(), j.dump());
        }

        return r;
      }

      /*! \brief Simple generic comparison check
       *
       *  Compare a value in a JSON object to a reference
       *  value.  The value in the JSON object is specified
       *  by a JSON pointer.
       *
       *  \param[in] j JSON object
       *  \param[in] key JSON pointer to specific key in JSON object
       *  \param[in] cmp comparison functor or lambda function
       *  \param[in] reference_value reference value to compare to the
       *             specified value in the JSON object
       *  \return result object
       *
       *  \par Examples
       *  \code{.cpp}
       *  json jin = {{"data", {{"v_int", 4096}, {"v_float", 3.1415},
       *                        {"v_string", "foo"}}}};
       *
       *  auto result1 = wassail::compare::check(
       *      jin, json::json_pointer("/data/v_int"), std::equal_to<int>{}, 1);
       *
       *  auto result2 = wassail::compare::check(
       *      jin, json::json_pointer("/data/v_string"),
       *      std::not_equal_to<std::string>{}, "bar");
       *
       *  auto result3 = wassail::compare::check(
       *      jin, json::json_pointer("/data/v_float"),
       *      [](auto a, auto b){ result a >= b; }, 3.0);
       *
       *  json jcompare = {{"v_int", 4096}, {"v_float", 3.1415},
       *                   {"v_string", "foo"}};
       *  auto result4 = wassail::compare::check(
       *      jin, json::json_pointer("/data"), std::equal_to<json>{},
       *      jcompare);
       *  \endcode
       */
      template <typename T, typename Tcompare>
      std::shared_ptr<wassail::result>
      check(const json &j, const json::json_pointer &key, Tcompare cmp,
            const T &reference_value) {
        return check(j, key, cmp, reference_value,
                     [](const T &val) -> T { return val; });
      }

      /*! \brief Simple generic comparison check
       *
       *  Compare the values in a range of JSON object to a reference
       *  value.  All the values must be true. The value in each JSON object is
       *  specified by a JSON pointer.
       *
       *  \param[in] begin beginning iterator (must be type json)
       *  \param[in] end ending iterator (must be type json)
       *  \param[in] key JSON pointer to specific key in each JSON object
       *  \param[in] cmp comparison functor or lambda function
       *  \param[in] reference_value reference value to compare to the
       *             specified value in the JSON object
       *  \return result object
       *
       *  \par Examples
       *  \code{.cpp}
       *  std::vector<json> jin = ...;
       *  auto result1 = wassail::compare::check(jin.cbegin(), jin.cend(),
       *      json::json_pointer("/data/v_int"), std::equal_to<int>{}, 1);
       *  \endcode
       */
      template <typename Titerator, typename T, typename Tcompare>
      std::shared_ptr<wassail::result>
      check(const Titerator &begin, const Titerator &end,
            const json::json_pointer &key, Tcompare cmp,
            const T &reference_value) {
        static_assert(
            std::is_same<typename std::iterator_traits<Titerator>::value_type,
                         json>::value,
            "iterator type must be json");

        /* overall result, everything will be a child of this result */
        auto r = make_result();

        /* TODO: make this format string configurable */
        auto count = std::distance(begin, end);
        r->format_brief(
            "Comparing {1} values at '{0}' to reference value of '{2}'",
            static_cast<std::string>(key), count, value(reference_value));

        auto child = [&](auto &r, const json j, const json::json_pointer &key,
                         Tcompare cmp, const T &reference_value) {
          auto r_ = check(j, key, cmp, reference_value);
          r->add_child(r_);
        };

        std::for_each(begin, end,
                      std::bind(child, r, std::placeholders::_1, key, cmp,
                                reference_value));

        /* set the issue and priority levels to the maximum child */
        r->issue = r->max_issue();
        r->priority = r->max_priority();

        return r;
      }
    };
  } // namespace check
} // namespace wassail

#endif
