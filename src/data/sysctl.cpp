/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <wassail/data/sysctl.hpp>

#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#ifndef HAVE_FIXPT_T
typedef uint32_t fixpt_t;
#endif

#ifndef HAVE_STRUCT_LOADAVG
/* \cond type_compat */
struct loadavg {
  fixpt_t ldavg[3]; /*!< 1, 5, and 15 minute load averages */
  long fscale;      /*!< Scaling factor */
};
/* \endcond */
#endif

#ifndef HAVE_STRUCT_XSW_USAGE
/* \cond type_compat */
struct xsw_usage {
  uint64_t xsu_total;    /*!< Total virtual memory */
  uint64_t xsu_avail;    /*!< Available virtual memory */
  uint64_t xsu_used;     /*!< Used virtual memory */
  uint32_t xsu_pagesize; /*!< Size of virtual memory page */
  bool xsu_encrypted;    /*!< Encryption state */
};
/* \endcond */
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class sysctl::impl {
    public:
      /*! \brief System information */
      struct {
        struct {
          int32_t cpufamily = 0;        /*!< Processor family */
          int64_t cpufrequency = 0;     /*!< Current CPU frequency in Hz */
          int64_t cpufrequency_max = 0; /*!< Max CPU frequency in Hz */
          int32_t cputype = 0;          /*!< mach-o CPU type */
          int32_t logicalcpu = 0;       /*!< Current number of logical
                                           processors */
          int32_t logicalcpu_max = 0;   /*!< Max number of logical processors */
          std::string machine;          /*!< Machine class */
          int64_t memsize = 0;          /*!< Physical memory size in bytes */
          std::string model;            /*!< Specific machine model */
          int32_t ncpu = 0;             /*!< Maximum number of CPUs */
          int32_t packages = 0;         /*!< Number of processor packages */
          int32_t physicalcpu = 0;      /*!< Current number of physical
                                           processors */
          int32_t physicalcpu_max = 0;  /*!< Max number of physical
                                           processors */
        } hw; /*!< Generic CPU, I/O, etc. information */

        struct {
          std::string hostname;  /*!< System hostname */
          std::string osrelease; /*!< System release string */
          int32_t osrevision;    /*!< System revision number */
          std::string ostype;    /*!< System type string */
          std::string osversion; /*!< System version string */
          std::string version;   /*!< System version string */
        } kern;                  /*!< Kernel infomation */

        struct {
          struct {
            std::string brand_string;    /*!< CPU brand string */
            int32_t core_count;          /*!< Number of cores */
            int32_t cores_per_package;   /*!< Number of cores per CPU
                                            package */
            int32_t family;              /*!< CPU family */
            int32_t logical_per_package; /*!< Number of cores per CPU
                                            packages */
            int32_t model;               /*!< CPU model */
            int32_t stepping;            /*!< CPU stepping */
            std::string vendor;          /*!< CPU vendor string */
          } cpu;                         /*!< CPU */
        } machdep;                       /*!< Machine dependent */

        struct {
          struct loadavg loadavg;     /*!< Load averages */
          struct xsw_usage swapusage; /*!< Swap memory */
        } vm;                         /*!< Virtual memory */
      } data;                         /*!< System information */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::sysctl::evaluate() */
      void evaluate(sysctl &d, bool force);

    private:
      /*! Helper function to wrap sysctlbyname()
       * \param[in] name Name of the sysctl field to query
       * \param[out] var The value of the sysctl field
       * \return 0 if successful, non-zero otherwise
       */
      template <typename T>
      int _sysctlbyname(const sysctl &d, const std::string name, T *var);

      /*! Helper function to wrap sysctlbyname()
       * \param[in] name Name of the sysctl field to query
       * \param[out] var The value of the sysctl field
       * \return 0 if successful, non-zero otherwise
       */
      int _sysctlbyname(const sysctl &d, const std::string name,
                        std::string *var);
    };

    sysctl::sysctl() : pimpl{std::make_unique<impl>()} {}
    sysctl::~sysctl() = default;
    sysctl::sysctl(sysctl &&) = default;            // LCOV_EXCL_LINE
    sysctl &sysctl::operator=(sysctl &&) = default; // LCOV_EXCL_LINE

    bool sysctl::enabled() const {
#ifdef HAVE_SYSCTLBYNAME
      return true;
#else
      return false;
#endif
    }

    void sysctl::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void sysctl::impl::evaluate(sysctl &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not d.collected()) {
#ifdef HAVE_SYSCTLBYNAME
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        _sysctlbyname(d, "hw.cpufamily", &(d.pimpl->data.hw.cpufamily));
        _sysctlbyname(d, "hw.cpufrequency", &(d.pimpl->data.hw.cpufrequency));
        _sysctlbyname(d, "hw.cpufrequency_max",
                      &(d.pimpl->data.hw.cpufrequency_max));
        _sysctlbyname(d, "hw.cputype", &(d.pimpl->data.hw.cputype));
        _sysctlbyname(d, "hw.logicalcpu", &(d.pimpl->data.hw.logicalcpu));
        _sysctlbyname(d, "hw.logicalcpu_max",
                      &(d.pimpl->data.hw.logicalcpu_max));
        _sysctlbyname(d, "hw.machine", &(d.pimpl->data.hw.machine));
        _sysctlbyname(d, "hw.memsize", &(d.pimpl->data.hw.memsize));
        _sysctlbyname(d, "hw.model", &(d.pimpl->data.hw.model));
        _sysctlbyname(d, "hw.ncpu", &(d.pimpl->data.hw.ncpu));
        _sysctlbyname(d, "hw.packages", &(d.pimpl->data.hw.packages));
        _sysctlbyname(d, "hw.physicalcpu", &(d.pimpl->data.hw.physicalcpu));
        _sysctlbyname(d, "hw.physicalcpu_max",
                      &(d.pimpl->data.hw.physicalcpu_max));

        _sysctlbyname(d, "kern.hostname", &(d.pimpl->data.kern.hostname));
        _sysctlbyname(d, "kern.osrelease", &(d.pimpl->data.kern.osrelease));
        _sysctlbyname(d, "kern.osrevision", &(d.pimpl->data.kern.osrevision));
        _sysctlbyname(d, "kern.ostype", &(d.pimpl->data.kern.ostype));
        _sysctlbyname(d, "kern.osversion", &(d.pimpl->data.kern.osversion));
        _sysctlbyname(d, "kern.version", &(d.pimpl->data.kern.version));

        _sysctlbyname(d, "machdep.cpu.brand_string",
                      &(d.pimpl->data.machdep.cpu.brand_string));
        _sysctlbyname(d, "machdep.cpu.core_count",
                      &(d.pimpl->data.machdep.cpu.core_count));
        _sysctlbyname(d, "machdep.cpu.cores_per_package",
                      &(d.pimpl->data.machdep.cpu.cores_per_package));
        _sysctlbyname(d, "machdep.cpu.family",
                      &(d.pimpl->data.machdep.cpu.family));
        _sysctlbyname(d, "machdep.cpu.logical_per_package",
                      &(d.pimpl->data.machdep.cpu.logical_per_package));
        _sysctlbyname(d, "machdep.cpu.model",
                      &(d.pimpl->data.machdep.cpu.model));
        _sysctlbyname(d, "machdep.cpu.stepping",
                      &(d.pimpl->data.machdep.cpu.stepping));
        _sysctlbyname(d, "machdep.cpu.vendor",
                      &(d.pimpl->data.machdep.cpu.vendor));

        _sysctlbyname(d, "vm.loadavg", &(d.pimpl->data.vm.loadavg));
        _sysctlbyname(d, "vm.swapusage", &(d.pimpl->data.vm.swapusage));

        d.common::evaluate(force);
#else
        throw std::runtime_error("sysctlbyname not available");
#endif
      }
    }

    template <typename T>
    int sysctl::impl::_sysctlbyname(const sysctl &d, const std::string name,
                                    T *var) {
#ifdef HAVE_SYSCTLBYNAME
      size_t len = sizeof(*var);
      int rv = sysctlbyname(name.c_str(), var, &len, NULL, 0);
      if (rv != 0) {
        wassail::internal::logger()->warn(
            "Unsuccessfully read sysctl field '{}'", name);
      }
      return rv;
#else
      throw std::runtime_error("sysctlbyname not available");
#endif
    }

    int sysctl::impl::_sysctlbyname(const sysctl &d, const std::string name,
                                    std::string *var) {
#ifdef HAVE_SYSCTLBYNAME
      size_t len;

      /* Get the size of the string being returned */
      int rv = sysctlbyname(name.c_str(), NULL, &len, NULL, 0);
      if (rv != 0) {
        wassail::internal::logger()->warn(
            "Unsuccessfully read sysctl field '{}'", name);
        return rv;
      }

      char *s = (char *)malloc(len * sizeof(char));

      /* Get the string */
      rv = sysctlbyname(name.c_str(), s, &len, NULL, 0);
      if (rv != 0) {
        wassail::internal::logger()->warn(
            "Unsuccessfully read sysctl field '{}'", name);
        free(s);
        return rv;
      }

      *var = std::string(s);

      free(s);

      return rv;
#else
      throw std::runtime_error("sysctlbyname() not available");
#endif
    }
    /* \endcond */

    void from_json(const json &j, sysctl &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("name", "") != d.name()) {
        throw std::runtime_error("name mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      d.pimpl->data.hw.cpufamily =
          j.value(json::json_pointer("/data/hw/cpufamily"), 0L);
      d.pimpl->data.hw.cpufrequency =
          j.value(json::json_pointer("/data/hw/cpufrequency"), 0LL);
      d.pimpl->data.hw.cpufrequency_max =
          j.value(json::json_pointer("/data/hw/cpufrequency_max"), 0LL);
      d.pimpl->data.hw.cputype =
          j.value(json::json_pointer("/data/hw/cputype"), 0L);
      d.pimpl->data.hw.logicalcpu =
          j.value(json::json_pointer("/data/hw/logicalcpu"), 0L);
      d.pimpl->data.hw.logicalcpu_max =
          j.value(json::json_pointer("/data/hw/logicalcpu_max"), 0L);
      d.pimpl->data.hw.machine =
          j.value(json::json_pointer("/data/hw/machine"), "");
      d.pimpl->data.hw.memsize =
          j.value(json::json_pointer("/data/hw/memsize"), 0LL);
      d.pimpl->data.hw.model =
          j.value(json::json_pointer("/data/hw/model"), "");
      d.pimpl->data.hw.ncpu = j.value(json::json_pointer("/data/hw/ncpu"), 0L);
      d.pimpl->data.hw.packages =
          j.value(json::json_pointer("/data/hw/packages"), 0L);
      d.pimpl->data.hw.physicalcpu =
          j.value(json::json_pointer("/data/hw/physicalcpu"), 0L);
      d.pimpl->data.hw.physicalcpu_max =
          j.value(json::json_pointer("/data/hw/physicalcpu_max"), 0L);

      d.pimpl->data.kern.hostname =
          j.value(json::json_pointer("/data/kern/hostname"), "");
      d.pimpl->data.kern.osrelease =
          j.value(json::json_pointer("/data/kern/osrelease"), "");
      d.pimpl->data.kern.osrevision =
          j.value(json::json_pointer("/data/kern/osrevision"), 0L);
      d.pimpl->data.kern.ostype =
          j.value(json::json_pointer("/data/kern/ostype"), "");
      d.pimpl->data.kern.osversion =
          j.value(json::json_pointer("/data/kern/osversion"), "");
      d.pimpl->data.kern.version =
          j.value(json::json_pointer("/data/kern/version"), "");

      d.pimpl->data.machdep.cpu.brand_string =
          j.value(json::json_pointer("/data/machdep/cpu/brand_string"), "");
      d.pimpl->data.machdep.cpu.core_count =
          j.value(json::json_pointer("/data/machdep/cpu/core_count"), 0L);
      d.pimpl->data.machdep.cpu.cores_per_package = j.value(
          json::json_pointer("/data/machdep/cpu/cores_per_package"), 0L);
      d.pimpl->data.machdep.cpu.family =
          j.value(json::json_pointer("/data/machdep/cpu/family"), 0L);
      d.pimpl->data.machdep.cpu.logical_per_package = j.value(
          json::json_pointer("/data/machdep/cpu/logical_per_package"), 0L);
      d.pimpl->data.machdep.cpu.model =
          j.value(json::json_pointer("/data/machdep/cpu/model"), 0L);
      d.pimpl->data.machdep.cpu.stepping =
          j.value(json::json_pointer("/data/machdep/cpu/stepping"), 0L);
      d.pimpl->data.machdep.cpu.vendor =
          j.value(json::json_pointer("/data/machdep/cpu/vendor"), "");

      d.pimpl->data.vm.loadavg.fscale =
          j.value(json::json_pointer("/data/vm/loadavg/fscale"), 0L);
      d.pimpl->data.vm.loadavg.ldavg[0] =
          j.value(json::json_pointer("/data/vm/loadavg/load1"),
                  static_cast<fixpt_t>(0));
      d.pimpl->data.vm.loadavg.ldavg[1] =
          j.value(json::json_pointer("/data/vm/loadavg/load5"),
                  static_cast<fixpt_t>(0));
      d.pimpl->data.vm.loadavg.ldavg[2] =
          j.value(json::json_pointer("/data/vm/loadavg/load15"),
                  static_cast<fixpt_t>(0));

      d.pimpl->data.vm.swapusage.xsu_total =
          j.value(json::json_pointer("/data/vm/swapusage/xsu_total"), 0ULL);
      d.pimpl->data.vm.swapusage.xsu_avail =
          j.value(json::json_pointer("/data/vm/swapusage/xsu_avail"), 0ULL);
      d.pimpl->data.vm.swapusage.xsu_used =
          j.value(json::json_pointer("/data/vm/swapusage/xsu_used"), 0ULL);
      d.pimpl->data.vm.swapusage.xsu_pagesize =
          j.value(json::json_pointer("/data/vm/swapusage/xsu_pagesize"), 0ULL);
    }

    void to_json(json &j, const sysctl &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["hw"]["cpufamily"] = d.pimpl->data.hw.cpufamily;
      j["data"]["hw"]["cpufrequency"] = d.pimpl->data.hw.cpufrequency;
      j["data"]["hw"]["cpufrequency_max"] = d.pimpl->data.hw.cpufrequency_max;
      j["data"]["hw"]["cputype"] = d.pimpl->data.hw.cputype;
      j["data"]["hw"]["logicalcpu"] = d.pimpl->data.hw.logicalcpu;
      j["data"]["hw"]["logicalcpu_max"] = d.pimpl->data.hw.logicalcpu_max;
      j["data"]["hw"]["machine"] = d.pimpl->data.hw.machine;
      j["data"]["hw"]["memsize"] = d.pimpl->data.hw.memsize;
      j["data"]["hw"]["model"] = d.pimpl->data.hw.model;
      j["data"]["hw"]["ncpu"] = d.pimpl->data.hw.ncpu;
      j["data"]["hw"]["packages"] = d.pimpl->data.hw.packages;
      j["data"]["hw"]["physicalcpu"] = d.pimpl->data.hw.physicalcpu;
      j["data"]["hw"]["physicalcpu_max"] = d.pimpl->data.hw.physicalcpu_max;
      j["data"]["kern"]["hostname"] = d.pimpl->data.kern.hostname;
      j["data"]["kern"]["osrelease"] = d.pimpl->data.kern.osrelease;
      j["data"]["kern"]["osrevision"] = d.pimpl->data.kern.osrevision;
      j["data"]["kern"]["ostype"] = d.pimpl->data.kern.ostype;
      j["data"]["kern"]["osversion"] = d.pimpl->data.kern.osversion;
      j["data"]["kern"]["version"] = d.pimpl->data.kern.version;
      j["data"]["machdep"]["cpu"]["brand_string"] =
          d.pimpl->data.machdep.cpu.brand_string;
      j["data"]["machdep"]["cpu"]["core_count"] =
          d.pimpl->data.machdep.cpu.core_count;
      j["data"]["machdep"]["cpu"]["cores_per_package"] =
          d.pimpl->data.machdep.cpu.cores_per_package;
      j["data"]["machdep"]["cpu"]["family"] = d.pimpl->data.machdep.cpu.family;
      j["data"]["machdep"]["cpu"]["logical_per_package"] =
          d.pimpl->data.machdep.cpu.logical_per_package;
      j["data"]["machdep"]["cpu"]["model"] = d.pimpl->data.machdep.cpu.model;
      j["data"]["machdep"]["cpu"]["stepping"] =
          d.pimpl->data.machdep.cpu.stepping;
      j["data"]["machdep"]["cpu"]["vendor"] = d.pimpl->data.machdep.cpu.vendor;
      j["data"]["vm"]["loadavg"]["fscale"] = d.pimpl->data.vm.loadavg.fscale;
      j["data"]["vm"]["loadavg"]["load1"] = d.pimpl->data.vm.loadavg.ldavg[0];
      j["data"]["vm"]["loadavg"]["load5"] = d.pimpl->data.vm.loadavg.ldavg[1];
      j["data"]["vm"]["loadavg"]["load15"] = d.pimpl->data.vm.loadavg.ldavg[2];
      j["data"]["vm"]["swapusage"]["xsu_total"] =
          d.pimpl->data.vm.swapusage.xsu_total;
      j["data"]["vm"]["swapusage"]["xsu_avail"] =
          d.pimpl->data.vm.swapusage.xsu_avail;
      j["data"]["vm"]["swapusage"]["xsu_used"] =
          d.pimpl->data.vm.swapusage.xsu_used;
      j["data"]["vm"]["swapusage"]["xsu_pagesize"] =
          d.pimpl->data.vm.swapusage.xsu_pagesize;
      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
