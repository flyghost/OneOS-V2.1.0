
## Class: PWM

### pwm.open(configuration, callback)
* `configuration` {Object} Configuration object which can have the following properties.
  * `device` {string} The PWM device path.
  * `period` {number} The period of the PWM signal, in seconds (positive number).
  * `dutyCycle` {number} The active time of the PWM signal, must be within the `0.0` and `1.0` range.
* `callback` {Function} Callback function.
  * `err` {Error|null} The error object or `null` if there were no error.
  * `pwmpin` {Object} An instance of PWMPin.
* Returns: {Object} An instance of PWMPin.

Opens PWM pin with the specified configuration.


**Example**
```js
var pwm = require('pwm');
var config = {
  device: 'pwm_tim2',
  period: 0.1,
  dutyCycle: 0.5
}
var pwm0 = pwm.open(config, function(err) {
  if (err) {
    throw err;
  }
});
```

### pwm.openSync(configuration)
* `configuration` {Object} Configuration object which can have the following properties.
  * `device` {string} The PWM device path.
  * `period` {number} The period of the PWM signal, in seconds (positive number).
  * `dutyCycle` {number} The active time of the PWM signal, must be within the `0.0` and `1.0` range.
* Returns: {Object} An instance of PWMPin.

Opens PWM pin with the specified configuration.


**Example**
```js
var pwm = require('pwm');
var config = {
  device: 'pwm_tim2',
  period: 0.1,
  dutyCycle: 0.5
}
var pwm0 = pwm.openSync(config);
```


## Class: PWMPin

### pwmpin.setPeriod(period[, callback])
* `period` {number} The period of the PWM signal, in seconds (positive number).
* `callback` {Function}
  * `err` {Error|null} The error object or `null` if there were no error.

The `setPeriod` method allows the configuration of the period of the PWM signal in seconds.
The `period` argument must specified and it should be a positive number.

Configuration is done asynchronously and the `callback` method is invoked after the
period is configured to the new value or if an error occured.

**Example**
```js
pwm0.setPeriod(1, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
// prints: do
console.log('do');
// prints: done
```


### pwmpin.setPeriodSync(period)
* `period` {number} The period of the PWM signal, in seconds (positive number).

The `setPeriodSync` method allows the configuration of the period of the PWM signal in seconds.
The `period` argument must specified and it should be a positive number.

Configuration is done synchronously and will block till the period is configured.

**Example**
```js
pwm0.setPeriodSync(1);
// prints: done
console.log('done');
```


### pwmpin.setFrequency(frequency[, callback])
* `frequency` {integer} In Hz (positive integer).
* `callback` {Function}
  * `err` {Error|null} The error object or `null` if there were no error.

The `setFrequency` method configures the frequency of the PWM signal.
`frequency` is the measurement of how often the signal is repeated in a single period.

Configuration is done asynchronously and the `callback` method is invoked after the
frequency is configured to the new value or if an error occured.

**Example**
```js
pwm0.setFrequency(1, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
// prints: do
console.log('do');
// prints: done
```


### pwmpin.setFrequencySync(frequency)
* `frequency` {integer} In Hz (positive integer).

The `setFrequencySync` method configures the frequency of the PWM signal.
`frequency` is the measurement of how often the signal is repeated in a single period.

Configuration is done synchronously and will block till the frequency is configured.

**Example**
```js
pwm0.setFrequencySync(1);
// prints: done
console.log('done');
```


### pwmpin.setDutyCycle(dutyCycle[, callback])
* `dutyCycle` {number} Must be within the `0.0` and `1.0` range.
* `callback` {Function}
  * `err` {Error|null} The error object or `null` if there were no error.

The `setDutyCycle` method allows the configuration of the duty cycle of the PWM signal.
The duty cycle is the ratio of the pulse width in one period.

Configuration is done asynchronously and the `callback` method is invoked after the
duty cycle is configured to the new value or if an error occured.

**Example**
```js
pwm0.setDutyCycle(1, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
// prints: do
console.log('do');
// prints: done
```


### pwmpin.setDutyCycleSync(dutyCycle)
* `dutyCycle` {number} Must be within the `0.0` and `1.0` range.

The `setDutyCycleSync` method allows the configuration of the duty cycle of the PWM signal.
The duty cycle is the ratio of the pulse width in one period.

Configuration is done synchronously and will block till the duty cycle is configured.

**Example**
```js
pwm0.setDutyCycleSync(1);
// prints: done
console.log('done');
```


### pwmpin.setEnable(enable[, callback])
* `enable` {boolean}
* `callback` {Function}
  * `err` {Error|null} The error object or `null` if there were no error.

The `setEnable` method can turn on/off the generation of the PWM signal.
If the `enable` argument is `true` then the PWN signal generation is turned on.
Otherwise the signal generation is turned off.

After enabling/disabling the signal generation the `callback` is invoked.

**Example**
```js
pwm0.setEnable(true, function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
// prints: do
console.log('do');
// prints: done
```


### pwmpin.setEnableSync(enable)
* `enable` {boolean}

The `setEnableSync` method can turn on/off the generation of the PWM signal.
If the `enable` argument is `true` then the PWN signal generation is turned on.
Otherwise the signal generation is turned off.

**Example**
```js
pwm0.setEnableSync(false);
// prints: done
console.log('done');
```


### pwmpin.close([callback])
* `callback` {Function}
  * `err` {Error|null} The error object or `null` if there were no error.

The `close` method closes the PWM pin asynchronously.

The `callback` method will be invoked after the PWM device is released.

**Example**
```js
pwm0.close(function(err) {
  if (err) {
    throw err;
  }
  console.log('done');
});
// prints: do
console.log('do');
// prints: done
```


### pwmpin.closeSync()

The `closeSync` method closes the PWM pin synchronously.

**Example**
```js
pwm0.closeSync();
// prints: done
console.log('done');
```
