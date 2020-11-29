/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <cstdlib>
#include <list>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <wassail/data/nvml.hpp>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#ifdef HAVE_NVML_H
#include <nvml.h>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class nvml::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

      /* \brief NvLink attributes */
      struct nvlink {
        bool active = false;      /*!< NvLink active state */
        unsigned int version = 0; /*!< NvLink version */
      };                          /*!< NvLink attributes */

      /* \brief GPU device attributes */
      struct gpu {
        struct {
          unsigned long long free = 0;  /*!< Unallocated BAR1 memory, bytes */
          unsigned long long total = 0; /*!< Total BAR1 memory, bytes */
          unsigned long long used = 0;  /*!< Allocated BAR1 memory, bytes */
        } bar1;                         /*!< BAR1 memory */

        std::string board_part_number; /*!< Board part number */
        uint16_t brand;                /*!< Brand of the device */

        struct {
          struct {
            unsigned int graphics = 0; /*!< Graphics clock, MHz */
            unsigned int memory = 0;   /*!< Memory clock, MHz */
            unsigned int sm = 0;       /*!< SM clock, MHz */
          } current;                   /*!< Current actual clock values */

          struct {
            unsigned int graphics = 0; /*!< Graphics clock, MHz */
            unsigned int memory = 0;   /*!< Memory clock, MHz */
            unsigned int sm = 0;       /*!< SM clock, MHz */
          } default_;                  /*!< Default application clock target */

          struct {
            unsigned int graphics = 0; /*!< Graphics clock, MHz */
            unsigned int memory = 0;   /*!< Memory clock, MHz */
            unsigned int sm = 0;       /*!< SM clock, MHz */
          } target;                    /*!< Target application clock */
        } clock;                       /*!< Device clock */

        uint16_t compute_mode; /*!< Current compute mode */

        struct {
          int major = 0;           /*!< Major version */
          int minor = 0;           /*!< Minor version */
        } cuda_compute_capability; /*!< CUDA compute capability */

        struct {
          bool current; /*!< Current ECC mode */
          struct {
            struct {
              unsigned long long corrected;   /*!< Corrected ECC errors */
              unsigned long long uncorrected; /*!< Uncorrected ECC errors */
            } aggregate;                      /*!< Aggregate ECC errors */
            struct {
              unsigned long long corrected;   /*!< Corrected ECC errors */
              unsigned long long uncorrected; /*!< Uncorrected ECC errors */
            } volatile_;                      /*!< Volatile ECC errors */
          } errors;                           /*!< ECC errors */
          bool pending;                       /*!< Pending ECC mode */
        } ecc;                                /*!< ECC information */

        unsigned int index = 0; /*!< GPU index */

        struct {
          unsigned int checksum = 0; /*!< InfoROM configuration checksum */
          std::string ecc_version;   /*!< ECC InfoROM version */
          std::string image_version; /*!< InfoROM image version */
          std::string oem_version;   /*!< OEM InfoROM version */
          std::string power_version; /*!< Power InfoROM version */
        } inforom;                   /*!< InfoROM information */

        struct {
          unsigned long long free = 0;  /*!< Unallocated FB memory, bytes */
          unsigned long long total = 0; /*!< Total FB memory, bytes */
          unsigned long long used = 0;  /*!< Allocated FB memory, bytes */
        } memory;                       /*!< Frame buffer memory */

        unsigned int minor_number; /*!< Minor number for the device */
        std::string name;          /*!< GPU product name */
        std::list<nvlink> nvlinks; /*!< List of device's NvLinks */

        struct {
          unsigned int bus = 0;       /*!< Bus on which the device resides */
          std::string bus_id;         /*!< domain:bus:device.function tuple */
          unsigned int device;        /*!< Device's ID on the bus */
          unsigned int device_id = 0; /*!< combined device and vendor IDs */
          unsigned int
              domain; /*!< PCI domain on which the device's bus resides */
          unsigned int generation = 0;   /*!< PCIE link generation */
          unsigned int subsystem_id = 0; /*!< sub-system device ID */
          unsigned int width = 0;        /*!< PCIE link width */
        } pcie;                          /*!< PCIE information */

        int pstate; /*!< Power state */

        struct {
          unsigned int double_bit =
              0; /*!< Number of retired pages due to a double bit error */
          bool pending;                /*!< Pages pending retirement? */
          unsigned int single_bit = 0; /*!< Number of retired pages due to
                                          multiple single bit errors */
        } retired_pages;               /*!< Retired pages information */

        unsigned int temperature; /*!< Temperature, Celsius */
        std::string serial;       /*!< GPU serial number */
        std::string uuid;         /*!< GPU UUID */
        std::string vbios;        /*!< VBIOS version */
      };                          /*!< GPU device attributes */

      /*! \brief GPUs */
      struct {
        int cuda_driver_version;    /*!< CUDA driver version */
        std::list<gpu> devices;     /*!< List of GPU devices */
        std::string driver_version; /*!< Driver version */
        std::string nvml_version;   /*!< NVML library version */
      } data;                       /*!< GPU devices */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::nvml::evaluate() */
      void evaluate(nvml &d, bool force);

    private:
#ifdef WITH_DATA_NVML
      void *handle = nullptr; /*!< Library handle */

      void device_query(nvmlDevice_t device);
      nvml::impl::nvlink nvlink_query(nvmlDevice_t device, unsigned int link);
      void system_query();

      void if_nvml_success(const nvmlReturn_t rv, std::function<void()> f) {
        if (rv == NVML_SUCCESS) {
          f();
        }
      };

      template <class T>
      std::function<T> load_symbol(std::string const &name) {
        void *const symbol = dlsym(handle, name.c_str());

        if (not symbol) {
          throw std::runtime_error(dlerror());
        }

        return reinterpret_cast<T *>(symbol);
      }
#endif
    };

    nvml::nvml() : pimpl{std::make_unique<impl>()} {}
    nvml::~nvml() = default;
    nvml::nvml(nvml &&) = default;            // LCOV_EXCL_LINE
    nvml &nvml::operator=(nvml &&) = default; // LCOV_EXCL_LINE

    bool nvml::enabled() const {
#ifdef WITH_DATA_NVML
      return true;
#else
      return false;
#endif
    }

    void nvml::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void nvml::impl::evaluate(nvml &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not collected) {
#ifdef WITH_DATA_NVML
        /* collect data */
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        nvmlReturn_t rv;
        unsigned int num_devices = 0;

        handle = dlopen("libnvidia-ml.so", RTLD_LAZY);
        if (not handle) {
          /* no libnvidia.so, try libnvidia.so.1 also */
          handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
          if (not handle) {
            wassail::internal::logger()->error(
                "unable to load libnvidia-ml library: {}", dlerror());
            return;
          }
        }

        /* Load symbols */
        auto const _nvmlDeviceGetCount =
            load_symbol<nvmlReturn_t(unsigned int *)>("nvmlDeviceGetCount");
        auto const _nvmlDeviceGetHandleByIndex =
            load_symbol<nvmlReturn_t(unsigned int, nvmlDevice_t *)>(
                "nvmlDeviceGetHandleByIndex");
        auto const _nvmlInit = load_symbol<nvmlReturn_t()>("nvmlInit");
        auto const _nvmlShutdown = load_symbol<nvmlReturn_t()>("nvmlShutdown");

        rv = _nvmlInit();
        if (rv != NVML_SUCCESS) {
          wassail::internal::logger()->error(
              "unable to initialize libnvidia-ml library");
          dlclose(handle);
          return;
        }

        /* Make system queries */
        system_query();

        /* Get number of GPU devices */
        rv = _nvmlDeviceGetCount(&num_devices);
        if (rv != NVML_SUCCESS) {
          wassail::internal::logger()->error("no GPUs found");
        }

        /* Query each GPU */
        for (unsigned int i = 0; i < num_devices; i++) {
          nvmlDevice_t d;

          rv = _nvmlDeviceGetHandleByIndex(i, &d);
          if (rv == NVML_SUCCESS) {
            /* adds a gpu to the device list */
            device_query(d);
          }
          else {
            wassail::internal::logger()->error("error looking up GPU {}", i);
          }
        }

        rv = _nvmlShutdown();
        if (rv != NVML_SUCCESS) {
          wassail::internal::logger()->error("error shutting down nvml");
        }

        d.common::evaluate(force);
        collected = true;

        dlclose(handle);
#else
        throw std::runtime_error("nvml data source is not available");
#endif
      }
    } // namespace data

