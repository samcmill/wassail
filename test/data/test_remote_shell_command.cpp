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
#include <wassail/data/remote_shell_command.hpp>

/* Some tests may fail if ssh is not setup */

TEST_CASE("remote_shell_command simple command", "[!mayfail]") {
  auto d = wassail::data::remote_shell_command("localhost", "echo 'foo'");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"].size() == 1);
    REQUIRE(j["data"][0]["data"]["elapsed"].get<double>() <= 1);
    REQUIRE(j["data"][0]["data"]["returncode"].get<int>() == 0);
    REQUIRE(j["data"][0]["data"]["stderr"].get<std::string>() == "");
    REQUIRE(j["data"][0]["data"]["stdout"].get<std::string>() == "foo\n");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("remote_shell_command simple command, multiple hosts", "[!mayfail]") {
  auto d = wassail::data::remote_shell_command(
      std::list<std::string>({"localhost", "localhost"}), "echo 'foo'");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"].size() == 2);
    REQUIRE(j["data"][0]["data"]["elapsed"].get<double>() <= 1);
    REQUIRE(j["data"][0]["data"]["returncode"].get<int>() == 0);
    REQUIRE(j["data"][0]["data"]["stderr"].get<std::string>() == "");
    REQUIRE(j["data"][0]["data"]["stdout"].get<std::string>() == "foo\n");
    REQUIRE(j["data"][1]["data"]["elapsed"].get<double>() <= 1);
    REQUIRE(j["data"][1]["data"]["returncode"].get<int>() == 0);
    REQUIRE(j["data"][1]["data"]["stderr"].get<std::string>() == "");
    REQUIRE(j["data"][1]["data"]["stdout"].get<std::string>() == "foo\n");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("remote_shell_command simple command with standard error",
          "[!mayfail]") {
  auto d = wassail::data::remote_shell_command("localhost",
                                               "echo 'foo' && 1>&2 echo 'bar'");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"].size() == 1);
    REQUIRE(j["data"][0]["data"]["stderr"].get<std::string>() == "bar\n");
    REQUIRE(j["data"][0]["data"]["stdout"].get<std::string>() == "foo\n");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("remote_shell_command bogus host") {
  auto d = wassail::data::remote_shell_command("bogus_host", "echo 'a'");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"].size() == 1);
    REQUIRE(j["data"][0]["data"]["returncode"].get<int>() != 0);
    REQUIRE(j["data"][0]["data"]["stderr"].get<std::string>() != "");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("remote_shell_command bogus command", "[!mayfail]") {
  auto d = wassail::data::remote_shell_command("localhost", "/path/to/bogus");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"].size() == 1);
    REQUIRE(j["data"][0]["data"]["returncode"].get<int>() != 0);
    REQUIRE(j["data"][0]["data"]["stderr"].get<std::string>() != "");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("remote_shell_command timeout", "[!mayfail]") {
  auto d = wassail::data::remote_shell_command(
      std::list<std::string>({"localhost"}),
      "echo 'foo' && sleep 5 && echo 'bar'", 1);

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"].size() == 1);
    REQUIRE(j["data"][0]["data"]["elapsed"].get<double>() ==
            Approx(1).epsilon(0.01));
    REQUIRE(j["data"][0]["data"]["returncode"].get<int>() != 0);
    REQUIRE(j["data"][0]["data"]["stdout"].get<std::string>() == "foo\n");
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("remote_shell_command long string", "[!mayfail]") {
  // the string should be longer than the chunk size of 4096 bytes,
  // and also not a multiple of the chunk size, so use 16400 (4*4100)
  auto d = wassail::data::remote_shell_command(
      "localhost",
      "env LC_ALL=C tr -dc '[:alnum'] < /dev/urandom | dd bs=4 count=4100");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"].size() == 1);
    REQUIRE(j["data"][0]["data"]["stdout"].get<std::string>().length() ==
            16400);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("remote_shell_command JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "command": "uptime",
        "exclusive": false,
        "hosts": [ "node1", "node2" ],
        "timeout": 10
      },
      "data": [
        {
          "data": {
            "command": "uptime",
            "elapsed": 0.011148364,
            "returncode": 0,
            "stderr": "",
            "stdout": "22:53  up 17 days, 23:57, 3 users, load averages: 1.25 1.42 1.63\n"
          },
          "hostname": "node1",
          "timestamp": 152894836,
          "uid": 99
        },
        {
          "data": {
            "command": "uptime",
            "elapsed": 0.011124915,
            "returncode": 0,
            "stderr": "",
            "stdout": "22:53  up 17 days, 23:57, 3 users, load averages: 0.43 0.42 0.63\n"
          },
          "hostname": "node2",
          "timestamp": 152894836,
          "uid": 99
        }
      ],
      "hostname": "localhost.local",
      "name": "remote_shell_command",
      "timestamp": 1528948436,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::remote_shell_command d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);
  REQUIRE(jout == jin);
}

TEST_CASE("remote_shell_command common pointer JSON conversion") {
  auto jin = R"(
    {
      "configuration": {
        "command": "uptime",
        "exclusive": false,
        "hosts": [ "node1", "node2" ],
        "timeout": 10
      },
      "data": [
        {
          "data": {
            "command": "uptime",
            "elapsed": 0.011148364,
            "returncode": 0,
            "stderr": "",
            "stdout": "22:53  up 17 days, 23:57, 3 users, load averages: 1.25 1.42 1.63\n"
          },
          "hostname": "node1",
          "timestamp": 152894836,
          "uid": 99
        },
        {
          "data": {
            "command": "uptime",
            "elapsed": 0.011124915,
            "returncode": 0,
            "stderr": "",
            "stdout": "22:53  up 17 days, 23:57, 3 users, load averages: 0.43 0.42 0.63\n"
          },
          "hostname": "node2",
          "timestamp": 152894836,
          "uid": 99
        }
      ],
      "hostname": "localhost.local",
      "name": "remote_shell_command",
      "timestamp": 1528948436,
      "uid": 99,
      "version": 100
    }
  )"_json;

  std::shared_ptr<wassail::data::common> d =
      std::make_shared<wassail::data::remote_shell_command>();

  d->from_json(jin);
  json jout = d->to_json();

  REQUIRE(jout.size() != 0);
  REQUIRE(jout == jin);
}

TEST_CASE("remote_shell_command invalid JSON conversion") {
  auto jin = R"({ "name": "invalid" })"_json;
  wassail::data::remote_shell_command d;
  REQUIRE_THROWS(d = jin);
}

TEST_CASE("remote_shell_command incomplete JSON conversion") {
  auto jin =
      R"({ "name": "remote_shell_command", "timestamp": 0, "version": 100})"_json;

  wassail::data::remote_shell_command d = jin;
  json jout = d;

  REQUIRE(jout["name"] == "remote_shell_command");
  REQUIRE(jout.count("data") == 1);
  REQUIRE(jout["data"].size() == 0);
}

TEST_CASE("remote_shell_command factory evaluate") {
  auto jin = R"(
    {
      "configuration": {
        "command": "echo 'foo'",
        "hosts": [ "localhost" ]
      },
      "name": "remote_shell_command"
    }
  )"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "remote_shell_command");
    REQUIRE(jout.count("data") == 1);
    REQUIRE(jout["data"].size() == 1);
    REQUIRE(jout["data"][0]["data"]["command"].get<std::string>() ==
            "echo 'foo'");
    if (jout["data"][0]["data"]["returncode"].get<int>() == 0) {
      REQUIRE(jout["data"][0]["data"]["stdout"].get<std::string>() == "foo\n");
    }
  }
}
