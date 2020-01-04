/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_INTERNAL_HPP
#define _WASSAIL_INTERNAL_HPP

#include "config.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>
#include <wassail/fmt/format.h>
#if defined(HAVE_EXECUTION_H) && __cpp_lib_execution >= 201603L
#include <execution>
#elif HAVE_DISPATCH_DISPATCH_H
#include <dispatch/dispatch.h>
#elif HAVE_TBB_PARALLEL_FOR_EACH_H
#include <tbb/parallel_for_each.h>
#endif

#include "spdlog/spdlog.h"

namespace wassail {
  namespace internal {
    /*! \brief Return a pointer to the internal logger
     *
     * The internal logger uses the spdlog framework
     */
    std::shared_ptr<spdlog::logger> logger();

#ifdef HAVE_LIBDISPATCH
    namespace libdispatch {
      /*! \brief Wrapper for parallel for_each implemented using libdispatch
       *
       *  Credit:
       * https://xenakios.wordpress.com/2014/09/29/concurrency-in-c-the-cross-platform-way/
       */
      template <typename It, typename F>
      inline void parallel_for_each(It a, It b, F &&f) {
        size_t count = std::distance(a, b);
        using data_t = std::pair<It, F>;
        data_t helper = data_t(a, std::forward<F>(f));
        dispatch_apply_f(
            count,
            dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
            &helper, [](void *ctx, size_t cnt) {
              data_t *d = static_cast<data_t *>(ctx);
              auto elem_it = std::next(d->first, cnt);
              (*d).second(*(elem_it));
            });
      }
    } // namespace libdispatch
#endif

    /*! \brief Indicate whether parallel algorithms are enabled or not.
     *  \return true if parallel algoritms are available, false otherwise
     */
    bool parallel();

    /*! \brief Wrapper for for_each, using a parallel version if available,
     *         otherwise, std::for_each().
     */
    template <class it, class func>
    void for_each(it first, it last, func f) {
#if __cpp_lib_execution >= 201603L
      std::for_each(std::execution::par,
#elif HAVE_LIBDISPATCH
      wassail::internal::libdispatch::parallel_for_each(
#elif HAVE_TBB
      tbb::parallel_for_each(
#else
      std::for_each(
#endif
                    first, last, f);
    }
  } // namespace internal

} // namespace wassail

#endif