#ifdef WITH_DATA_NVML
    void nvml::impl::device_query(nvmlDevice_t device) {
      nvmlReturn_t rv;
      nvml::impl::gpu gpu;

      /* BAR1 memory info */
      {
        auto const _nvmlDeviceGetBAR1MemoryInfo =
            load_symbol<nvmlReturn_t(nvmlDevice_t, nvmlBAR1Memory_t *)>(
                "nvmlDeviceGetBAR1MemoryInfo");
        nvmlBAR1Memory_t bar1_memory;
        rv = _nvmlDeviceGetBAR1MemoryInfo(device, &bar1_memory);
        if_nvml_success(rv, [&]() {
          gpu.bar1.free = bar1_memory.bar1Free;
          gpu.bar1.total = bar1_memory.bar1Total;
          gpu.bar1.used = bar1_memory.bar1Used;
        });
      }

      /* Board part number */
      {
        auto const _nvmlDeviceGetBoardPartNumber =
            load_symbol<nvmlReturn_t(nvmlDevice_t, char *, unsigned int)>(
                "nvmlDeviceGetBoardPartNumber");
        char part_number[NVML_DEVICE_PART_NUMBER_BUFFER_SIZE];
        rv = _nvmlDeviceGetBoardPartNumber(device, part_number,
                                           sizeof(part_number));
        if_nvml_success(rv, [&]() { gpu.board_part_number = part_number; });
      }

      /* Brand ID */
      {
        auto const _nvmlDeviceGetBrand =
            load_symbol<nvmlReturn_t(nvmlDevice_t, nvmlBrandType_t *)>(
                "nvmlDeviceGetBrand");
        nvmlBrandType_t brand;
        rv = _nvmlDeviceGetBrand(device, &brand);
        if_nvml_success(rv, [&]() { gpu.brand = brand; });
      }

      /* Clocks */
      {
        auto const _nvmlDeviceGetClock = load_symbol<nvmlReturn_t(
            nvmlDevice_t, nvmlClockType_t, nvmlClockId_t, unsigned int *)>(
            "nvmlDeviceGetClock");
        unsigned int clock;
        rv = _nvmlDeviceGetClock(device, NVML_CLOCK_GRAPHICS,
                                 NVML_CLOCK_ID_CURRENT, &clock);
        if_nvml_success(rv, [&]() { gpu.clock.current.graphics = clock; });
        rv = _nvmlDeviceGetClock(device, NVML_CLOCK_GRAPHICS,
                                 NVML_CLOCK_ID_APP_CLOCK_TARGET, &clock);
        if_nvml_success(rv, [&]() { gpu.clock.target.graphics = clock; });
        rv = _nvmlDeviceGetClock(device, NVML_CLOCK_GRAPHICS,
                                 NVML_CLOCK_ID_APP_CLOCK_DEFAULT, &clock);
        if_nvml_success(rv, [&]() { gpu.clock.default_.graphics = clock; });

        rv = _nvmlDeviceGetClock(device, NVML_CLOCK_SM, NVML_CLOCK_ID_CURRENT,
                                 &clock);
        if_nvml_success(rv, [&]() { gpu.clock.current.sm = clock; });
        rv = _nvmlDeviceGetClock(device, NVML_CLOCK_SM,
                                 NVML_CLOCK_ID_APP_CLOCK_TARGET, &clock);
        if_nvml_success(rv, [&]() { gpu.clock.target.sm = clock; });
        rv = _nvmlDeviceGetClock(device, NVML_CLOCK_SM,
                                 NVML_CLOCK_ID_APP_CLOCK_DEFAULT, &clock);
        if_nvml_success(rv, [&]() { gpu.clock.default_.sm = clock; });

        rv = _nvmlDeviceGetClock(device, NVML_CLOCK_MEM, NVML_CLOCK_ID_CURRENT,
                                 &clock);
        if_nvml_success(rv, [&]() { gpu.clock.current.memory = clock; });
        rv = _nvmlDeviceGetClock(device, NVML_CLOCK_MEM,
                                 NVML_CLOCK_ID_APP_CLOCK_TARGET, &clock);
        if_nvml_success(rv, [&]() { gpu.clock.target.memory = clock; });
        rv = _nvmlDeviceGetClock(device, NVML_CLOCK_MEM,
                                 NVML_CLOCK_ID_APP_CLOCK_DEFAULT, &clock);
        if_nvml_success(rv, [&]() { gpu.clock.default_.memory = clock; });
      }

      /* Compute mode */
      {
        auto const _nvmlDeviceGetComputeMode =
            load_symbol<nvmlReturn_t(nvmlDevice_t, nvmlComputeMode_t *)>(
                "nvmlDeviceGetComputeMode");
        nvmlComputeMode_t compute_mode;
        rv = _nvmlDeviceGetComputeMode(device, &compute_mode);
        if_nvml_success(rv, [&]() { gpu.compute_mode = compute_mode; });
      }

      /* CUDA compute capability */
      {
        auto const _nvmlDeviceGetCudaComputeCapability =
            load_symbol<nvmlReturn_t(nvmlDevice_t, int *, int *)>(
                "nvmlDeviceGetCudaComputeCapability");
        int major, minor;
        rv = _nvmlDeviceGetCudaComputeCapability(device, &major, &minor);
        if_nvml_success(rv, [&]() {
          gpu.cuda_compute_capability.major = major;
          gpu.cuda_compute_capability.minor = minor;
        });
      }

      /* Index */
      {
        auto const _nvmlDeviceGetIndex =
            load_symbol<nvmlReturn_t(nvmlDevice_t, unsigned int *)>(
                "nvmlDeviceGetIndex");
        unsigned int index;
        rv = _nvmlDeviceGetIndex(device, &index);
        if_nvml_success(rv, [&]() { gpu.index = index; });
      }

      /* Memory info */
      {
        auto const _nvmlDeviceGetMemoryInfo =
            load_symbol<nvmlReturn_t(nvmlDevice_t, nvmlMemory_t *)>(
                "nvmlDeviceGetMemoryInfo");
        nvmlMemory_t memory;
        rv = _nvmlDeviceGetMemoryInfo(device, &memory);
        if_nvml_success(rv, [&]() {
          gpu.memory.free = memory.free;
          gpu.memory.total = memory.total;
          gpu.memory.used = memory.used;
        });
      }

      /* NvLink */
      {
        auto const _nvmlDeviceGetFieldValues =
            load_symbol<nvmlReturn_t(nvmlDevice_t, int, nvmlFieldValue_t *)>(
                "nvmlDeviceGetFieldValues");
        nvmlFieldValue_t fv;
        fv.fieldId = NVML_FI_DEV_NVLINK_LINK_COUNT;
        rv = _nvmlDeviceGetFieldValues(device, 1, &fv);
        if (rv == NVML_SUCCESS and fv.nvmlReturn == NVML_SUCCESS and
            fv.valueType == NVML_VALUE_TYPE_UNSIGNED_INT) {
          unsigned int nvlink_count = fv.value.uiVal;
          for (unsigned int i = 0; i < nvlink_count; i++) {
            /* adds a link to the nvlink list */
            auto nvlink = nvlink_query(device, i);
            gpu.nvlinks.push_back(nvlink);
          }
        }
      }

      /* Product name */
      {
        auto const _nvmlDeviceGetName =
            load_symbol<nvmlReturn_t(nvmlDevice_t, char *, unsigned int)>(
                "nvmlDeviceGetName");
        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        rv = _nvmlDeviceGetName(device, name, sizeof(name));
        if_nvml_success(rv, [&]() { gpu.name = name; });
      }

      /* PCIe info */
      {
        auto const _nvmlDeviceGetCurrPcieLinkGeneration =
            load_symbol<nvmlReturn_t(nvmlDevice_t, unsigned int *)>(
                "nvmlDeviceGetCurrPcieLinkGeneration");
        unsigned int link_gen;
        rv = _nvmlDeviceGetCurrPcieLinkGeneration(device, &link_gen);
        if_nvml_success(rv, [&]() { gpu.pcie.generation = link_gen; });

        auto const _nvmlDeviceGetCurrPcieLinkWidth =
            load_symbol<nvmlReturn_t(nvmlDevice_t, unsigned int *)>(
                "nvmlDeviceGetCurrPcieLinkWidth");
        unsigned int link_width;
        rv = _nvmlDeviceGetCurrPcieLinkWidth(device, &link_width);
        if_nvml_success(rv, [&]() { gpu.pcie.width = link_width; });

        auto const _nvmlDeviceGetPciInfo =
            load_symbol<nvmlReturn_t(nvmlDevice_t, nvmlPciInfo_t *)>(
                "nvmlDeviceGetPciInfo");
        nvmlPciInfo_t pci_info;
        rv = _nvmlDeviceGetPciInfo(device, &pci_info);
        if_nvml_success(rv, [&]() {
          /* pci_info.busId does not return a valid string, so build it */
          char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];
          snprintf(busId, sizeof(busId), NVML_DEVICE_PCI_BUS_ID_FMT,
                   NVML_DEVICE_PCI_BUS_ID_FMT_ARGS(&pci_info));

          gpu.pcie.bus = pci_info.bus;
          gpu.pcie.bus_id = busId;
          gpu.pcie.device = pci_info.device;
          gpu.pcie.device_id = pci_info.pciDeviceId;
          gpu.pcie.domain = pci_info.domain;
          gpu.pcie.subsystem_id = pci_info.pciSubSystemId;
        });
      }

      /* ECC */
      {
        auto const _nvmlDeviceGetEccMode = load_symbol<nvmlReturn_t(
            nvmlDevice_t, nvmlEnableState_t *, nvmlEnableState_t *)>(
            "nvmlDeviceGetEccMode");
        nvmlEnableState_t ecc_current, ecc_pending;
        rv = _nvmlDeviceGetEccMode(device, &ecc_current, &ecc_pending);
        if_nvml_success(rv, [&]() {
          gpu.ecc.current = ecc_current == NVML_FEATURE_ENABLED;
          gpu.ecc.pending = ecc_pending == NVML_FEATURE_ENABLED;
        });

        auto const _nvmlDeviceGetTotalEccErrors = load_symbol<nvmlReturn_t(
            nvmlDevice_t, nvmlMemoryErrorType_t, nvmlEccCounterType_t,
            unsigned long long *)>("nvmlDeviceGetTotalEccErrors");
        unsigned long long ecc_count;
        rv = _nvmlDeviceGetTotalEccErrors(device,
                                          NVML_MEMORY_ERROR_TYPE_CORRECTED,
                                          NVML_AGGREGATE_ECC, &ecc_count);
        if_nvml_success(
            rv, [&]() { gpu.ecc.errors.aggregate.corrected = ecc_count; });

        rv = _nvmlDeviceGetTotalEccErrors(device,
                                          NVML_MEMORY_ERROR_TYPE_UNCORRECTED,
                                          NVML_AGGREGATE_ECC, &ecc_count);
        if_nvml_success(
            rv, [&]() { gpu.ecc.errors.aggregate.uncorrected = ecc_count; });

        rv = _nvmlDeviceGetTotalEccErrors(device,
                                          NVML_MEMORY_ERROR_TYPE_CORRECTED,
                                          NVML_VOLATILE_ECC, &ecc_count);
        if_nvml_success(
            rv, [&]() { gpu.ecc.errors.volatile_.corrected = ecc_count; });

        rv = _nvmlDeviceGetTotalEccErrors(device,
                                          NVML_MEMORY_ERROR_TYPE_UNCORRECTED,
                                          NVML_VOLATILE_ECC, &ecc_count);
        if_nvml_success(
            rv, [&]() { gpu.ecc.errors.volatile_.uncorrected = ecc_count; });
      }

      /* InfoROM */
      {
        auto const _nvmlDeviceGetInforomConfigurationChecksum =
            load_symbol<nvmlReturn_t(nvmlDevice_t, unsigned int *)>(
                "nvmlDeviceGetInforomConfigurationChecksum");
        unsigned int inforom_checksum;
        rv = _nvmlDeviceGetInforomConfigurationChecksum(device,
                                                        &inforom_checksum);
        if_nvml_success(rv, [&]() { gpu.inforom.checksum = inforom_checksum; });

        auto const _nvmlDeviceGetInforomImageVersion =
            load_symbol<nvmlReturn_t(nvmlDevice_t, char *, unsigned int)>(
                "nvmlDeviceGetInforomImageVersion");
        char inforom_image_version[NVML_DEVICE_INFOROM_VERSION_BUFFER_SIZE];
        rv = _nvmlDeviceGetInforomImageVersion(device, inforom_image_version,
                                               sizeof(inforom_image_version));
        if_nvml_success(
            rv, [&]() { gpu.inforom.image_version = inforom_image_version; });

        auto const _nvmlDeviceGetInforomVersion = load_symbol<nvmlReturn_t(
            nvmlDevice_t, nvmlInforomObject_t, char *, unsigned int)>(
            "nvmlDeviceGetInforomVersion");
        char inforom_ecc_version[NVML_DEVICE_INFOROM_VERSION_BUFFER_SIZE];
        rv = _nvmlDeviceGetInforomVersion(device, NVML_INFOROM_ECC,
                                          inforom_ecc_version,
                                          sizeof(inforom_ecc_version));
        if_nvml_success(
            rv, [&]() { gpu.inforom.ecc_version = inforom_ecc_version; });

        char inforom_power_version[NVML_DEVICE_INFOROM_VERSION_BUFFER_SIZE];
        rv = _nvmlDeviceGetInforomVersion(device, NVML_INFOROM_POWER,
                                          inforom_power_version,
                                          sizeof(inforom_power_version));
        if_nvml_success(
            rv, [&]() { gpu.inforom.power_version = inforom_power_version; });

        char inforom_oem_version[NVML_DEVICE_INFOROM_VERSION_BUFFER_SIZE];
        rv = _nvmlDeviceGetInforomVersion(device, NVML_INFOROM_OEM,
                                          inforom_oem_version,
                                          sizeof(inforom_oem_version));
        if_nvml_success(
            rv, [&]() { gpu.inforom.oem_version = inforom_oem_version; });
      }

      /* Minor version number */
      {
        auto const _nvmlDeviceGetMinorNumber =
            load_symbol<nvmlReturn_t(nvmlDevice_t, unsigned int *)>(
                "nvmlDeviceGetMinorNumber");
        unsigned int minor_number;
        rv = _nvmlDeviceGetMinorNumber(device, &minor_number);
        if_nvml_success(rv, [&]() { gpu.minor_number = minor_number; });
      }

      /* Power state */
      {
        auto const _nvmlDeviceGetPowerState =
            load_symbol<nvmlReturn_t(nvmlDevice_t, nvmlPstates_t *)>(
                "nvmlDeviceGetPowerState");
        nvmlPstates_t pstate;
        rv = _nvmlDeviceGetPowerState(device, &pstate);
        if_nvml_success(rv, [&]() { gpu.pstate = pstate; });
      }

      /* Retired pages */
      {
        auto const _nvmlDeviceGetRetiredPages =
            load_symbol<nvmlReturn_t(nvmlDevice_t, nvmlPageRetirementCause_t,
                                     unsigned int *, unsigned long long *)>(
                "nvmlDeviceGetRetiredPages");
        unsigned int page_count = 0;
        rv = _nvmlDeviceGetRetiredPages(
            device, NVML_PAGE_RETIREMENT_CAUSE_MULTIPLE_SINGLE_BIT_ECC_ERRORS,
            &page_count, NULL);
        if_nvml_success(rv,
                        [&]() { gpu.retired_pages.single_bit = page_count; });

        page_count = 0;
        rv = _nvmlDeviceGetRetiredPages(
            device, NVML_PAGE_RETIREMENT_CAUSE_DOUBLE_BIT_ECC_ERROR,
            &page_count, NULL);
        if_nvml_success(rv,
                        [&]() { gpu.retired_pages.double_bit = page_count; });

        auto const _nvmlDeviceGetRetiredPagesPendingStatus =
            load_symbol<nvmlReturn_t(nvmlDevice_t, nvmlEnableState_t *)>(
                "nvmlDeviceGetRetiredPagesPendingStatus");
        nvmlEnableState_t pending_retirement;
        rv = _nvmlDeviceGetRetiredPagesPendingStatus(device,
                                                     &pending_retirement);
        if_nvml_success(rv, [&]() {
          gpu.retired_pages.pending =
              pending_retirement == NVML_FEATURE_ENABLED;
        });
      }

      /* Serial number */
      {
        auto const _nvmlDeviceGetSerial =
            load_symbol<nvmlReturn_t(nvmlDevice_t, char *, unsigned int)>(
                "nvmlDeviceGetSerial");
        char serial[NVML_DEVICE_SERIAL_BUFFER_SIZE];
        rv = _nvmlDeviceGetSerial(device, serial, sizeof(serial));
        if_nvml_success(rv, [&]() { gpu.serial = serial; });
      }

      /* Temperature */
      {
        auto const _nvmlDeviceGetTemperature = load_symbol<nvmlReturn_t(
            nvmlDevice_t, nvmlTemperatureSensors_t, unsigned int *)>(
            "nvmlDeviceGetTemperature");
        unsigned int temperature;
        rv = _nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU,
                                       &temperature);
        if_nvml_success(rv, [&]() { gpu.temperature = temperature; });
      }

      /* UUID */
      {
        auto const _nvmlDeviceGetUUID =
            load_symbol<nvmlReturn_t(nvmlDevice_t, char *, unsigned int)>(
                "nvmlDeviceGetUUID");
        char uuid[NVML_DEVICE_UUID_BUFFER_SIZE];
        rv = _nvmlDeviceGetUUID(device, uuid, sizeof(uuid));
        if_nvml_success(rv, [&]() { gpu.uuid = uuid; });
      }

      /* VBIOS */
      {
        auto const _nvmlDeviceGetVbiosVersion =
            load_symbol<nvmlReturn_t(nvmlDevice_t, char *, unsigned int)>(
                "nvmlDeviceGetVbiosVersion");
        char vbios[NVML_DEVICE_VBIOS_VERSION_BUFFER_SIZE];
        rv = _nvmlDeviceGetVbiosVersion(device, vbios, sizeof(vbios));
        if_nvml_success(rv, [&]() { gpu.vbios = vbios; });
      }

      data.devices.push_back(gpu);
    }
