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
var ClientRequest = require('http_client').ClientRequest;
var IncomingMessage = require('http_incoming').IncomingMessage;
var HTTPParser = require('http_parser').HTTPParser;

exports.ClientRequest = ClientRequest;

exports.request = function(options, cb) {
  // Create socket.
  var socket = new net.Socket();
  options.port = options.port || 80;

  return new ClientRequest(options, cb, socket);
};

exports.METHODS = HTTPParser.methods;

exports.get = function(options, cb) {
  var req = exports.request(options, cb);
  req.end();
  return req;
};

exports.IncomingMessage = IncomingMessage;
