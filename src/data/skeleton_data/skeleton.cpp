/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <chrono>
#include <cstdlib>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <wassail/data/skeleton.hpp>

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class skeleton::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

      /*! \brief System data */
      struct {
      } data; /*!< System data */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::skeleton::evaluate() */
      void evaluate(skeleton &d, bool force);
    };

    skeleton::skeleton() : pimpl{std::make_unique<impl>()} {}
    skeleton::~skeleton() = default;
    skeleton::skeleton(skeleton &&) = default;            // LCOV_EXCL_LINE
    skeleton &skeleton::operator=(skeleton &&) = default; // LCOV_EXCL_LINE

    bool skeleton::enabled() const {
#ifdef WITH_DATA_SKELETON
      return true;
#else
      return false;
#endif
    }

    void skeleton::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void skeleton::impl::evaluate(skeleton &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not collected) {
#ifdef WITH_DATA_SKELETON
        /* collect data */
#else
        throw std::runtime_error("skeleton data source is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, skeleton &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      /* d.pimpl->data.foo = j.value(json::json_pointer("/data/foo"), 0); */
    }

    void to_json(json &j, const skeleton &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      /* j["data"]["foo"] = d.pimpl->data.foo; */
      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
