/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include <chrono>
#include <cstdlib>
#include <memory>
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
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

      /*! \brief File information data */
      struct {
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
      if (force or not collected) {
#ifdef WITH_DATA_STAT
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        /* collect data */
        int rv;
        struct ::stat s;

        rv = ::stat(d.path.c_str(), &s);
        if (rv == 0) {
          d.timestamp = std::chrono::system_clock::now();
          collected = true;
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
        }
#else
        throw std::runtime_error("stat() is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, stat &d) {
      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      try {
        auto jdata = j.at("data");
        d.path = jdata.at("path").get<std::string>();
        d.pimpl->data.dev = jdata.at("device").get<dev_t>();
        d.pimpl->data.mode = jdata.at("mode").get<mode_t>();
        d.pimpl->data.nlink = jdata.at("nlink").get<nlink_t>();
        d.pimpl->data.ino = jdata.at("inode").get<ino_t>();
        d.pimpl->data.uid = jdata.at("uid").get<uid_t>();
        d.pimpl->data.gid = jdata.at("gid").get<gid_t>();
        d.pimpl->data.rdev = jdata.at("rdev").get<dev_t>();
        d.pimpl->data.atime = jdata.at("atime").get<double>();
        d.pimpl->data.mtime = jdata.at("mtime").get<double>();
        d.pimpl->data.ctime = jdata.at("ctime").get<double>();
        d.pimpl->data.size = jdata.at("size").get<off_t>();
        d.pimpl->data.blocks = jdata.at("blocks").get<blkcnt_t>();
        d.pimpl->data.blksize = jdata.at("blksize").get<blksize_t>();
      }
      catch (std::exception &e) {
        throw std::runtime_error(
            std::string("Unable to convert JSON string '") + j.dump() +
            std::string("' to object: ") + e.what());
      }
    }

    void to_json(json &j, const stat &d) {
      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["path"] = d.path;
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
