# vacuum-controller

[![screenshot](images/vacuum_pump_controller_with_sensor_small.jpg)](https://raw.githubusercontent.com/koendv/vacuum-controller/master/images/vacuum_pump_controller_with_sensor_big.jpg)

This is a controller for a vacuum pump, useful in pick-and-place machines. The controller adjusts the rpm of a vacuum pump to maintain a constant vacuum. The controller can be used over usb, as part of a pick-and-place machine, or standalone, with a footswitch.

## Features:

- up to 4 x BMP280 pressure sensors: 1 for outside pressure, 1 for vacuum pressure, and 2 for two pick-and-place nozzles. The pressure sensors on the nozzles are used to check whether a component has been picked up or fallen off. 

- up to 2 x 3V210-06-NC solenoid valve, one for each pick-and-place nozzle.

## Usage

Plug the controller in a usb port, and connect to the usb serial port. On linux, this is ``minicom -D /dev/ttyACM0``

Hit the reset button to see the boot message:

```
vacuum controller - type h for help
sensors ok  ok  -  -
ready
>
```
The ``sensors`` line indicates sensor status:

- ``ok`` sensor detected and running
- ``-`` no sensor detected
- ``?`` bmp280 sensor hangs, or sensor is wrong type (not bmp280)

All commands are a single character, optionally followed by a number, and terminated with *enter*.

#### Help

 ``h`` *help* prints a command summary:

```
>h
commands, ## = int, #.## = float:
? print status
s#.## setpoint
p#.## proportional gain
i#.## integral gain
d#.## derivative gain
m#.## manual mode
a autotune
l## logging on/off
v## valve on/off
w write settings
r reset
f firmware
h help
>
```

#### print status

```
>?
vacuum hPa: 99.97 motor: 73.60% mode: auto
setpoint hPa: 100.00 Kp: 150.00 Ki: 50.00 Kd: 0.00 logging: 0
pressure hPa: 938.86 838.89 939.41 0.00 
sensors ok  ok  ok  -
```
We have three sensors. The first sensor measures atmospheric pressure; the second sensor measures the pressure in the vacuum vat; the third measures the pressure at the nozzle. The vacuum is the difference between the first and the second sensor. All pressures are in hPa. Atmospheric pressure is around 1000 hPa.

*vacuum* is the measured value. *motor* is PWM, in percent. *mode* is *auto* if running the PID controller; *manual* if the motor PWM has been set using the ``m`` command.

The vacuum pump controller is a PID controller. *setpoint* is the 
desired value; *Kp*, *Ki* and *Kd* are controller proportional, integral and derivative gain. It is normal for *Kd* to be zero. If *logging* is non-zero, pressure is logged on the console every 0.1s.

*pressure* is the pressure from the four sensors, in hPa. If a sensor is not plugged in, the pressure is 0. Lastly, *sensors* is the sensor state. If a sensor says ``?``, check the cable, check the sensor is plugged in right, or power cycle the board.

#### Setpoint

The *setpoint* command sets the desired vacuum, in hPa.
```
>s100.0
ok
```

#### Proportional Gain

Sets the PID controller Kp.

#### Integral Gain

Sets the PID controller Ki.

#### Derivative Gain

Sets the PID controller Kd. Usually zero.

#### Logging

Switches logging on or off.

- ``l0`` logging off.
- ``l1`` logging on.

Use the ``w`` command if you want this setting to be saved.

When logging is on, every sample is printed on the console output, in one line:
```
        1743075 52417   93876   83764   93926   0       2067058
```
The fields are:
- time since boot in milliseconds
- motor PWM; a value from 0 to 65535.
- pressure 1, 2, 3, 4 in Pa.
- a checksum that is the 32 bit sum of the 6 previous numbers. 

The line begins with a tab character, and numbers are separated by a tab character. If you parse console output, check for lines that begin with a tab. 

#### Valve

Switches outputs BO1 and BO2 on/off. These outputs are normally connected to solenoid valves.

- ``v00`` switch BO1 off
- ``v01`` switch BO1 on
- ``v10`` switch BO2 off
- ``v11`` switch BO2 on

#### Manual

Sets the motor PWM manually. Value is a number from 0 to 100, inclusive.

- ``m100`` full speed
- ``m0`` stop
- ``m`` return to automatic mode

#### Autotune

*Autotune* runs the pump at full speed, and measures how pressure changes. From this measurement values for Kp, Ki and Kd are calculated. 

```
>a
autotune - press c to cancel
cycle 1/5
  0%    0 hPa
 11%    1 hPa
100%  131 hPa
  0%    0 hPa
100%   83 hPa
minpwm: 7141 maxpwm: 65535 minpressure: 0.49 maxpressure: 130.83
tau: 2.69s Kp: 149.34 Ki: 55.54
...
Kp: 149.60 Ki: 54.03 Kd: 0.00
enter w to save
```

After *autotune*, if the calculated settings seem correct, use ``w`` to store these settings in non-volatile memory.

Run *autotune* again if something has changed; e.g. a different vacuum pump or vacuum vessel.
 
#### Write 
The ``w`` write command saves Kp, Ki, Kd, setpoint, and logging to non-volatile memory. The saved values will be restored on power-up.

#### Reset
The ``r`` reset command reboots the controller.

#### Firmware
The ``f`` command prints the compilation date and ram usage.
```
>f
compiled May 24 2021
11312 bytes free
13 ms slowest loop
```

### Connections

Connect pressure sensors, vacuum pump, and solenoid valve.

#### Sensors

Up to 4 BMP280 boards can be connected in headers H1..H4.

- H1: air pressure
- H2: pressure inside vacuum vessel
- H3: optional, pressure at nozzle 1
- H4: optional, pressure at nozzle 2

The minimal configuration is sensors at H1 and H2.

#### Vacuum Pump
 Connect a 12V DC brushless vacuum pump. Maximum current of the TB6612 driver is 1A continuous.
 
 If the vacuum pump has two wires:
 
 - red (+) to AO1
 - black (-) to GND
 
 If the vacuum pump has four wires:
 
 - red (+) to 12V
 - black (-) to GND
 - PWM input to AO1
 - Tacho output does not need to be connected 

#### Solenoid valve

Connect the first solenoid valve:

 - red (+) to B01
 - black (-) to GND
 
If there is a second solenoid valve, connect:

 - red (+) to B02
 - black (-) to GND
 
### Footswitch

![](https://raw.githubusercontent.com/koendv/paste_dispenser/master/doc/footswitch.jpg)

The optional footswitch can be used to switch the first solenoid valve on and off.

The footswitch connects using a 3.5mm jack, as used in earphones. Connections are:

- Tip: NO Normally open
- Ring: NC Normally closed
- Shield: Common

The controller does switch debouncing.

## Building
*work in progress*
 
[oshw project page](https://oshwlab.com/koendv/vacuum-pump-controller)