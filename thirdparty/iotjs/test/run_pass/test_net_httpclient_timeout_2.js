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



var assert = require('assert');
var http = require('http');

var timeouted = false;

var host = "121.89.166.244"
var port = 6710

var options = {
  host: host,
  port: port,
  method: 'GET',
  path: '/delay'
};

var req = http.request(options, function(res) {
  var destroyer = function() {
    timeouted = true;
    req.socket.destroy();
  }

  res.on('data', function() {
    // after connection established
    req.setTimeout(100, destroyer);
  });
});

req.on('close', function() {});

var before = function(){
  /* this handler must not be called  */
};
req.setTimeout(1000, before);

req.on('error', function(){});
req.end();

process.on('exit', function(code) {
  assert.equal(timeouted, true);
  console.log('iotjs exit')
});
