# IoT.js: Platform for Internet of Things with JavaScript
[![License](https://img.shields.io/badge/licence-Apache%202.0-brightgreen.svg?style=flat)](LICENSE)
[![Build Status](https://travis-ci.org/jerryscript-project/iotjs.svg?branch=master)](https://travis-ci.org/jerryscript-project/iotjs)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/12140/badge.svg)](https://scan.coverity.com/projects/samsung-iotjs)
[![SonarCloud Status](https://sonarcloud.io/api/project_badges/measure?project=pando-project_iotjs&metric=alert_status)](https://sonarcloud.io/dashboard?id=pando-project_iotjs)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FSamsung%2Fiotjs.svg?type=shield)](https://app.fossa.io/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FSamsung%2Fiotjs?ref=badge_shield)
[![IRC Channel](https://img.shields.io/badge/chat-on%20freenode-brightgreen.svg)](https://kiwiirc.com/client/irc.freenode.net/#iotjs)

You can find project details on our [project page](http://jerryscript-project.github.io/iotjs/) and [wiki](https://github.com/jerryscript-project/iotjs/wiki).

Memory usage and Binary footprint are measured at [here](https://jerryscript-project.github.io/iotjs-test-results) with real target daily.

The following table shows the latest results on the devices:

|      Raspberry Pi 3         | [![Remote Testrunner](https://firebasestorage.googleapis.com/v0/b/jsremote-testrunner.appspot.com/o/status%2Fiotjs%2Frpi3.svg?alt=media&token=1)](https://jerryscript-project.github.io/iotjs-test-results/?view=rpi3)  |
| :---: | :---: |
| **Raspberry Pi 2**    | [![Remote Testrunner](https://firebasestorage.googleapis.com/v0/b/jsremote-testrunner.appspot.com/o/status%2Fiotjs%2Frpi2.svg?alt=media&token=1)](https://jerryscript-project.github.io/iotjs-test-results/?view=rpi2)          |
| **STM32F4-Discovery** | [![Remote Testrunner](https://firebasestorage.googleapis.com/v0/b/jsremote-testrunner.appspot.com/o/status%2Fiotjs%2Fstm32f4dis.svg?alt=media&token=1)](https://jerryscript-project.github.io/iotjs-test-results/?view=stm32f4dis)   |


IRC channel: #iotjs on [freenode](https://freenode.net)
Mailing list: iotjs-dev@groups.io, you can subscribe [here](https://groups.io/g/iotjs-dev) and access the mailing list archive [here](https://groups.io/g/iotjs-dev/topics).

## Quick Start
### Getting the sources

```bash
git clone https://github.com/jerryscript-project/iotjs.git
cd iotjs
```

### How to Build

```bash
tools/build.py
```

### How to Test

```bash
tools/testrunner.py build/x86_64-linux/debug/bin/iotjs
```

### Trying out with a REPL

```bash
build/x86_64-linux/debug/bin/iotjs tools/repl.js
```


For Additional information see [Getting Started](docs/Getting-Started.md).

## Documentation
- [Getting Started](docs/Getting-Started.md)
- [API Reference](docs/api/IoT.js-API-reference.md)

## License
IoT.js is Open Source software under the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0). Complete license and copyright information can be found within the code.

[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FSamsung%2Fiotjs.svg?type=large)](https://app.fossa.io/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FSamsung%2Fiotjs?ref=badge_large)

> Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors

> Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

> Copyright Node.js contributors. All rights reserved.

> Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to
 deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

> The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 IN THE SOFTWARE.

> This license applies to parts of '*.js' files in '/src/js', implementing node.js
 compatible API, originating from the https://github.com/nodejs/node repository:
