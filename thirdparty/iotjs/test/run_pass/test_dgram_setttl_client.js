/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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
var dgram = require('dgram');

var host = "121.89.166.244";
var port = 6711;

var startlis = false;

var client = dgram.createSocket('udp4');

client.bind()

client.on('error', function(err) {
  assert.fail();
  console.log('error');
});

client.on('listening', function(err) {
  console.log('listening');
  startlis = true;
});

client.on('message', function(data, rinfo) {
  console.log('client got data : ' + data);
  console.log('server address : ' + rinfo.address);
  console.log('server port : ' + rinfo.port);
  console.log('server family : ' + rinfo.family);
  if (data.length > 3) {
    console.log('client close');
    clearInterval(timer);
    client.close();
  }
});

var ttl = 3;
var timer = setInterval(function() {
  if (startlis) {
    client.setTTL(ttl);
    client.send('ttl ' + ttl, port, host, function(err, len) {
      assert.equal(err, null);
    });
    ttl++;
  }
}, 100);

process.on('exit', function(code) {
  assert.equal(code, 0);
});
