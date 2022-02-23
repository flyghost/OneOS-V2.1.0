
# GPIO

The GPIO module provides access the General Purpose Input Output pins of the hardware.

Each pin has a logical number starting from `0`.


### DIRECTION
* `IN` Input pin.
* `OUT` Output pin.

An enumeration which can be used to specify the direction of the pin.


### MODE
* `NONE` None.
* `PULLUP` Pull-up (pin direction must be [`IN`](#direction)).
* `PULLDOWN` Pull-down (pin direction must be [`IN`](#direction)).
* `PUSHPULL` Push-pull (pin direction must be [`OUT`](#direction)).
* `OPENDRAIN` Open drain (pin direction must be [`OUT`](#direction)).

An enumeration which can be used to specify the


### EDGE
* `NONE` None.
* `RISING` Rising.
* `FALLING` Falling.
* `BOTH` Both.

An enumeration which can be used to specify the
edge of the pin.


### gpio.open(configuration, callback)
* `configuration` {Object} Configuration for open GPIOPin.
  * `pin` {number} Pin number. Mandatory field.
  * `direction` {[gpio.DIRECTION](#direction)} Pin direction. **Default:** `gpio.DIRECTION.OUT`
  * `mode` {[gpio.MODE](#mode)} Pin mode. **Default:** `gpio.MODE.NONE`
  * `edge` {[gpio.EDGE](#edge)} Pin edge. **Default:** `gpio.EDGE.NONE`
* `callback` {Function}
  * `error` {Error|null}
  * `gpioPin` {Object} An instance of GPIOPin.
* Returns: {Object} An instance of GPIOPin.

Get GPIOPin object with configuration asynchronously.

The `callback` function will be called after
opening is completed. The `error` argument is an
`Error` object on failure or `null` otherwise.

**Example**

```js
var gpio = require('gpio');

var gpio10 = gpio.open({
  pin: 10,
  direction: gpio.DIRECTION.OUT,
  mode: gpio.MODE.PUSHPULL,
  edge: gpio.EDGE.RISING
}, function(err, pin) {
  if (err) {
    throw err;
  }
});
```

### gpio.openSync(configuration)
* `configuration` {Object} Configuration for open GPIOPin.
  * `pin` {number} Pin number. Mandatory field.
  * `direction` {[gpio.DIRECTION](#direction)} Pin direction. **Default:** `gpio.DIRECTION.OUT`
  * `mode` {[gpio.MODE](#mode)} Pin mode. **Default:** `gpio.MODE.NONE`
  * `edge` {[gpio.EDGE](#edge)} Pin edge. **Default:** `gpio.EDGE.NONE`
* Returns: {Object} An instance of GPIOPin.

Get GPIOPin object with configuration synchronously.

**Example**

```js
var gpio = require('gpio');

var gpio10 = gpio.openSync({
  pin: 10,
  direction: gpio.DIRECTION.IN,
  mode: gpio.MODE.PULLUP
});
```


## Class: GPIOPin

This class represents an opened and configured GPIO pin.
It allows getting and setting the status of the pin.

### gpiopin.setDirectionSync(direction)
  * `direction` {[gpio.DIRECTION](#direction)} Pin direction.

Set the direction of a GPIO pin.

**Example**

```js
gpio10.setDirectionSync(gpio.DIRECTION.OUT);
gpio10.writeSync(1);

gpio10.setDirectionSync(gpio.DIRECTION.IN);
var value = gpio10.readSync();
```

### gpiopin.write(value[, callback])
* `value` {number|boolean}
* `callback` {Function}
  * `error` {Error|null}

Asynchronously writes out a boolean `value` to a GPIO pin
(a number `value` is converted to boolean first).

The optional `callback` function will be called
after the write is completed. The `error` argument
is an `Error` object on failure or `null` otherwise.

**Example**

```js
gpio10.write(1, function(err) {
  if (err) {
    throw err;
  }
});
```


### gpiopin.writeSync(value)
* `value` {number|boolean}

Writes out a boolean `value` to a GPIO pin synchronously
(a number `value` is converted to boolean first).

**Example**

```js
gpio10.writeSync(1);
```


### gpiopin.read([callback])
* `callback` {Function}
  * `error` {Error|null}
  * `value` {boolean}

Asynchronously reads a boolean value from a GPIO pin.

The optional `callback` function will be called
after the read is completed. The `error` argument
is an `Error` object on failure or `null` otherwise.
The `value` argument contains the boolean value
of the pin.

**Example**

```js
gpio10.read(function(err, value) {
  if (err) {
    throw err;
  }
  console.log('value:', value);
});
```


### gpiopin.readSync()
* Returns: {boolean}

Returns the boolean value of a GPIO pin.

**Example**

```js
console.log('value:', gpio10.readSync());
```


### gpiopin.close([callback])
* `callback` {Function}
  * `error` {Error|null}

Asynchronously closes a GPIO pin.

The optional `callback` function will be called
after the close is completed. The `error` argument
is an `Error` object on failure or `null` otherwise.

**Example**

```js
gpio10.close(function(err) {
  if (err) {
    throw err;
  }

  // prints: gpio pin is closed
  console.log('gpio pin is closed');
});
```


### gpiopin.closeSync()

Synchronously closes a GPIO pin.

**Example**

```js
gpio10.closeSync();

// prints: gpio pin is closed
console.log('gpio pin is closed');
```
