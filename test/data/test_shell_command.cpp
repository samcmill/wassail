/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define CATCH_CONFIG_MAIN
#include "3rdparty/catch/catch.hpp"
#include "3rdparty/catch/catch_reporter_automake.hpp"

#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>
#include <wassail/data/shell_command.hpp>

TEST_CASE("shell_command simple command") {
  auto d = wassail::data::shell_command("echo 'foo'");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["elapsed"].get<double>() <= 1);
    REQUIRE(j["data"]["returncode"].get<int>() == 0);
    REQUIRE(j["data"]["stderr"].get<std::string>() == "");
    REQUIRE(j["data"]["stdout"].get<std::string>() == "foo\n");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("shell_command simple command with standard error") {
  auto d = wassail::data::shell_command("echo 'foo' && 1>&2 echo 'bar'");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["stderr"].get<std::string>() == "bar\n");
    REQUIRE(j["data"]["stdout"].get<std::string>() == "foo\n");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("shell_command bogus command") {
  auto d = wassail::data::shell_command("/path/to/bogus");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["returncode"].get<int>() != 0);
    REQUIRE(j["data"]["stderr"].get<std::string>() != "");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("shell_command timeout") {
  auto d =
      wassail::data::shell_command("echo 'foo' && sleep 5 && echo 'bar'", 1);

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["elapsed"].get<double>() == Approx(1).epsilon(0.01));
    REQUIRE(j["data"]["returncode"].get<int>() != 0);
    REQUIRE(j["data"]["stdout"].get<std::string>() == "foo\n");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("shell_command no evaluate") {
  auto d = wassail::data::shell_command("echo 'foo'");

  if (d.enabled()) {
    json j = d;

    REQUIRE(j["data"]["elapsed"].get<double>() == Approx(0).epsilon(1e-6));
    REQUIRE(j["data"]["returncode"].get<int>() != 0);
    REQUIRE(j["data"]["stderr"].get<std::string>() == "");
    REQUIRE(j["data"]["stdout"].get<std::string>() == "");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("shell_command long string") {
  // the string should be longer than the chunk size of 4096 bytes,
  // and also not a multiple of the chunk size, so use 16400 (4*4100)
  auto d = wassail::data::shell_command(
      "env LC_ALL=C tr -dc '[:alnum'] < /dev/urandom | dd bs=4 count=4100");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["stdout"].get<std::string>().length() == 16400);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("shell_command environment variables") {
  auto d = wassail::data::shell_command("export FOO='bar' && echo $FOO");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["stdout"].get<std::string>() == "bar\n");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("exclusive shell_command") {
  auto d1 = wassail::data::shell_command("sleep 1");
  d1.exclusive = true;
  auto d2 = wassail::data::shell_command("sleep 1");
  /* d2.exclusive = true; */ /* No need to make both exclusive */

  if (d1.enabled() and d2.enabled()) {
    auto start = std::chrono::steady_clock::now();

    std::future<json> f1 = std::async(std::launch::async, [&d1]() {
      d1.evaluate();
      return static_cast<json>(d1);
    });
    std::future<json> f2 = std::async(std::launch::async, [&d2]() {
      d2.evaluate();
      return static_cast<json>(d2);
    });

    f1.wait();
    f2.wait();

    auto end = std::chrono::steady_clock::now();
    float elapsed =
        std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

    auto j1 = f1.get();
    auto j2 = f2.get();

    REQUIRE(elapsed == Approx(2).epsilon(0.01));
  }
  else {
    REQUIRE_THROWS(d1.evaluate());
  }
}

TEST_CASE("non-exclusive shell_command") {
  auto d1 = wassail::data::shell_command("sleep 1");
  auto d2 = wassail::data::shell_command("sleep 1");

  if (d1.enabled() and d2.enabled()) {
    auto start = std::chrono::steady_clock::now();

    std::future<json> f1 = std::async(std::launch::async, [&d1]() {
      d1.evaluate();
      return static_cast<json>(d1);
    });
    std::future<json> f2 = std::async(std::launch::async, [&d2]() {
      d2.evaluate();
      return static_cast<json>(d2);
    });

    f1.wait();
    f2.wait();

    auto end = std::chrono::steady_clock::now();
    float elapsed =
        std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

    auto j1 = f1.get();
    auto j2 = f2.get();

    REQUIRE(elapsed == Approx(1).epsilon(0.01));
  }
  else {
    REQUIRE_THROWS(d1.evaluate());
  }
}

TEST_CASE("overlapping reader and writer access") {
  /* The guarantee is that the json cast (reader) will block while the data
   * building block is being evaluated (writer).  It is still necessary to
   * ensure that the writer starts before the reader, otherwise the reader will
   * return immediately with "empty" data.
   */

  auto d = wassail::data::shell_command("sleep 1 && echo 'foo'");

  if (d.enabled()) {
    std::mutex m;
    std::condition_variable cv;
    bool started = false;

    std::thread t([&]() {
      {
        std::lock_guard<std::mutex> lock(m);
        started = true;
      }
      cv.notify_all();
      d.evaluate();
    });

    {
      std::unique_lock<std::mutex> lock(m);
      cv.wait(lock, [&started] { return started; });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto j = static_cast<json>(d);

    t.join();

    REQUIRE(j["data"]["stdout"].get<std::string>() == "foo\n");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("shell_command JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "command": "uptime",
        "exclusive": false,
        "timeout": 60
      },
      "data": {
        "command": "uptime",
        "elapsed": 0.011148364,
        "returncode": 0,
        "stderr": "",
        "stdout": "22:53  up 17 days, 23:57, 3 users, load averages: 1.25 1.42 1.63\n"
      },
      "hostname": "localhost.local",
      "name": "shell_command",
      "timestamp": 1528948436,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::shell_command d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);
  REQUIRE(jout == jin);
}

TEST_CASE("shell_command common pointer JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "command": "uptime",
        "exclusive": false,
        "timeout": 60
      },
      "data": {
        "command": "uptime",
        "elapsed": 0.011148364,
        "returncode": 0,
        "stderr": "",
        "stdout": "22:53  up 17 days, 23:57, 3 users, load averages: 1.25 1.42 1.63\n"
      },
      "hostname": "localhost.local",
      "name": "shell_command",
      "timestamp": 1528948436,
      "uid": 99,
      "version": 100
    }
  )"_json;

  std::shared_ptr<wassail::data::common> d =
      std::make_shared<wassail::data::shell_command>();

  d->from_json(jin);
  json jout = d->to_json();

  REQUIRE(jout.size() != 0);
  REQUIRE(jout == jin);
}

TEST_CASE("shell_command invalid JSON conversion") {
  auto jin = R"({ "name": "invalid" })"_json;
  wassail::data::shell_command d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("shell_command incomplete JSON conversion") {
  auto jin =
      R"({ "name": "shell_command", "timestamp": 0, "version": 100})"_json;

  wassail::data::shell_command d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "shell_command");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].count("command") == 1);
  REQUIRE(jout["data"]["command"] == "");
}

TEST_CASE("shell_command factory evaluate") {
  auto jin = R"(
    {
      "configuration": {
        "command": "echo 'foo'"
      },
      "name": "shell_command"
    }
  )"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "shell_command");
    REQUIRE(jout.count("data") == 1);
    REQUIRE(jout["data"]["command"].get<std::string>() == "echo 'foo'");
    REQUIRE(jout["data"]["stdout"].get<std::string>() == "foo\n");
  }
}
