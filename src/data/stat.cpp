/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <cstdlib>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <time.h>
#include <wassail/data/stat.hpp>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#else
typedef int32_t dev_t;
typedef uint16_t mode_t;
typedef uint16_t nlink_t;
typedef uint64_t ino_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef int64_t off_t;
typedef int64_t blkcnt_t;
typedef int32_t blksize_t;
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class stat::impl {
    public:
      /*! \brief File information data */
      struct {
        std::string path;  /*!< fileystem path */
        dev_t dev;         /*!< ID of device containing file */
        mode_t mode;       /*!< protection mode of file */
        ino_t ino;         /*!< file serial number */
        nlink_t nlink;     /*!< number of hard links */
        uid_t uid;         /*<! user ID of file */
        gid_t gid;         /*<! group ID of file */
        dev_t rdev;        /*<! device ID */
        double atime;      /*<! time of last access */
        double mtime;      /*<! time of last modification */
        double ctime;      /*<! time of last file status change */
        off_t size;        /*<! file size, in bytes */
        blkcnt_t blocks;   /*<! blocks allocated for file */
        blksize_t blksize; /*<! blocksize for file system I/O */
      } data;              /*!< file information data */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::stat::evaluate() */
      void evaluate(stat &d, bool force);
    };

    stat::stat() : pimpl{std::make_unique<impl>()} {}
    stat::stat(std::string _path) : pimpl{std::make_unique<impl>()} {
      path = _path;
    }

    stat::~stat() = default;
    stat::stat(stat &&) = default;            // LCOV_EXCL_LINE
    stat &stat::operator=(stat &&) = default; // LCOV_EXCL_LINE

    bool stat::enabled() const {
#ifdef WITH_DATA_STAT
      return true;
#else
      return false;
#endif
    }

    void stat::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void stat::impl::evaluate(stat &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not d.collected()) {
#ifdef WITH_DATA_STAT
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        /* collect data */
        int rv;
        struct ::stat s;

        rv = ::stat(d.path.c_str(), &s);
        if (rv == 0) {
          data.path = d.path;
          data.dev = s.st_dev;
          data.mode = s.st_mode;
          data.nlink = s.st_nlink;
          data.ino = s.st_ino;
          data.uid = s.st_uid;
          data.gid = s.st_gid;
          data.rdev = s.st_rdev;
          data.size = s.st_size;
          data.blocks = s.st_blocks;
          data.blksize = s.st_blksize;

#ifdef HAVE_STRUCT_STAT_ST_ATIMESPEC
          data.atime = (double)s.st_atimespec.tv_sec +
                       (double)s.st_atimespec.tv_nsec / 1e9;
#else
          data.atime = static_cast<double>(s.st_atime);
#endif

#ifdef HAVE_STRUCT_STAT_ST_ATIMESPEC
          data.ctime = (double)s.st_ctimespec.tv_sec +
                       (double)s.st_ctimespec.tv_nsec / 1e9;
#else
          data.ctime = static_cast<double>(s.st_ctime);
#endif

#ifdef HAVE_STRUCT_STAT_ST_MTIMESPEC
          data.mtime = (double)s.st_mtimespec.tv_sec +
                       (double)s.st_mtimespec.tv_nsec / 1e9;
#else
          data.mtime = static_cast<double>(s.st_mtime);
#endif

          d.common::evaluate_common();
        }
#else
        throw std::runtime_error("stat() is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, stat &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("name", "") != d.name()) {
        throw std::runtime_error("name mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      d.path = j.value(json::json_pointer("/configuration/path"), "");
      if (d.path == "") {
        wassail::internal::logger()->warn("No path specified");
      }

      d.pimpl->data.path = j.value(json::json_pointer("/data/path"), "");
      d.pimpl->data.dev =
          j.value(json::json_pointer("/data/device"), static_cast<dev_t>(0));
      d.pimpl->data.mode =
          j.value(json::json_pointer("/data/mode"), static_cast<mode_t>(0));
      d.pimpl->data.nlink =
          j.value(json::json_pointer("/data/nlink"), static_cast<nlink_t>(0));
      d.pimpl->data.ino =
          j.value(json::json_pointer("/data/inode"), static_cast<ino_t>(0));
      d.pimpl->data.uid =
          j.value(json::json_pointer("/data/uid"), static_cast<uid_t>(0));
      d.pimpl->data.gid =
          j.value(json::json_pointer("/data/gid"), static_cast<gid_t>(0));
      d.pimpl->data.rdev =
          j.value(json::json_pointer("/data/rdev"), static_cast<dev_t>(0));
      d.pimpl->data.atime = j.value(json::json_pointer("/data/atime"), 0.0);
      d.pimpl->data.mtime = j.value(json::json_pointer("/data/mtime"), 0.0);
      d.pimpl->data.ctime = j.value(json::json_pointer("/data/ctime"), 0.0);
      d.pimpl->data.size =
          j.value(json::json_pointer("/data/size"), static_cast<off_t>(0));
      d.pimpl->data.blocks =
          j.value(json::json_pointer("/data/blocks"), static_cast<blkcnt_t>(0));
      d.pimpl->data.blksize = j.value(json::json_pointer("/data/blksize"),
                                      static_cast<blksize_t>(0));
    }

    void to_json(json &j, const stat &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["configuration"]["path"] = d.path;

      j["data"]["path"] = d.pimpl->data.path;
      j["data"]["device"] = d.pimpl->data.dev;
      j["data"]["mode"] = d.pimpl->data.mode;
      j["data"]["nlink"] = d.pimpl->data.nlink;
      j["data"]["inode"] = d.pimpl->data.ino;
      j["data"]["uid"] = d.pimpl->data.uid;
      j["data"]["gid"] = d.pimpl->data.gid;
      j["data"]["rdev"] = d.pimpl->data.rdev;
      j["data"]["atime"] = d.pimpl->data.atime;
      j["data"]["mtime"] = d.pimpl->data.mtime;
      j["data"]["ctime"] = d.pimpl->data.ctime;
      j["data"]["size"] = d.pimpl->data.size;
      j["data"]["blocks"] = d.pimpl->data.blocks;
      j["data"]["blksize"] = d.pimpl->data.blksize;
      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
