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

var net = require('net');
var assert = require('assert');
var timers = require('timers');

var host = "127.0.0.1";
var port = 6710;

// 在使用lwip时，如果同时连接多个socket，需要修改MEMP_NUM_NETDB=socket数
var sock1 = new net.Socket();
var sock2 = new net.Socket();

var msg = '';

sock1.connect(port, host);
sock2.connect(port, host);

sock1.on('data', function(data) {
  msg += data;
});

sock1.on('end', function() {
  sock1.end();
});

sock2.on('data', function(data) {
  msg += data;
});

sock2.on('end', function() {
  sock2.end();
});

timers.setTimeout(function() {
  sock1.write('sock1');
  sock1.write('disconnect');
}, 1000);

timers.setTimeout(function() {
  sock2.write('sock2');
  sock2.write('disconnect');
}, 2000);

process.on('exit', function(code) {
  assert.equal(code, 0);
  console.log(msg)
});
