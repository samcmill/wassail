/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <cstdio>
#include <list>
#include <string>
#include <wassail/data/getmntent.hpp>
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#else
typedef long long blkcnt_t;
typedef long long fsfilcnt_t;
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class getmntent::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the load
                                average has been collected */

      /*! \brief Mounted filesystems */
      struct fs_item {
        unsigned long bsize;  /*!< file system block size */
        unsigned long frsize; /*!< fragment size */
        blkcnt_t blocks;      /*!< size of fs in frsize units */
        blkcnt_t bfree;       /*!< # free blocks */
        blkcnt_t bavail;      /*!< # free blocks for unprivileged users */
        fsfilcnt_t files;     /*!< # inodes */
        fsfilcnt_t ffree;     /*!< # free inodes */
        fsfilcnt_t favail;    /*!< # free inodes for unprivileged users */
        unsigned long fsid;   /*!< file system ID */
        unsigned long flag;   /*!< mount flags */
        std::string fsname;   /*!< name of mounted file system */
        std::string dir;      /*!< file system path prefix */
        std::string type;     /*!< mount type */
      };

      struct {
        std::list<fs_item> file_systems;
      } data; /*!< Mounted filesystems */

      /*! Private implementation of wassail::data::getmntent::evaluate() */
      void evaluate(getmntent &d, bool force);

    private:
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
      const char *mtab = "/etc/mtab"; /*!< mtab file location */
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    };

    getmntent::getmntent() : pimpl{std::make_unique<impl>()} {}
    getmntent::~getmntent() = default;
    getmntent::getmntent(getmntent &&) = default;            // LCOV_EXCL_LINE
    getmntent &getmntent::operator=(getmntent &&) = default; // LCOV_EXCL_LINE

    bool getmntent::enabled() const {
#ifdef HAVE_GETMNTENT
      return true;
#else
      return false;
#endif
    }

    void getmntent::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void getmntent::impl::evaluate(getmntent &d, bool force) {
      if (force or not collected) {
#ifdef HAVE_GETMNTENT
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        FILE *fp = setmntent(mtab, "r");
        if (fp == NULL) {
          wassail::internal::logger()->warn("Unable to open {}", mtab);
          return;
        }

        struct mntent *mnt;
        while ((mnt = ::getmntent(fp)) != NULL) {
          if (mnt->mnt_dir != NULL) {
            struct statvfs vfs;
            int rv = statvfs(mnt->mnt_dir, &vfs);
            if (rv == 0) {
              fs_item item;

              item.bsize = vfs.f_bsize;
              item.frsize = vfs.f_frsize;
              item.blocks = vfs.f_blocks;
              item.bavail = vfs.f_bavail;
              item.bfree = vfs.f_bfree;
              item.files = vfs.f_files;
              item.ffree = vfs.f_ffree;
              item.favail = vfs.f_favail;
              item.fsid = vfs.f_fsid;
              item.flag = vfs.f_flag;
              item.fsname = mnt->mnt_fsname;
              item.dir = mnt->mnt_dir;
              item.type = mnt->mnt_type;

              data.file_systems.push_back(item);
            }
          }
        }

        endmntent(fp);

        d.timestamp = std::chrono::system_clock::now();
        collected = true;
#else
        throw std::runtime_error("getmntent data source is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, getmntent &d) {
      if (j.at("version").get<uint16_t>() != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));
      d.pimpl->collected = true;

      try {
        for (auto i : j.at("data").at("file_systems")) {
          getmntent::impl::fs_item item;
          item.bavail = i.at("bavail").get<blkcnt_t>();
          item.bfree = i.at("bfree").get<blkcnt_t>();
          item.blocks = i.at("blocks").get<blkcnt_t>();
          item.bsize = i.at("bsize").get<unsigned long>();
          item.dir = i.at("dir").get<std::string>();
          item.favail = i.at("favail").get<fsfilcnt_t>();
          item.ffree = i.at("ffree").get<fsfilcnt_t>();
          item.files = i.at("files").get<fsfilcnt_t>();
          item.flag = i.at("flag").get<unsigned long>();
          item.frsize = i.at("frsize").get<unsigned long>();
          item.fsid = i.at("fsid").get<unsigned long>();
          item.fsname = i.at("fsname").get<std::string>();
          item.type = i.at("type").get<std::string>();
          d.pimpl->data.file_systems.push_back(item);
        }
      }
      catch (std::exception &e) {
        throw std::runtime_error(
            std::string("Unable to convert JSON string '") + j.dump() +
            std::string("' to object: ") + e.what());
      }
    }

    void to_json(json &j, const getmntent &d) {
      j = dynamic_cast<const wassail::data::common &>(d);

      for (auto i : d.pimpl->data.file_systems) {
        json temp;

        temp["bsize"] = i.bsize;
        temp["frsize"] = i.frsize;
        temp["blocks"] = i.blocks;
        temp["bfree"] = i.bfree;
        temp["bavail"] = i.bavail;
        temp["files"] = i.files;
        temp["ffree"] = i.ffree;
        temp["favail"] = i.favail;
        temp["fsid"] = i.fsid;
        temp["flag"] = i.flag;
        temp["fsname"] = i.fsname;
        temp["dir"] = i.dir;
        temp["type"] = i.type;

        j["data"]["file_systems"].push_back(temp);
      }

      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
