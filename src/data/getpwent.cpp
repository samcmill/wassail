/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <cerrno>
#include <cstring>
#include <list>
#include <memory>
#include <shared_mutex>
#include <string>
#include <wassail/data/getpwent.hpp>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class getpwent::impl {
    public:
      /*! \brief passwd entry */
      struct pw_item {
        std::string pw_name;  /*!< user name */
        uid_t pw_uid;         /*!< user uid */
        gid_t pw_gid;         /*!< user gid */
        std::string pw_dir;   /*!< home directory */
        std::string pw_shell; /*!< default shell */
      };

      struct {
        std::list<pw_item> users;
      } data; /*!< User data */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::getpwent::evaluate() */
      void evaluate(getpwent &d, bool force);
    };

    getpwent::getpwent() : pimpl{std::make_unique<impl>()} {}
    getpwent::~getpwent() = default;
    getpwent::getpwent(getpwent &&) = default;            // LCOV_EXCL_LINE
    getpwent &getpwent::operator=(getpwent &&) = default; // LCOV_EXCL_LINE

    bool getpwent::enabled() const {
#ifdef WITH_DATA_GETPWENT
      return true;
#else
      return false;
#endif
    }

    void getpwent::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void getpwent::impl::evaluate(getpwent &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (force or not d.collected()) {
#ifdef WITH_DATA_GETPWENT
        std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

        setpwent(); // Rewind the database stream

        errno = 0;
        struct passwd *pw;

        while ((pw = ::getpwent()) != NULL) {
          if (errno != 0) {
            wassail::internal::logger()->error("getpwent() failed: {}",
                                               std::strerror(errno));
            break;
          }

          pw_item item;

          item.pw_name = pw->pw_name;
          item.pw_uid = pw->pw_uid;
          item.pw_gid = pw->pw_gid;
          item.pw_dir = pw->pw_dir;
          item.pw_shell = pw->pw_shell;

          data.users.push_back(item);

          errno = 0;
        }

        endpwent(); // Close the database stream

        d.common::evaluate_common();
#else
        throw std::runtime_error("getpwent data source is not available");
#endif
      }
    }
    /* \endcond */

    void from_json(const json &j, getpwent &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.at("name").get<std::string>() != d.name()) {
        throw std::runtime_error("name mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      for (auto i : j.value(json::json_pointer("/data/users"), json::array())) {
        getpwent::impl::pw_item item;

        item.pw_name = i.value("pw_name", "");
        item.pw_uid = i.value("pw_uid", static_cast<uid_t>(0));
        item.pw_gid = i.value("pw_gid", static_cast<gid_t>(0));
        item.pw_dir = i.value("pw_dir", "");
        item.pw_shell = i.value("pw_shell", "");

        d.pimpl->data.users.push_back(item);
      }
    }

    void to_json(json &j, const getpwent &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"]["users"] = json::array();

      for (auto i : d.pimpl->data.users) {
        json temp;

        temp["pw_name"] = i.pw_name;
        temp["pw_uid"] = i.pw_uid;
        temp["pw_gid"] = i.pw_gid;
        temp["pw_dir"] = i.pw_dir;
        temp["pw_shell"] = i.pw_shell;

        j["data"]["users"].push_back(temp);
      }

      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
