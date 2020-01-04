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
#include <future>
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

TEST_CASE("shell_command JSON conversion") {
  auto jin = R"(
    {
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

TEST_CASE("shell_command invalid version JSON conversion") {
  auto jin = R"({ "version": 999999 })"_json;
  wassail::data::shell_command d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("shell_command incomplete JSON conversion") {
  auto jin =
      R"({ "name": "shell_command", "timestamp": 0, "version": 100})"_json;
  wassail::data::shell_command d;
  REQUIRE_THROWS(d = jin);
}
