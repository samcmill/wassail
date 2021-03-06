/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <limits>
#include <list>
#include <shared_mutex>
#include <wassail/data/getfsstat.hpp>
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_UCRED_H
#include <sys/ucred.h>
#endif

#ifndef MNT_NOWAIT
#define MNT_NOWAIT 2
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class getfsstat::impl {
    public:
      /*! \brief Mounted filesystems */
      struct fs_item {
        long bsize;              /*!< fundamental file system block size */
        long blocks;             /*!< total data blocks in file system */
        long bfree;              /*!< free blocks in fs */
        long bavail;             /*!< free blocks avail to non-superuser */
        long files;              /*!< total file nodes in file system */
        long ffree;              /*!< free file nodes in fs */
        uid_t owner;             /*!< user that mounted the file system */
        long flags;              /*!< copy of mount flags */
        std::string fstypename;  /*!< fs type name */
        std::string mntonname;   /*!< directory on which mounted */
        std::string mntfromname; /*!< mounted file system */
      };

      struct {
        std::list<fs_item> file_systems;
      } data; /*!< Mounted filesystems */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::getfsstat::evaluate() */
      void evaluate(getfsstat &d, bool force);

    private:
      int flags = MNT_NOWAIT; /*!< getfsstat() flag, return immediately */
    };

    getfsstat::getfsstat() : pimpl{std::make_unique<impl>()} {}
    getfsstat::~getfsstat() = default;
    getfsstat::getfsstat(getfsstat &&) = default;            // LCOV_EXCL_LINE
    getfsstat &getfsstat::operator=(getfsstat &&) = default; // LCOV_EXCL_LINE

    bool getfsstat::enabled() const {
#ifdef HAVE_GETFSSTAT
      return true;
#else
      return false;
#endif
    }

    void getfsstat::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void getfsstat::impl::evaluate(getfsstat &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not d.collected()) {
#ifdef HAVE_GETFSSTAT
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        auto numfs = ::getfsstat(NULL, 0, flags);

        auto buf_size = sizeof(struct statfs) * numfs;
        /* LCOV_EXCL_START */
        if (buf_size == -1) {
          wassail::internal::logger()->warn("Error calling getfsstat()");
          return;
        }
        /* LCOV_EXCL_STOP */

        struct statfs *buf = (struct statfs *)malloc(buf_size);

        auto ret = ::getfsstat(buf, buf_size, flags);
        /* LCOV_EXCL_START */
        if (ret == -1) {
          wassail::internal::logger()->warn("Error calling getfsstat()");
          free(buf);
          return;
        }
        /* LCOV_EXCL_STOP */

        for (int i = 0; i < numfs; i++) {
          fs_item item;

          item.bsize = buf[i].f_bsize;
          item.blocks = buf[i].f_blocks;
          item.bfree = buf[i].f_bfree;
          item.bavail = buf[i].f_bavail;
          item.files = buf[i].f_files;
          item.ffree = buf[i].f_ffree;
          item.owner = buf[i].f_owner;
          item.flags = buf[i].f_flags;
          item.fstypename = buf[i].f_fstypename;
          item.mntonname = buf[i].f_mntonname;
          item.mntfromname = buf[i].f_mntfromname;

          data.file_systems.push_back(item);
        }

        free(buf);

        d.common::evaluate_common();
#else
        throw std::runtime_error("getfsstat() not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, getfsstat &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("name", "") != d.name()) {
        throw std::runtime_error("name mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      for (auto i :
           j.value(json::json_pointer("/data/file_systems"), json::array())) {
        getfsstat::impl::fs_item item;

        item.bavail = i.value("bavail", 0L);
        item.bfree = i.value("bfree", 0L);
        item.blocks = i.value("blocks", 0L);
        item.bsize = i.value("bsize", 0L);
        item.ffree = i.value("ffree", 0L);
        item.files = i.value("files", 0L);
        item.flags = i.value("flags", 0L);
        item.fstypename = i.value("fstypename", "");
        item.mntfromname = i.value("mntfromname", "");
        item.mntonname = i.value("mntonname", "");
        item.owner = i.value(
            "owner", static_cast<uid_t>(std::numeric_limits<uid_t>::max()));

        d.pimpl->data.file_systems.push_back(item);
      }
    }

    void to_json(json &j, const getfsstat &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["file_systems"] = json::array();

      for (auto i : d.pimpl->data.file_systems) {
        json temp;

        temp["bavail"] = i.bavail;
        temp["bfree"] = i.bfree;
        temp["blocks"] = i.blocks;
        temp["bsize"] = i.bsize;
        temp["ffree"] = i.ffree;
        temp["files"] = i.files;
        temp["flags"] = i.flags;
        temp["fstypename"] = i.fstypename;
        temp["mntonname"] = i.mntonname;
        temp["mntfromname"] = i.mntfromname;
        temp["owner"] = i.owner;

        j["data"]["file_systems"].push_back(temp);
      }

      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
