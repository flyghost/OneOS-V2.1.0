/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// var EventEmitter = require('events').EventEmitter;

// TODO : gpio irq feature
// function mixin(target, source) {
//   for (var prop in source) {
//     if (source.hasOwnProperty(prop) && !target[prop]) {
//       target[prop] = source[prop];
//     }
//   }
// }

var gpio = {
  open: function(config, callback) {
    var gpioPin = new native(config, function(err) {
      // mixin(gpioPin, EventEmitter.prototype);
      callback(err, gpioPin);
    });
    return gpioPin;
  },
  openSync: function(config) {
    var gpioPin = new native(config);
    // mixin(gpioPin, EventEmitter.prototype);
    return gpioPin;
  },
  DIRECTION: native.DIRECTION,
  EDGE: native.EDGE,
  MODE: native.MODE,
};

module.exports = gpio;
