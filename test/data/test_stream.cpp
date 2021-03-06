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
#include <mutex>
#include <thread>
#include <wassail/data/stream.hpp>

TEST_CASE("stream basic usage") {
  auto d = wassail::data::stream();

  REQUIRE(d.command == std::string(WASSAIL_LIBEXECDIR) + "/stream");

  if (d.enabled()) {
    d.evaluate();
    json j = d;

    REQUIRE(j["data"]["triad"] >= 1);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("overlapping reader and writer access") {
  /* The guarantee is that the json cast (reader) will block while the data
   * building block is being evaluated (writer).  It is still necessary to
   * ensure that the writer starts before the reader, otherwise the reader will
   * return immediately with "empty" data.
   */
  auto d = wassail::data::stream();

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

    REQUIRE(j["data"]["triad"] >= 1);
  }
  else {
    REQUIRE_THROWS(d.evaluate());
  }
}

TEST_CASE("stream JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "command": "/usr/libexec/wassail/stream",
        "elapsed": 0.982017832,
        "returncode": 0,
        "stderr": "",
        "stdout": "-------------------------------------------------------------\nSTREAM version $Revision: 5.10 $\n-------------------------------------------------------------\nThis system uses 8 bytes per array element.\n-------------------------------------------------------------\nArray size = 10000000 (elements), Offset = 0 (elements)\nMemory per array = 76.3 MiB (= 0.1 GiB).\nTotal memory required = 228.9 MiB (= 0.2 GiB).\nEach kernel will be executed 10 times.\n The *best* time for each kernel (excluding the first iteration)\n will be used to compute the reported bandwidth.\n-------------------------------------------------------------\nYour clock granularity/precision appears to be 1 microseconds.\nEach test below will take on the order of 11497 microseconds.\n   (= 11497 clock ticks)\nIncrease the size of the arrays if this shows that\nyou are not getting at least 20 clock ticks per test.\n-------------------------------------------------------------\nWARNING -- The above is only a rough guideline.\nFor best results, please be sure you know the\nprecision of your system timer.\n-------------------------------------------------------------\nFunction    Best Rate MB/s  Avg time     Min time     Max time\nCopy:           15941.1     0.010863     0.010037     0.012266\nScale:          11350.7     0.014821     0.014096     0.015258\nAdd:            11951.0     0.021416     0.020082     0.022762\nTriad:          12277.4     0.021307     0.019548     0.023780\n-------------------------------------------------------------\nSolution Validates: avg error less than 1.000000e-13 on all three arrays\n-------------------------------------------------------------\n"
      },
      "hostname": "localhost.local",
      "name": "stream",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  wassail::data::stream d = jin;
  json jout = d;

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  REQUIRE(jout["data"]["add"] == 11951.0);
  REQUIRE(jout["data"]["copy"] == 15941.1);
  REQUIRE(jout["data"]["scale"] == 11350.7);
  REQUIRE(jout["data"]["triad"] == 12277.4);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("add");
  jout["data"].erase("copy");
  jout["data"].erase("scale");
  jout["data"].erase("triad");
  REQUIRE(jout == jin);
}

TEST_CASE("stream common pointer JSON conversion") {
  auto jin = R"(
    {
      "data": {
        "command": "/usr/libexec/wassail/stream",
        "elapsed": 0.982017832,
        "returncode": 0,
        "stderr": "",
        "stdout": "-------------------------------------------------------------\nSTREAM version $Revision: 5.10 $\n-------------------------------------------------------------\nThis system uses 8 bytes per array element.\n-------------------------------------------------------------\nArray size = 10000000 (elements), Offset = 0 (elements)\nMemory per array = 76.3 MiB (= 0.1 GiB).\nTotal memory required = 228.9 MiB (= 0.2 GiB).\nEach kernel will be executed 10 times.\n The *best* time for each kernel (excluding the first iteration)\n will be used to compute the reported bandwidth.\n-------------------------------------------------------------\nYour clock granularity/precision appears to be 1 microseconds.\nEach test below will take on the order of 11497 microseconds.\n   (= 11497 clock ticks)\nIncrease the size of the arrays if this shows that\nyou are not getting at least 20 clock ticks per test.\n-------------------------------------------------------------\nWARNING -- The above is only a rough guideline.\nFor best results, please be sure you know the\nprecision of your system timer.\n-------------------------------------------------------------\nFunction    Best Rate MB/s  Avg time     Min time     Max time\nCopy:           15941.1     0.010863     0.010037     0.012266\nScale:          11350.7     0.014821     0.014096     0.015258\nAdd:            11951.0     0.021416     0.020082     0.022762\nTriad:          12277.4     0.021307     0.019548     0.023780\n-------------------------------------------------------------\nSolution Validates: avg error less than 1.000000e-13 on all three arrays\n-------------------------------------------------------------\n"
      },
      "hostname": "localhost.local",
      "name": "stream",
      "timestamp": 1539144880,
      "uid": 99,
      "version": 100
    }
  )"_json;

  std::shared_ptr<wassail::data::common> d =
      std::make_shared<wassail::data::stream>();

  d->from_json(jin);
  json jout = d->to_json();

  REQUIRE(jout.size() != 0);

  /* Verify fields are parsed correctly from stdout */
  REQUIRE(jout["data"]["add"] == 11951.0);
  REQUIRE(jout["data"]["copy"] == 15941.1);
  REQUIRE(jout["data"]["scale"] == 11350.7);
  REQUIRE(jout["data"]["triad"] == 12277.4);

  /* jout should be equal to jin except for the parsed process list */
  jout["data"].erase("add");
  jout["data"].erase("copy");
  jout["data"].erase("scale");
  jout["data"].erase("triad");
  REQUIRE(jout == jin);
}

TEST_CASE("stream factory evaluate") {
  auto jin = R"({ "name": "stream" })"_json;

  auto jout = wassail::data::evaluate(jin);

  if (not jout.is_null()) {
    REQUIRE(jout["name"] == "stream");
    REQUIRE(jout.count("data") == 1);

    /* The factory method is compiled separately with the normal libexecdir, not
     * the specially defined value used when building this test.  Therefore
     * unless the package has been installed, the stream binary will not be
     * found. */
    if (jout["data"]["command"] ==
            std::string(WASSAIL_LIBEXECDIR) + "/stream" or
        jout["data"]["returncode"] == 0) {
      REQUIRE(jout["data"]["triad"] >= 1);
    }
  }
}
