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

var host = "127.0.0.1";
var port = 6710;

var msg3 = '';
var socket3 = net.createConnection({port: port, host: host});

socket3.write('Hello IoT.js')
socket3.write('disconnect')
socket3.on('data', function(data) {
  msg3 += data;
});

process.on('exit', function(code) {
  console.log('exit event', msg3)
});