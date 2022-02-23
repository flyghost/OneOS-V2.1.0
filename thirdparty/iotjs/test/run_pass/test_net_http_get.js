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

// 1. GET req
options = {
  method : 'GET',
  port : 80,
  hostname : "os.iot.10086.cn"
};

var getResponseHandler = function (res) {

  assert.equal(301, res.statusCode);

  var endHandler = function(){
    console.log('end')
  };
  res.on('end', endHandler);

  res.on('data', function(chunk){
    console.log(chunk.toString())
  });
};

http.get(options, getResponseHandler);
