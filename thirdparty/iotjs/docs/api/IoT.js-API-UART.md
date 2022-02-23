## Class: UART

The UART (Universal Asynchronous Receiver/Transmitter) class supports asynchronous serial communication.

### uart.open(configuration, callback)
* `configuration` {Object}
  * `device` {string} Mandatory configuration. The specified device path.
  * `baudRate` {number} Specifies how fast data is sent over a serial line. **Default:** `9600`.
  * `dataBits` {number} Number of data bits that are being transmitted. **Default:** `8`.
* `callback` {Function}.
  * `err` {Error|null}.
* Returns: {UARTPort}.

Opens an UARTPort object with the specified configuration.

The `baudRate` must be equal to one of these values: [2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400].

The `dataBits` must be equal to one of these values: [5, 6, 7, 8].

[^1]: On stm32, The valid value of `databits` is only 8

**Example**

```js
var uart = require('uart');

var configuration = {
  device: "/dev/uart6",
  baudRate: 115200,
  dataBits: 8,
}

var serial = uart.open(configuration, function(err) {
  // Do something.
});

serial.closeSync();

```

### uart.openSync(configuration)
* `configuration` {Object}
  * `device` {string} Mandatory configuration. The specified device path.
  * `baudRate` {number} Specifies how fast data is sent over a serial line. **Default:** `9600`.
  * `dataBits` {number} Number of data bits that are being transmitted. **Default:** `8`.
* Returns: {UARTPort}.

Opens an UARTPort object with the specified configuration.

**Example**

```js
var uart = require('uart');

var configuration = {
  device: "/dev/uart6",
  baudRate: 115200,
  dataBits: 8,
}

var serial = uart.openSync(configuration);

serial.closeSync();

```

## Class: UARTPort
The UARTPort class is responsible for transmitting and receiving serial data.

### uartport.write(data, callback).
* `data` {string}.
* `callback` {Function}.
  * `err` {Error|null}.

Writes the given `data` to the UART device asynchronously.

**Example**

```js
var serial = uart.openSync({device: '/dev/ttyUSB0'});

serial.write('Hello?', function(err) {
  if (err) {
    // Do something.
  }
  serial.closeSync();
});

```

### uartport.writeSync(data)
* `data` {string}.

Writes the given `data` to the UART device synchronously.

**Example**

```js
var serial = uart.openSync({device: '/dev/ttyUSB0'});
serial.writeSync('Hello?');
serial.closeSync();

```

### uartport.close([callback])
* `callback` {Function}.
  * `err` {Error|null)}.

Closes the UART device asynchronously.

### uartport.closeSync()

Closes the UART device synchronously.

### Event: 'data'
* `callback` {Function}
  * `data` {Buffer} A data from the sender.

**Example**

```js

/* ... */

serial.on('data', function(data) {
  console.log('read result: ' + data.toString());
});

```
