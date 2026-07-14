/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"
#include "internal.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <poll.h>
#include <shared_mutex>
#include <signal.h>
#include <spawn.h>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <wassail/data/shell_command.hpp>

#if defined HAVE_KILLPG && defined HAVE_PIPE && defined HAVE_POLL &&           \
    defined HAVE_POSIX_SPAWNP && defined HAVE_SETPGID && defined HAVE_WAITPID
#define HAVE_SHELL_COMMAND
#endif

namespace wassail {
  namespace data {
    /* \cond pimpl */
    class shell_command::impl {
    public:
      /*! \brief Shell command output data */
      struct {
        std::string command = ""; /*!< Shell command */
        double elapsed = 0.0;     /*!< Number of seconds the command
                                       took to execute */
        int returncode = 255;     /*!< Shell exit status */
        std::string stderr = "";  /*!< Standard error */
        std::string stdout = "";  /*!< Standard output */
      } data;                     /*!< Shell command output data */

      /* \brief Mutex to control concurrent reads and writes */
      std::shared_timed_mutex rw_mutex;

      /*! Private implementation of wassail::data::shell_command::evaluate() */
      void evaluate(shell_command &d, bool force);

    private:
      /*! Execute the shell command, with separate streams for standard
       *  output and standard error.  Populates the Data struct.
       *  \return Exit status of the shell command
       */
      int popen3(shell_command &d);
    };

    shell_command::shell_command() : pimpl{std::make_unique<impl>()} {}
    shell_command::shell_command(std::string _command)
        : pimpl{std::make_unique<impl>()} {
      command = _command;
    }

    shell_command::shell_command(std::string _command, uint8_t _timeout)
        : pimpl{std::make_unique<impl>()} {
      command = _command;
      timeout = _timeout;
    }
    shell_command::~shell_command() = default;
    shell_command::shell_command(shell_command &&) = default; // LCOV_EXCL_LINE
    shell_command &
    shell_command::operator=(shell_command &&) = default; // LCOV_EXCL_LINE

    bool shell_command::enabled() const {
#ifdef HAVE_SHELL_COMMAND
      return true;
#else
      return false;
#endif
    }

    void shell_command::evaluate(bool force) { pimpl->evaluate(*this, force); }

    void shell_command::impl::evaluate(shell_command &d, bool force) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (d.command.empty()) {
        throw std::runtime_error("Missing command");
      }

      if (force or not d.collected()) {
        d.exclusive ? d.mutex.lock() : d.mutex.lock_shared();
        popen3(d);
        d.exclusive ? d.mutex.unlock() : d.mutex.unlock_shared();
        d.common::evaluate_common();
      }
    }

    int shell_command::impl::popen3(shell_command &d) {
#ifdef HAVE_SHELL_COMMAND
      int rv = 0; // child process return value
      int in[2], out[2], err[2];

      // shell command
      data.command = d.command;

      // initialize the pipes
      /* LCOV_EXCL_START */
      if (pipe(in) != 0 or pipe(err) != 0 or pipe(out) != 0) {
        wassail::internal::logger()->error("Failed to initialize pipes");
        return -1;
      }
      /* LCOV_EXCL_STOP */

      // Set up file actions: close unused pipe ends, wire stdin/stdout/stderr
      posix_spawn_file_actions_t fa;
      posix_spawn_file_actions_init(&fa);
      posix_spawn_file_actions_addclose(&fa, in[1]);
      posix_spawn_file_actions_addclose(&fa, out[0]);
      posix_spawn_file_actions_addclose(&fa, err[0]);
      posix_spawn_file_actions_adddup2(&fa, in[0], STDIN_FILENO);
      posix_spawn_file_actions_adddup2(&fa, out[1], STDOUT_FILENO);
      posix_spawn_file_actions_adddup2(&fa, err[1], STDERR_FILENO);

      // Place the child in its own process group
      posix_spawnattr_t attr;
      posix_spawnattr_init(&attr);
      posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);
      posix_spawnattr_setpgroup(&attr, 0);

      char *argv[] = {(char *)"/bin/sh", (char *)"-c",
                      (char *)d.command.c_str(), NULL};
      pid_t child = -1;

      int spawn_err = posix_spawnp(&child, argv[0], &fa, &attr, argv, NULL);

      posix_spawn_file_actions_destroy(&fa);
      posix_spawnattr_destroy(&attr);

      /* LCOV_EXCL_START */
      if (spawn_err != 0) {
        wassail::internal::logger()->error("Failed to spawn child process");
        close(in[0]);
        close(in[1]);
        close(out[0]);
        close(out[1]);
        close(err[0]);
        close(err[1]);
        return -1;
      }
      /* LCOV_EXCL_STOP */

      // Apply double-setpgid to close the race before the first killpg
      setpgid(child, 0);

      // no output from the parent
      close(in[0]);
      close(out[1]);
      close(err[1]);

      pollfd fds[] = {{out[0], POLLIN | POLLERR | POLLHUP | POLLNVAL, 0},
                      {err[0], POLLIN | POLLERR | POLLHUP | POLLNVAL, 0}};

      auto start = std::chrono::steady_clock::now();
      auto current = start;
      std::chrono::duration<double> elapsed = current - start;

