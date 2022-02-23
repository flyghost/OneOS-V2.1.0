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

var host = "121.89.166.244"
var port = 6710

var res_body = '';

// 2. echo server req
var finalMsg = 'echo hello';
var finalOptions = {
  method : 'POST',
  host : host,
  path : '/echo',
  port : port,
  headers : {'content-type': 'text/plain',
    'Content-Length': finalMsg.length}
};

var finalResponseHandler = function (res) {

  assert.equal(200, res.statusCode);

  var endHandler = function(){
    assert.equal(finalMsg, res_body);
  };
  res.on('end', endHandler);

  res.on('data', function(chunk){
    res_body += chunk.toString();
  });
};

var finalReq = http.request(finalOptions, finalResponseHandler);
finalReq.write(finalMsg);
finalReq.end();

process.on('exit', function(code) {
  console.log(res_body)
});
