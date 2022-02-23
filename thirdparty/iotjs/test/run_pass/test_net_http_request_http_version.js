/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

var host = "121.89.166.244"
var port = 6710

var request = http.request({
  method: 'GET',
  host: host,
  port: port,
  path: '/version'
}, function(response) {
  assert.equal(response.statusCode, 200);

  response.on('end', function() {
    console.log('end')
  });

  response.on('data', function(chunk){
    console.log(chunk.toString())
    assert.equal(chunk.toString(), 'HTTP/1.1')
  });
})
request.end();
