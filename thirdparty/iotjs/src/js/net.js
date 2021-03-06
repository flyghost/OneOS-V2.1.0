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


var EventEmitter = require('events').EventEmitter;
var stream = require('stream');
var util = require('util');
var assert = require('assert');
var Tcp = require('tcp');

function createTCP() {
  var _tcp = new Tcp();
  return _tcp;
}


function SocketState(options) {
  // 'true' during connection handshaking.
  this.connecting = false;

  // become 'true' when connection established.
  this.connected = false;

  this.writable = true;
  this.readable = true;

  this.destroyed = false;
  this.errored = false;

  this.allowHalfOpen = options && options.allowHalfOpen || false;
}


function Socket(options) {
  if (!(this instanceof Socket)) {
    return new Socket(options);
  }

  if (options === undefined) {
    options = {};
  }

  stream.Duplex.call(this, options);

  this._timer = null;
  this._timeout = 0;

  this._socketState = new SocketState(options);

  if (options.handle) {
    this._handle = options.handle;
    this._handle.owner = this;
  }

  this.on('finish', onSocketFinish);
  this.on('end', onSocketEnd);
}


// Socket inherits Duplex.
util.inherits(Socket, stream.Duplex);


Socket.prototype.connect = function() {
  var self = this;
  var state = self._socketState;

  var args = normalizeConnectArgs(arguments);
  var options = args[0];
  var callback = args[1];

  if (state.connecting || state.connected) {
    return self;
  }

  if (!self._handle) {
    self._handle = createTCP();
    self._handle.owner = self;
  }

  if (util.isFunction(callback)) {
    self.once('connect', callback);
  }

  resetSocketTimeout(self);

  state.connecting = true;

  var dns = require('dns');
  var host = options.host ? options.host : 'localhost';
  var port = options.port;
  var dnsopts = {
    family: options.family >>> 0,
    hints: 0,
  };

  if (!util.isNumber(port) || port < 0 || port > 65535)
    throw new RangeError('port should be >= 0 and < 65536: ' + options.port);

  if (dnsopts.family !== 0 && dnsopts.family !== 4 && dnsopts.family !== 6)
    throw new RangeError('family should be 4 or 6: ' + dnsopts.family);

  self._host = host;
  dns.lookup(host, dnsopts, function(err, ip, family) {
    if (self._socketState.destroyed) {
      return;
    }
    self.emit('lookup', err, ip, family);

    if (err) {
      process.nextTick(function() {
        self.emit('error', err);
        self.destroy();
      });
    } else {
      resetSocketTimeout(self);
      connect(self, ip, port);
    }
  });

  return self;
};


Socket.prototype.write = function(data, callback) {
  if (!util.isString(data) && !util.isBuffer(data)) {
    throw new TypeError('invalid argument');
  }
  return stream.Duplex.prototype.write.call(this, data, callback);
};


Socket.prototype._write = function(chunk, callback, afterWrite) {
  assert(util.isBuffer(chunk));
  assert(util.isFunction(afterWrite));

  var self = this;

  if (self.errored) {
    process.nextTick(afterWrite, 1);
    if (util.isFunction(callback)) {
      process.nextTick(function(self, status) {
        callback.call(self, status);
      }, self, 1);
    }
  } else {
    resetSocketTimeout(self);

    self._handle.owner = self;

    self._handle.write(chunk, function(status) {
      afterWrite(status);
      if (util.isFunction(callback)) {
        callback.call(self, status);
      }
    });
  }
};


Socket.prototype.end = function(data, callback) {
  var self = this;
  var state = self._socketState;

  // end of writable stream.
  stream.Writable.prototype.end.call(self, data, callback);

  // this socket is no longer writable.
  state.writable = false;
};


// Destroy this socket as fast as possible.
Socket.prototype.destroy = function() {
  var self = this;
  var state = self._socketState;

  if (state.destroyed) {
    return;
  }

  if (state.writable) {
    self.end();
  }

  // unset timeout
  clearSocketTimeout(self);

  if (self._writableState.ended && self._handle) {
    close(self);
    state.destroyed = true;
  } else {
    self.once('finish', function() {
      self.destroy();
    });
  }
};


// Destroy this socket as fast as possible if this socket is no longer readable.
Socket.prototype.destroySoon = function() {
  var self = this;
  var state = self._socketState;

  if (state.writable) {
    self.end();
  }

  if (self._writableState.finished) {
    self.destroy();
  } else {
    self.once('finish', self.destroy);
  }
};


Socket.prototype.setKeepAlive = function(enable, delay) {
  var self = this;
  enable = +Boolean(enable);
  if (self._handle && self._handle.setKeepAlive) {
    self._handle.setKeepAlive(enable, ~~(delay / 1000));
  }
};