      // read stdout and stderr pipes until the command exits normally
      // or the allowed time is exceeded.
      do {
        // calculate the time remaining
        current = std::chrono::steady_clock::now();
        elapsed = current - start;
        int remaining =
            d.timeout -
            std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
        if (remaining < 0) {
          remaining = 0;
        }

        if (poll(fds, 2, remaining) < 0) {
          rv = -1;
          break;
        }

        // read stdout
        if (fds[0].revents & POLLIN) {
          char buf[4096] = {0};
          ssize_t nbytes = read(fds[0].fd, buf, sizeof(buf));

          if (nbytes > 0) {
            data.stdout.append(buf, nbytes);
          }
          else if (nbytes == -1) {
            rv = -1;
            break;
          }
        }

        // read stderr
        if (fds[1].revents & POLLIN) {
          char buf[4096] = {0};
          ssize_t nbytes = read(fds[1].fd, buf, sizeof(buf));

          if (nbytes > 0) {
            data.stderr.append(buf, nbytes);
          }
          else if (nbytes == -1) {
            rv = -1;
            break;
          }
        }

        if ((fds[0].revents & POLLHUP) and (fds[1].revents & POLLHUP)) {
          // the pipes may still have pending data; read until empty
          ssize_t nbytes;

          // drain stdout
          do {
            char buf[4096] = {0};
            nbytes = read(fds[0].fd, buf, sizeof(buf));

            if (nbytes > 0) {
              data.stdout.append(buf, nbytes);
            }
          } while (nbytes > 0);

          // drain stderr
          do {
            char buf[4096] = {0};
            nbytes = read(fds[1].fd, buf, sizeof(buf));

            if (nbytes > 0) {
              data.stderr.append(buf, nbytes);
            }
          } while (nbytes > 0);

          break;
        }

        /* Check for stdout pipe error */
        if (fds[0].revents & POLLERR or fds[1].revents & POLLERR) {
          rv = -1;
          break;
        }
      } while (elapsed.count() < d.timeout);

      // elapsed time taken
      current = std::chrono::steady_clock::now();
      data.elapsed = std::chrono::duration<double>(current - start).count();

      int status = -1;
      if (elapsed.count() >= d.timeout or rv < 0) {
        wassail::internal::logger()->warn(
            "Shell command exceeded allowed time, killing...");
        pid_t pgid = getpgid(child);
        if (killpg(pgid, SIGTERM) != 0) {
          /* LCOV_EXCL_START */
          wassail::internal::logger()->warn(
              "Shell command did not respond to SIGTERM, sending SIGKILL");
          if (killpg(pgid, SIGKILL) != 0) {
            wassail::internal::logger()->error(
                "Shell command did not respond to SIGKILL, likely "
                "leftover processes");
          }
          /* LCOV_EXCL_STOP */
        }
      }

      waitpid(child, &status, 0);

      if (WIFEXITED(status))
        data.returncode = WEXITSTATUS(status);
      else if (WIFSIGNALED(status))
        data.returncode = 128 + WTERMSIG(status);

      // close pipes
      close(out[0]);
      close(err[0]);

      return rv;
#else
      throw std::runtime_error("shell commands not available");
#endif
    }
    /* \endcond */

    void from_json(const json &j, shell_command &d) {
      std::unique_lock<std::shared_timed_mutex> writer(d.pimpl->rw_mutex);

      if (j.value("name", "") != d.name()) {
        throw std::runtime_error("name mismatch");
      }

      from_json(j, dynamic_cast<wassail::data::common &>(d));

      /* Derived classes need to handle their own configuration */
      if (j.value("name", "") == "shell_command") {
        /* Need to keep these default values in sync with shell_command.hpp */
        d.command = j.value(json::json_pointer("/configuration/command"), "");
        if (d.command == "") {
          wassail::internal::logger()->warn("No shell command specified");
        }
        d.exclusive =
            j.value(json::json_pointer("/configuration/exclusive"), false);
        d.timeout = j.value(json::json_pointer("/configuration/timeout"), 60);
      }

      d.pimpl->data.command = j.value(json::json_pointer("/data/command"), "");
      d.pimpl->data.elapsed = j.value(json::json_pointer("/data/elapsed"), 0.0);
      d.pimpl->data.returncode =
          j.value(json::json_pointer("/data/returncode"), 0);
      d.pimpl->data.stderr = j.value(json::json_pointer("/data/stderr"), "");
      d.pimpl->data.stdout = j.value(json::json_pointer("/data/stdout"), "");
    }

    void to_json(json &j, const shell_command &d) {
      std::shared_lock<std::shared_timed_mutex> reader(d.pimpl->rw_mutex);

      j = dynamic_cast<const wassail::data::common &>(d);

      /* Derived classes need to handle their own configuration */
      if (d.name() == "shell_command") {
        j["configuration"]["command"] = d.command;
        j["configuration"]["exclusive"] = d.exclusive;
        j["configuration"]["timeout"] = d.timeout;
      }

      j["data"]["command"] = d.pimpl->data.command;
      j["data"]["elapsed"] = d.pimpl->data.elapsed;
      j["data"]["returncode"] = d.pimpl->data.returncode;
      j["data"]["stderr"] = d.pimpl->data.stderr;
      j["data"]["stdout"] = d.pimpl->data.stdout;
      j["name"] = d.name();
      j["version"] = d.version();
    }
  } // namespace data
} // namespace wassail