#endif

#ifdef WITH_DATA_NVML
    nvml::impl::nvlink nvml::impl::nvlink_query(nvmlDevice_t device,
                                                unsigned int link) {
      nvmlReturn_t rv;
      nvml::impl::nvlink nvl;

      /* State */
      {
        auto const _nvmlDeviceGetNvLinkState = load_symbol<nvmlReturn_t(
            nvmlDevice_t, unsigned int, nvmlEnableState_t *)>(
            "nvmlDeviceGetNvLinkState");
        nvmlEnableState_t state;
        rv = _nvmlDeviceGetNvLinkState(device, link, &state);
        if_nvml_success(rv,
                        [&]() { nvl.active = state == NVML_FEATURE_ENABLED; });
      }

      /* Version */
      {
        auto const _nvmlDeviceGetNvLinkVersion = load_symbol<nvmlReturn_t(
            nvmlDevice_t, unsigned int, unsigned int *)>(
            "nvmlDeviceGetNvLinkVersion");
        unsigned int version;
        rv = _nvmlDeviceGetNvLinkVersion(device, link, &version);
        if_nvml_success(rv, [&]() { nvl.version = version; });
      }

      return nvl;
    }
#endif

#ifdef WITH_DATA_NVML
    void nvml::impl::system_query() {
      nvmlReturn_t rv;

      /* CUDA driver version */
      {
        auto const _nvmlSystemGetCudaDriverVersion =
            load_symbol<nvmlReturn_t(int *)>("nvmlSystemGetCudaDriverVersion");
        int cuda_driver_version;
        rv = _nvmlSystemGetCudaDriverVersion(&cuda_driver_version);
        if_nvml_success(
            rv, [&]() { data.cuda_driver_version = cuda_driver_version; });
      }

      /* Driver version */
      {
        auto const _nvmlSystemGetDriverVersion =
            load_symbol<nvmlReturn_t(char *, unsigned int)>(
                "nvmlSystemGetDriverVersion");
        char driver_version[NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE];
        rv =
            _nvmlSystemGetDriverVersion(driver_version, sizeof(driver_version));
        if_nvml_success(rv, [&]() { data.driver_version = driver_version; });
      }

      /* NVML version */
      {
        auto const _nvmlSystemGetNVMLVersion =
            load_symbol<nvmlReturn_t(char *, unsigned int)>(
                "nvmlSystemGetNVMLVersion");
        char nvml_version[NVML_SYSTEM_NVML_VERSION_BUFFER_SIZE];
        rv = _nvmlSystemGetNVMLVersion(nvml_version, sizeof(nvml_version));
        if_nvml_success(rv, [&]() { data.nvml_version = nvml_version; });
      }

      return;
    }
