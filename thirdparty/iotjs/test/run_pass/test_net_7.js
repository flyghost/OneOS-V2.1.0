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

var socket = new net.Socket();
var msg = "";

socket.connect(port, host);
socket.on('connect', function() {
  // client writes "1" first, but server is paused for 2 secs
  // server gets "1" after 2 secs
  socket.write('Hello IoT.js');

  // "2" is appended to msg before "1"
  timers.setTimeout(function() {
    msg += '2';
  }, 2000);
});

socket.on('data', function(data) {
  msg += '1';
  socket.end();
});

process.on('exit', function(code) {
  assert.equal(code, 0);
  assert.equal(msg, "12");
  console.log('process exit', msg);
});