Socket.prototype.address = function() {
  if (!this._handle || !this._handle.getsockname) {
    return {};
  }
  if (!this._sockname) {
    var out = {};
    var err = this._handle.getsockname(out);
    if (err) return {}; // FIXME(bnoordhuis) Throw?
    this._sockname = out;
  }
  return this._sockname;
};


Socket.prototype.setTimeout = function(msecs, callback) {
  var self = this;

  self._timeout = msecs;
  clearSocketTimeout(self);

  if (msecs === 0) {
    if (callback) {
      self.removeListener('timeout', callback);
    }
  } else {
    self._timer = setTimeout(function() {
      self.emit('timeout');
      clearSocketTimeout(self);
    }, msecs);
    if (callback) {
      self.once('timeout', callback);
    }
  }
};


function connect(socket, ip, port) {
  var afterConnect = function(status) {
    var state = socket._socketState;
    state.connecting = false;

    if (state.destroyed) {
      return;
    }

    if (status == 0) {
      onSocketConnect(socket);
      socket.emit('connect');
    } else {
      socket.destroy();
      emitError(socket, new Error('connect failed - status: ' +
        Tcp.errname(status)));
    }
  };

  var err = socket._handle.connect(ip, port, afterConnect);
  if (err) {
    emitError(socket, new Error('connect failed - status: ' +
      Tcp.errname(err)));
  }
}


function close(socket) {
  socket._handle.owner = socket;
  socket._handle.onclose = function() {
    socket.emit('close');
  };

  var handle = socket._handle;
  socket._handle = null;
  handle.close();

  if (socket._server) {
    var server = socket._server;
    server._socketCount--;
    server._emitCloseIfDrained();
    socket._server = null;
  }
}


function resetSocketTimeout(socket) {
  var state = socket._socketState;

  if (!state.destroyed) {
    // start timeout over again
    clearSocketTimeout(socket);
    socket._timer = setTimeout(function() {
      socket.emit('timeout');
      clearSocketTimeout(socket);
    }, socket._timeout);
  }
}


function clearSocketTimeout(socket) {
  if (socket._timer) {
    clearTimeout(socket._timer);
    socket._timer = null;
  }
}


function emitError(socket, err) {
  socket.errored = true;
  stream.Duplex.prototype.end.call(socket, '', function() {
    socket.destroy();
  });
  socket._readyToWrite();
  socket.emit('error', err);
}


function maybeDestroy(socket) {
  var state = socket._socketState;

  if (!state.connecting &&
    !state.writable &&
    !state.readable) {
    socket.destroy();
  }
}


function onSocketConnect(socket) {
  var state = socket._socketState;

  state.connecting = false;
  state.connected = true;

  resetSocketTimeout(socket);

  socket._readyToWrite();

  // `readStart` on next tick, after connection event handled.
  process.nextTick(function() {
    socket._handle.owner = socket;
    socket._handle.onread = onread;
    socket._handle.readStart();
  });
}


function onread(socket, nread, isEOF, buffer) {
  var state = socket._socketState;

  resetSocketTimeout(socket);

  if (isEOF) {
    // pushing readable stream null means EOF.
    stream.Readable.prototype.push.call(socket, null);

    if (socket._readableState.length == 0) {
      // this socket is no longer readable.
      state.readable = false;
      // destroy if this socket is not writable.
      maybeDestroy(socket);
    }
  } else if (nread < 0) {
    var err = new Error('read error: ' + nread);
    stream.Readable.prototype.error.call(socket, err);
  } else if (nread > 0) {
    stream.Readable.prototype.push.call(socket, buffer);
  }
}


// Writable stream finished.
function onSocketFinish() {
  var self = this;
  return self.destroy();

  // if (!state.readable || self._readableState.ended || !self._handle) {
  //   // no readable stream or ended, destroy(close) socket.
  //   return self.destroy();
  // } else {
  //   // Readable stream alive, shutdown only outgoing stream.
  //   self._handle.shutdown(function() {
  //     if (self._readableState.ended) {
  //       self.destroy();
  //     }
  //   });
  // }
}


// Readable stream ended.
function onSocketEnd() {
  var state = this._socketState;

  maybeDestroy(this);

  if (!state.allowHalfOpen) {
    this.destroySoon();
  }
}


function normalizeConnectArgs(args) {
  var options = {};

  if (util.isObject(args[0])) {
    options = args[0];
  } else {
    options.port = args[0];
    if (util.isString(args[1])) {
      options.host = args[1];
    }
  }

  var cb = args[args.length - 1];

  return util.isFunction(cb) ? [options, cb] : [options];
}


// net.connect(options[, connectListener])
// net.connect(port[, host][, connectListener])
exports.connect = exports.createConnection = function() {
  var args = normalizeConnectArgs(arguments);
  var socket = new Socket(args[0]);
  return Socket.prototype.connect.apply(socket, args);
};


module.exports.Socket = Socket;