#endif
    /* \endcond */

    void from_json(const json &j, nvml &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("version", 0) != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      if (j.contains("data")) {
        d.pimpl->collected = true;
      }

      d.pimpl->data.cuda_driver_version =
          j.value(json::json_pointer("/data/cuda_driver_version"), 0);
      d.pimpl->data.driver_version =
          j.value(json::json_pointer("/data/driver_version"), "");
      d.pimpl->data.nvml_version =
          j.value(json::json_pointer("/data/nvml_version"), "");

      for (auto i :
           j.value(json::json_pointer("/data/devices"), json::array())) {
        nvml::impl::gpu gpu;

        gpu.bar1.free = i.value(json::json_pointer("/bar1/free"), 0ULL);
        gpu.bar1.total = i.value(json::json_pointer("/bar1/total"), 0ULL);
        gpu.bar1.used = i.value(json::json_pointer("/bar1/used"), 0ULL);

        gpu.board_part_number = i.value("board_part_number", "");
        gpu.brand = i.value("brand", 0);

        gpu.clock.current.graphics =
            i.value(json::json_pointer("/clock/current/graphics"), 0);
        gpu.clock.current.memory =
            i.value(json::json_pointer("/clock/current/memory"), 0);
        gpu.clock.current.sm =
            i.value(json::json_pointer("/clock/current/sm"), 0);

        gpu.clock.default_.graphics =
            i.value(json::json_pointer("/clock/default/graphics"), 0);
        gpu.clock.default_.memory =
            i.value(json::json_pointer("/clock/default/memory"), 0);
        gpu.clock.default_.sm =
            i.value(json::json_pointer("/clock/default/sm"), 0);

        gpu.clock.target.graphics =
            i.value(json::json_pointer("/clock/target/graphics"), 0);
        gpu.clock.target.memory =
            i.value(json::json_pointer("/clock/target/memory"), 0);
        gpu.clock.target.sm =
            i.value(json::json_pointer("/clock/target/sm"), 0);

        gpu.compute_mode = i.value("compute_mode", 0);

        gpu.cuda_compute_capability.major =
            i.value(json::json_pointer("/cuda_compute_capability/major"), 0);
        gpu.cuda_compute_capability.minor =
            i.value(json::json_pointer("/cuda_compute_capability/minor"), 0);

        gpu.ecc.current = i.value(json::json_pointer("/ecc/current"), false);
        gpu.ecc.errors.aggregate.corrected = i.value(
            json::json_pointer("/ecc/errors/aggregate/corrected"), 0ULL);
        gpu.ecc.errors.aggregate.uncorrected = i.value(
            json::json_pointer("/ecc/errors/aggregate/uncorrected"), 0ULL);
        gpu.ecc.errors.volatile_.corrected =
            i.value(json::json_pointer("/ecc/errors/volatile/corrected"), 0ULL);
        gpu.ecc.errors.volatile_.uncorrected = i.value(
            json::json_pointer("/ecc/errors/volatile/uncorrected"), 0ULL);
        gpu.ecc.pending = i.value(json::json_pointer("/ecc/pending"), false);

        gpu.index = i.value("/index", 0);

        gpu.inforom.checksum =
            i.value(json::json_pointer("/inforom/checksum"), 0);
        gpu.inforom.ecc_version =
            i.value(json::json_pointer("/inforom/ecc_version"), "");
        gpu.inforom.image_version =
            i.value(json::json_pointer("/inforom/image_version"), "");
        gpu.inforom.oem_version =
            i.value(json::json_pointer("/inforom/oem_version"), "");
        gpu.inforom.power_version =
            i.value(json::json_pointer("/inforom/power_version"), "");

        gpu.memory.free = i.value(json::json_pointer("/memory/free"), 0ULL);
        gpu.memory.total = i.value(json::json_pointer("/memory/total"), 0ULL);
        gpu.memory.used = i.value(json::json_pointer("/memory/used"), 0ULL);

        gpu.minor_number = i.value("minor_number", 0);

        for (auto l : i.value("nvlinks", json::array())) {
          nvml::impl::nvlink nvlink;

          nvlink.active = l.value("active", false);
          nvlink.version = l.value("version", 0);

          gpu.nvlinks.push_back(nvlink);
        }

        gpu.pcie.bus = i.value(json::json_pointer("/pcie/bus"), 0);
        gpu.pcie.bus_id = i.value(json::json_pointer("/pcie/bus_id"), "");
        gpu.pcie.device = i.value(json::json_pointer("/pcie/device"), 0);
        gpu.pcie.device_id = i.value(json::json_pointer("/pcie/device_id"), 0);
        gpu.pcie.domain = i.value(json::json_pointer("/pcie/domain"), 0);
        gpu.pcie.generation =
            i.value(json::json_pointer("/pcie/generation"), 0);
        gpu.pcie.subsystem_id =
            i.value(json::json_pointer("/pcie/subsystem_id"), 0);
        gpu.pcie.width = i.value(json::json_pointer("/pcie/width"), 0);

        gpu.name = i.value("name", "");
        gpu.pstate = i.value("pstate", 32);

        gpu.retired_pages.double_bit =
            i.value(json::json_pointer("/retired_pages/double_bit"), 0);
        gpu.retired_pages.pending =
            i.value(json::json_pointer("/retired_pages/pending"), false);
        gpu.retired_pages.single_bit =
            i.value(json::json_pointer("/retired_pages/single_bit"), 0);

        gpu.serial = i.value("serial", "");
        gpu.temperature = i.value("temperature", 0);
        gpu.uuid = i.value("uuid", "");
        gpu.vbios = i.value("vbios", "");

        d.pimpl->data.devices.push_back(gpu);
      }
    }

    void to_json(json &j, const nvml &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["cuda_driver_version"] = d.pimpl->data.cuda_driver_version;
      j["data"]["driver_version"] = d.pimpl->data.driver_version;
      j["data"]["nvml_version"] = d.pimpl->data.nvml_version;

      j["data"]["devices"] = json::array();

      for (auto i : d.pimpl->data.devices) {
        json gpu;

        gpu["bar1"]["free"] = i.bar1.free;
        gpu["bar1"]["total"] = i.bar1.total;
        gpu["bar1"]["used"] = i.bar1.used;

        gpu["board_part_number"] = i.board_part_number;
        gpu["brand"] = i.brand;

        gpu["clock"]["current"]["graphics"] = i.clock.current.graphics;
        gpu["clock"]["current"]["memory"] = i.clock.current.memory;
        gpu["clock"]["current"]["sm"] = i.clock.current.sm;

        gpu["clock"]["default"]["graphics"] = i.clock.default_.graphics;
        gpu["clock"]["default"]["memory"] = i.clock.default_.memory;
        gpu["clock"]["default"]["sm"] = i.clock.default_.sm;

        gpu["clock"]["target"]["graphics"] = i.clock.target.graphics;
        gpu["clock"]["target"]["memory"] = i.clock.target.memory;
        gpu["clock"]["target"]["sm"] = i.clock.target.sm;

        gpu["compute_mode"] = i.compute_mode;

        gpu["cuda_compute_capability"]["major"] =
            i.cuda_compute_capability.major;
        gpu["cuda_compute_capability"]["minor"] =
            i.cuda_compute_capability.minor;

        gpu["ecc"]["current"] = i.ecc.current;
        gpu["ecc"]["errors"]["aggregate"]["corrected"] =
            i.ecc.errors.aggregate.corrected;
        gpu["ecc"]["errors"]["aggregate"]["uncorrected"] =
            i.ecc.errors.aggregate.uncorrected;
        gpu["ecc"]["errors"]["volatile"]["corrected"] =
            i.ecc.errors.volatile_.corrected;
        gpu["ecc"]["errors"]["volatile"]["uncorrected"] =
            i.ecc.errors.volatile_.uncorrected;
        gpu["ecc"]["pending"] = i.ecc.pending;

        gpu["inforom"]["checksum"] = i.inforom.checksum;
        gpu["inforom"]["ecc_version"] = i.inforom.ecc_version;
        gpu["inforom"]["image_version"] = i.inforom.image_version;
        gpu["inforom"]["oem_version"] = i.inforom.oem_version;
        gpu["inforom"]["power_version"] = i.inforom.power_version;

        gpu["index"] = i.index;

        gpu["memory"]["free"] = i.memory.free;
        gpu["memory"]["total"] = i.memory.total;
        gpu["memory"]["used"] = i.memory.used;

        gpu["minor_number"] = i.minor_number;

        gpu["nvlinks"] = json::array();
        for (auto l : i.nvlinks) {
          json nvlink;

          nvlink["active"] = l.active;
          nvlink["version"] = l.version;

          gpu["nvlinks"].push_back(nvlink);
        }

        gpu["pcie"]["bus"] = i.pcie.bus;
        gpu["pcie"]["bus_id"] = i.pcie.bus_id;
        gpu["pcie"]["device"] = i.pcie.device;
        gpu["pcie"]["device_id"] = i.pcie.device_id;
        gpu["pcie"]["domain"] = i.pcie.domain;
        gpu["pcie"]["generation"] = i.pcie.generation;
        gpu["pcie"]["subsystem_id"] = i.pcie.subsystem_id;
        gpu["pcie"]["width"] = i.pcie.width;

        gpu["name"] = i.name;
        gpu["pstate"] = i.pstate;

        gpu["retired_pages"]["double_bit"] = i.retired_pages.double_bit;
        gpu["retired_pages"]["pending"] = i.retired_pages.pending;
        gpu["retired_pages"]["single_bit"] = i.retired_pages.single_bit;

        gpu["serial"] = i.serial;
        gpu["temperature"] = i.temperature;
        gpu["uuid"] = i.uuid;
        gpu["vbios"] = i.vbios;

        j["data"]["devices"].push_back(gpu);
      }

      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
