/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <chrono>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <wassail/data/getcpuid.hpp>

#ifdef HAVE_CPUID_H
#include <cpuid.h>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class getcpuid::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the load
                                   average has been collected */

      struct {
        uint32_t family;    /*!< Processor family */
        uint32_t model;     /*!< Processor model */
        std::string name;   /*!< Processor brand string */
        uint32_t stepping;  /*!< Processor stepping */
        uint32_t type;      /*!< Processor type */
        std::string vendor; /*!< Processor vendor */
      } data;               /*!< CPU data */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::getcpuid::evaluate() */
      void evaluate(getcpuid &d, bool force);

    private:
      void cpuid(const getcpuid &d);
    };

    getcpuid::getcpuid() : pimpl{std::make_unique<impl>()} {}
    getcpuid::~getcpuid() = default;
    getcpuid::getcpuid(getcpuid &&) = default;            // LCOV_EXCL_LINE
    getcpuid &getcpuid::operator=(getcpuid &&) = default; // LCOV_EXCL_LINE

    bool getcpuid::enabled() const {
#ifdef HAVE_GETCPUID
      return true;
#else
      return false;
#endif
    }

    void getcpuid::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void getcpuid::impl::evaluate(getcpuid &d, bool force = false) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not collected) {
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        cpuid(d);

        d.timestamp = std::chrono::system_clock::now();
        collected = true;
      }
    }

    void getcpuid::impl::cpuid(const getcpuid &d) {
#ifdef HAVE_GETCPUID
      unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
      int rv;

      /* Vendor string */
      rv = __get_cpuid(0, &eax, &ebx, &ecx, &edx);
      if (rv == 0) {
        wassail::internal::logger()->warn(
            "Processor vendor string not available");
      }
      else {
        data.vendor = std::string(reinterpret_cast<const char *>(&ebx), 4) +
                      std::string(reinterpret_cast<const char *>(&edx), 4) +
                      std::string(reinterpret_cast<const char *>(&ecx), 4);
      }

      /* Processor info */
      rv = __get_cpuid(1, &eax, &ebx, &ecx, &edx);
      data.stepping = eax & 0xF;
      uint32_t model = (eax >> 4) & 0xF;
      uint32_t family = (eax >> 8) & 0xF;
      data.type = (eax >> 12) & 0x3;
      uint32_t extended_model = (eax >> 16) & 0xF;
      uint32_t extended_family = (eax >> 20) & 0xFF;

      data.family = family;
      data.model = model;

      if (family == 0xF) {
        data.family = extended_family + family;
      }

      if (family == 0x6 or family == 0xF) {
        data.model = (extended_model << 4) + model;
      }

      /* Processor name */
      rv = __get_cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
      if (eax >= 0x80000004) {
        __get_cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
        data.name = std::string(reinterpret_cast<const char *>(&eax), 4) +
                    std::string(reinterpret_cast<const char *>(&ebx), 4) +
                    std::string(reinterpret_cast<const char *>(&ecx), 4) +
                    std::string(reinterpret_cast<const char *>(&edx), 4);

        __get_cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
        data.name += std::string(reinterpret_cast<const char *>(&eax), 4) +
                     std::string(reinterpret_cast<const char *>(&ebx), 4) +
                     std::string(reinterpret_cast<const char *>(&ecx), 4) +
                     std::string(reinterpret_cast<const char *>(&edx), 4);

        __get_cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
        data.name += std::string(reinterpret_cast<const char *>(&eax), 4) +
                     std::string(reinterpret_cast<const char *>(&ebx), 4) +
                     std::string(reinterpret_cast<const char *>(&ecx), 4) +
                     /* Last item is a null */
                     std::string(reinterpret_cast<const char *>(&edx), 4 - 1);

        /* Trim leading whitespace */
        data.name.erase(0, data.name.find_first_not_of(" \t\n\r\f\v"));
      }
      else {
        wassail::internal::logger()->warn(
            "Processor brand string not available");
      }

      return;
#else
      throw std::runtime_error("__getcpuid() not available");
#endif
    }
    /* \endcond */

    void from_json(const json &j, getcpuid &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      try {
        auto jdata = j.at("data");
        d.pimpl->data.family = jdata.at("family").get<uint32_t>();
        d.pimpl->data.model = jdata.at("model").get<uint32_t>();
        d.pimpl->data.name = jdata.at("name").get<std::string>();
        d.pimpl->data.stepping = jdata.at("stepping").get<uint32_t>();
        d.pimpl->data.type = jdata.at("type").get<uint32_t>();
        d.pimpl->data.vendor = jdata.at("vendor").get<std::string>();
      }
      catch (std::exception &e) {
        throw std::runtime_error(
            std::string("Unable to convert JSON string '") + j.dump() +
            std::string("' to object: ") + e.what());
      }
    }

    void to_json(json &j, const getcpuid &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["family"] = d.pimpl->data.family;
      j["data"]["model"] = d.pimpl->data.model;
      j["data"]["type"] = d.pimpl->data.type;
      j["data"]["stepping"] = d.pimpl->data.stepping;
      j["data"]["name"] = d.pimpl->data.name;
      j["data"]["vendor"] = d.pimpl->data.vendor;
      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
