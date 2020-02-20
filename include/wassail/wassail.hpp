/* Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#ifndef _WASSAIL_HPP
#define _WASSAIL_HPP

#if __cplusplus < 201402L
#error "A C++14 or later compiler is required"
#endif

/* Helpers */
#include <wassail/common.hpp>
#include <wassail/result.hpp>

/* 3rd party components */
#include <wassail/json/json.hpp>

using json = nlohmann::json;

/* Data sources */
#include <wassail/data/environment.hpp>
#include <wassail/data/getcpuid.hpp>
#include <wassail/data/getfsstat.hpp>
#include <wassail/data/getloadavg.hpp>
#include <wassail/data/getmntent.hpp>
#include <wassail/data/getrlimit.hpp>
#include <wassail/data/pciaccess.hpp>
#include <wassail/data/pciutils.hpp>
#include <wassail/data/ps.hpp>
#include <wassail/data/remote_shell_command.hpp>
#include <wassail/data/shell_command.hpp>
#include <wassail/data/stat.hpp>
#include <wassail/data/stream.hpp>
#include <wassail/data/sysconf.hpp>
#include <wassail/data/sysctl.hpp>
#include <wassail/data/sysinfo.hpp>
#include <wassail/data/umad.hpp>
#include <wassail/data/uname.hpp>

/* Checks */
#include <wassail/checks/compare.hpp>
#include <wassail/checks/cpu/core_count.hpp>
#include <wassail/checks/disk/amount_free.hpp>
#include <wassail/checks/disk/percent_free.hpp>
#include <wassail/checks/file/permissions.hpp>
#include <wassail/checks/memory/physical_size.hpp>
#include <wassail/checks/misc/environment.hpp>
#include <wassail/checks/misc/load_average.hpp>
#include <wassail/checks/rules_engine.hpp>

#endif
