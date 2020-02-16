# wassail

[![Build status](https://img.shields.io/github/workflow/status/samcmill/wassail/Linux%20build/master?label=Linux%20Build)](https://github.com/samcmill/wassail/actions?query=workflow%3A%22Linux+build%22)
[![Build status](https://img.shields.io/github/workflow/status/samcmill/wassail/macOS%20build/master?label=macOS%20Build)](https://github.com/samcmill/wassail/actions?query=workflow%3A%22macOS+build%22)
[![Coverage Status](https://coveralls.io/repos/github/samcmill/wassail/badge.svg?branch=master)](https://coveralls.io/github/samcmill/wassail?branch=master)
[![Coverity Status](https://scan.coverity.com/projects/20226/badge.svg)](https://scan.coverity.com/projects/samcmill-wassail)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/samcmill/wassail.svg)](https://lgtm.com/projects/g/samcmill/wassail/context:cpp)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://samcmill.github.io/wassail)
[![License](https://img.shields.io/github/license/samcmill/wassail)](https://github.com/samcmill/wassail/blob/master/LICENSE)

wassail is a set of C++ building blocks to check the health of
UNIX-based systems.  Python bindings are also included.

The library is based on the observation that high performance
computing sites tend to create their own custom local health tools.
The user interfaces vary widely but the health checks themselves are
universal. wassail provides highly customizable "low level" building
blocks for creating bespoke user level system health tools.

## Examples

Check whether the value of the environment variable `FOO` is equal to
`bar`:

```
#include <wassail.hpp>

auto d = wassail::data::environment();
auto c = wassail::check::misc::environment("FOO", "bar");
auto r = c.check(d); /* implicitly evalutes d */

// {"action":"","brief":"Checking environment variable 'FOO'","children":[],"detail":"Value 'bar' matches 'bar'","issue":0,"priority":3,"system_id":["MacBook-Pro.local"],"timestamp":1578031355}
std::cout << static_cast<json>(r) << std::endl;
```

The three categories of building blocks are shown in this example.

1. Data building blocks collect information about the system.  wassail
   provides multiple data sources in the `wassail::data` namespace.
   Data sources are not queried when the object is initialized but
   rather when the `evaluate()` method is invoked (the data is
   cached).

2. Check building blocks validate the collected data against a
   specified reference value.  wassail provides multiple checks in the
   `wassail::check` namespace as well as flexible generic checks.  The
   reference value could be hard-coded or user configurable via the
   user interface of the user level system health tool.

3. Checks return result objects.  The user level health tool can
   easily customize the presentation of the result, such as choosing
   the terminology to appear if an issue is detected.  In C++ tools
   the result `<<` operator can be overloaded.  The message strings
   can also be customized by specifying alternative templates when
   constructing the check.

```
// customize the result message templates (libfmt syntax)
auto c = wassail::check::misc::environment("FOO", "bar", false,
           "Validating environment variable {0}",
           "Observed value '{0}' does not equal the reference value '{1}'",
           "Unable to validate",
           "Observed value and reference value are the same: '{0}'");
auto r = c.check(d); /* implicitly evalutes d */

// {"action":"","brief":"Validating environment variable FOO","children":[],"detail":"Observed value and reference value are the same: 'bar'","issue":0,"priority":3,"system_id":["MacBook-Pro.local"],"timestamp":1578033076}
std::cout << static_cast<json>(r) << std::endl;
```

The equivalent example to check the value of the environment variable
`FOO` using the Python binding:

```
import wassail

d = wassail.data.environment()
c = wassail.check.misc.environment('FOO', 'bar')
r = c.check(d) # implicitly evaluates d

# {"action":"","brief":"Checking environment variable 'FOO'","children":[],"detail":"Value 'bar' matches 'bar'","issue":0,"priority":3,"system_id":["MacBook-Pro.local"],"timestamp":1578031355}
print(r)
```

Several C++ and Python [samples](src/samples) are included.

## Installation

A C++14 compiler is required (C++17 preferred).  See [INSTALL](INSTALL).

## License

wassail is distributed under the [Mozilla Public License 2.0](LICENSE).

Copyright © 2018-2020 Scott McMillan

## Third Party Software

wassail incorporates the following third party software.

- [catch](https://github.com/catchorg/Catch2)
 Copyright (c) 2019 Two Blue Cubes Ltd.

- [fmtlib](https://github.com/fmtlib/fmt)
 Copyright (c) 2012 - present, Victor Zverovich

- [json](https://github.com/nlohmann/json)
 Copyright (c) 2013-2019 Niels Lohmann &lt;http://nlohmann.me&gt;

- [pybind11](https://github.com/pybind/pybind11)
 Copyright (c) 2016 Wenzel Jakob &lt;wenzel.jakob@epfl.ch&gt;

- [pybind11_json](https://github.com/pybind/pybind11_json)
 Copyright (c) 2019, Martin Renou

- [spdlog](https://github.com/gabime/spdlog)
 Copyright (c) 2015-present, Gabi Melman & spdlog contributors.

- [STREAM](http://www.cs.virginia.edu/stream/ref.html)
 Copyright 1991-2013: John D. McCalpin

## Entomology

wassail originates from the Middle English "wæs hæil", or "be in good
health".  May your system be in good health!

> Here we come a-wassailing  
> Among the leaves so green  
> Here we come a-wand'ring  
> So fair to be seen  

> Love and joy come to you,  
> And to you your wassail too;  
> And God bless you and send you a Happy New Year  
> And God send you a Happy New Year  
