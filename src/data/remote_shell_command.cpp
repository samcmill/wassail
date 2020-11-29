/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <unistd.h>
#include <wassail/data/remote_shell_command.hpp>
#ifdef HAVE_LIBSSH_LIBSSHPP_HPP
#include <libssh/libsshpp.hpp>
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class remote_shell_command::impl {
    public:
      bool collected = false; /*!< Flag to denote whether the data
                                   has been collected */

      /*! \brief Remote shell command output
       *
       *  This is an intentional duplicate of wassail::data::shell_command.
       */
      struct item {
        struct shell {
          std::string command = ""; /*!< Shell command */
          double elapsed = 0.0;     /*!< Number of seconds the command
                                         took to execute */
          int returncode = 255;     /*!< Shell exit status */
          std::string stderr = "";  /*!< Standard error */
          std::string stdout = "";  /*!< Standard output */
        } shell;
        std::string hostname = ""; /*!< Remote host */
        std::chrono::time_point<std::chrono::system_clock>
            timestamp; /*!< Timestamp when the remote shell was invoked */
        uid_t uid;     /*!< User ID when the remote shell was invoked */
      };

      std::list<item> data; /*!< Remote shell command data */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of
       * wassail::data::remote_shell_command::evaluate() */
      void evaluate(remote_shell_command &d, bool force);

    private:
      std::mutex data_mutex;

      /*! Execute the shell command on a remote host, with separate
       *  streams for standard output and standard error.  Populates
       *  the data struct.
       */
      void ssh(std::string host, remote_shell_command &d);
    };

    remote_shell_command::remote_shell_command()
        : pimpl{std::make_unique<impl>()} {}
    remote_shell_command::remote_shell_command(std::string _host,
                                               std::string _command)
        : pimpl{std::make_unique<impl>()} {
      hosts = {_host};
      command = _command;
    }

    remote_shell_command::remote_shell_command(std::list<std::string> _hosts,
                                               std::string _command)
        : pimpl{std::make_unique<impl>()} {
      hosts = _hosts;
      command = _command;
    }

    remote_shell_command::remote_shell_command(std::list<std::string> _hosts,
                                               std::string _command,
                                               uint8_t _timeout)
        : pimpl{std::make_unique<impl>()} {
      hosts = _hosts;
      command = _command;
      timeout = _timeout;
    }

    remote_shell_command::~remote_shell_command() = default;
    remote_shell_command::remote_shell_command(remote_shell_command &&) =
        default;
    remote_shell_command &remote_shell_command::
    operator=(remote_shell_command &&) = default;

    bool remote_shell_command::enabled() const {
#ifdef WITH_DATA_REMOTE_SHELL_COMMAND
      return true;
#else
      return false;
#endif
    }

    void remote_shell_command::evaluate(bool force) {
      pimpl->evaluate(*this, force);
    }

    void remote_shell_command::impl::evaluate(remote_shell_command &d,
                                              bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

#ifdef WITH_DATA_REMOTE_SHELL_COMMAND
      std::shared_lock<std::shared_timed_mutex> lock(d.mutex);

      if (d.command.empty()) {
        throw std::runtime_error("Missing command");
      }

      if (d.hosts.size() == 0) {
        throw std::runtime_error("Empty host list");
      }

      if (force or not collected) {
        d.common::evaluate(force);

        /* libssh initialization may not be thread safe */
        ssh_init();

        wassail::internal::for_each(
            d.hosts.begin(), d.hosts.end(),
            [&d, this](std::string host) { ssh(host, d); });

        /* libssh cleanup may not be thread safe */
        ssh_finalize();

        collected = true;
      }
#else
      throw std::runtime_error(
          "remote_shell_command data source is not available");
#endif
    }

    void remote_shell_command::impl::ssh(std::string host,
                                         remote_shell_command &d) {
#ifdef WITH_DATA_REMOTE_SHELL_COMMAND
      item node;
      node.hostname = host;
      node.timestamp = std::chrono::system_clock::now();
      node.uid = getuid(); /* assume remote uid is the same */
      node.shell.command = d.command;

      /* collect data */
      try {
        auto session = ssh::Session();

        session.setOption(SSH_OPTIONS_HOST, host.c_str());
        session.setOption(SSH_OPTIONS_PORT, &d.port);

        session.connect();

        int rc = session.userauthPublickeyAuto();
        if (rc != SSH_AUTH_SUCCESS) {
          node.shell.stderr.append("authentication failure");
          std::lock_guard<std::mutex> lock(data_mutex);
          d.pimpl->data.push_back(node);
          return;
        }

        auto channel = ssh::Channel(session);
        channel.openSession();

        // non-blocking
        channel.requestExec(d.command.c_str());

        auto start = std::chrono::steady_clock::now();
        auto current = start;
        std::chrono::duration<double> elapsed = current - start;

        do {
          current = std::chrono::steady_clock::now();
          elapsed = current - start;
          int remaining =
              d.timeout -
              std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
          if (remaining < 0) {
            remaining = 0;
          }

          // read stdout
          if (channel.poll() > 0) {
            char buf[4096] = {0};
            int nbytes;
            while ((nbytes = channel.read(buf, sizeof(buf), false, 0))) {
              if (nbytes > 0) {
                node.shell.stdout.append(buf, nbytes);
              }
            }
          }

          // read stderr
          if (channel.poll(true) > 0) {
            char buf[4096] = {0};
            int nbytes;
            while ((nbytes = channel.read(buf, sizeof(buf), true, 0))) {
              if (nbytes > 0) {
                node.shell.stderr.append(buf, nbytes);
              }
            }
          }

          if (channel.isEof()) {
            break;
          }
        } while (elapsed.count() < d.timeout);

        node.shell.elapsed =
            std::chrono::duration<double>(current - start).count();

        if (elapsed.count() > d.timeout) {
          /* Sending signals does not work reliably.  According to
           * https://bugzilla.mindrot.org/show_bug.cgi?id=1424 OpenSSH version
           * 7.9 and later support signal handling.  Otherwise, this
           * will not actually kill the remote process and instead it will
           * block on channel.sendEof() until the remote process
           * completes normally. */
          wassail::internal::logger()->warn(
              "Shell command exceeded allowed time, killing...");
          channel.requestSendSignal("TERM");
        }

        node.shell.returncode = channel.getExitStatus();

        channel.sendEof();
        channel.close();
      }
      catch (ssh::SshException &e) {
        node.shell.stderr.append(e.getError());
      }

      std::lock_guard<std::mutex> lock(data_mutex);
      d.pimpl->data.push_back(node);
#else
      throw std::runtime_error(
          "remote_shell_command data source is not available");
#endif
    }
    /* \endcond */

    void from_json(const json &j, remote_shell_command &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("version", 0) != d.version()) {
        throw std::runtime_error("Version mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      if (j.contains(json::json_pointer("/data/0/data/returncode"))) {
        d.pimpl->collected = true;
      }

      for (auto i : j.value("data", json::array())) {
        remote_shell_command::impl::item tmp;

        tmp.shell.command = i.value(json::json_pointer("/data/command"), "");
        tmp.shell.elapsed = i.value(json::json_pointer("/data/elapsed"), 0.0);
        tmp.shell.returncode =
            i.value(json::json_pointer("/data/returncode"), 0);
        tmp.shell.stderr = i.value(json::json_pointer("/data/stderr"), "");
        tmp.shell.stdout = i.value(json::json_pointer("/data/stdout"), "");

        tmp.hostname = i.value("hostname", "");
        tmp.timestamp = std::chrono::system_clock::from_time_t(
            i.value("timestamp", static_cast<time_t>(0)));
        tmp.uid = i.value(
            "uid", static_cast<uid_t>(std::numeric_limits<uid_t>::max()));

        d.pimpl->data.push_back(tmp);
      }
    }

    void to_json(json &j, const remote_shell_command &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      j["data"] = json::array();

      for (auto i : d.pimpl->data) {
        json item;

        item["data"]["command"] = i.shell.command;
        item["data"]["elapsed"] = i.shell.elapsed;
        item["data"]["returncode"] = i.shell.returncode;
        item["data"]["stderr"] = i.shell.stderr;
        item["data"]["stdout"] = i.shell.stdout;

        item["hostname"] = i.hostname;
        item["timestamp"] = std::chrono::system_clock::to_time_t(i.timestamp);
        item["uid"] = i.uid;

        j["data"].push_back(item);
      }

      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
