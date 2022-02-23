/* Copyright 2016-present Samsung Electronics Co., Ltd. and other contributors
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

var assert = require('assert');
// var pin = require('tools/systemio_common').pin;
// var checkError = require('tools/systemio_common').checkError;
var i2c = require('i2c');

function checkError(err) {
  if (err) {
    console.log('Have an error: ' + err.message);
    assert.fail();
  }
}

function at24SetCurAddr(_i2c, addr) {
  _i2c.writeSync([addr])
}

function at24ReadSync(_i2c, addr, num) {
  at24SetCurAddr(_i2c, addr)
  return _i2c.readSync(num)
}

function at24Write(_i2c, addr, data, callback) {
  arr = [addr]
  arr = arr.concat(data)
  _i2c.write(arr, callback)
}

var configuration = {
  address: 0x50,
  device: '/dev/hard_i2c1'
};

var data1 = [11,12,34,45,24,34]
var data2 = [45,46,78,89,25,36]

at24 = i2c.openSync(configuration);

at24Write(at24, 0, data1, function(err) {
  checkError(err);

  setTimeout(function() {
    var res = at24ReadSync(at24, 0, 6)
    console.log('read result', res);
    at24.closeSync()
  }, 1000);
})
