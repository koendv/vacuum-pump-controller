# Developer Notes

## Project goals

Goal is stable, steady vacuum at low cost.

## Software

The software allows hot-plugging the sensors. 

Plugging in a sensor is detected by setting the chip select pin to INPUT_PULLDOWN and reading the pin status. When a sensor is present chip select pin will be pulled high.

A removed sensor is detected by trying to read the chip id over spi, and finding the chip id has changed.

The Adafruit Unified Sensor interface was not used because using getPressure() directly has more precision, and more precision gives smoother PID control.

The software is easily adapted to other BMPxxx sensors.

## Hardware

The electronics have been kept simple, so there is little that can fail. The board can be soldered by hand.

I've experimented plugging and unplugging sensor and display when connecting over i2c and spi. With spi, worst case is the sensor does not work. With i2c, worst case is the system hangs. Because I want long term stability and unattended functioning I used spi *and* configured a watchdog timer.

The sensors do not have a reset pin. If long-term stability is a problem, power the sensors through a AP2112K-3.3 low drop voltage regulator with the voltage regulator enable connected to a processor pin. This way the processor can power cycle the sensors if the sensors need a hard reset.

## Footswitch

The footswitch connector has one digital/analog and two digital contacts. The controller works with a SPDT footswitch.

If you wish to connect an analogue footswitch (like those used on sewing machines) there is source in [footswitch.cpp.analog](arduino/vacuumcontroller/attic/footswitch.cpp.analog)

## Autotuning

Autotuning sets the PID controller gain.

The algorithm used is the following:

- Find the lowest voltage where the pump begins to turn.
Beginning with pressure 0 and pwm 0, slowly increase pwm until pressure increases. Note this data as (minpwm, minpressure).
- Find the steady-state pressure.
Set the motor to full 12V (100% PWM), and wait until pressure no longer increases. Note this data as (maxpwm, maxpressure).
- Draw a line through the two points (minpwm, minpressure) and (maxpwm, maxpressure) and calculate the slope. The PID controller proportional gain is set to the slope, divided by a safety factor 3:

<img src="https://render.githubusercontent.com/render/math?math=K_{p}= \frac{1}{3}\cdot\frac{maxpwm-minpwm}{maxpressure-minpressure}">

- An exponentially dampened process <img src="https://render.githubusercontent.com/render/math?math=y(t)=1-e^{-\frac{t}{\tau}}">
reaches value 0.63 at t = &tau;. Beginning with pressure 0 and pwm 0, set the vacuum pump at full 12V (100% PWM), and measure the time for pressure to reach 63% of steady-state pressure:
<img src="https://render.githubusercontent.com/render/math?math=pressure=minpressure\%2b{0.63}\cdot(maxpressure-minpressure)">
This time measurement gives the time constant &tau;. The PID controller integral gain is set at
 
<img src="https://render.githubusercontent.com/render/math?math=K_{i}= \frac{K_{p}}{\tau}">

- The PID controller differential gain is set to zero:

<img src="https://render.githubusercontent.com/render/math?math=K_{d}= 0">

The safety factor can be tuned a little to trade off stability for accuracy, if needed.

## Ways to save flash

Simple measures to save flash.

- The sketch uses a 24pt bitmap font to display the vacuum on an oled display. Fonts use a lot of flash memory, especially at larger point sizes. Optimize by storing only those characters actually needed. The font ``Veranda24ptDigits.h`` only contains the characters ``-./01234567890:``, sufficient to print positive and negative integers and floats and the time. Font size is 1223 bytes, a saving of 7643 bytes compared to the full font.

- Disable support for unused Serial1 port. The serial port is only used for initial firmware upload. 
  *Tools -> U(S)ART support -> Disabled (no Serial support)*
  saves 2468 bytes flash

- Remove Adafruit_SSD1306 flash screen.
  This removes the splash screen bitmap from the SSD1306 driver. Saves 1352 bytes flash.
  [patch](arduino/libraries/Adafruit_SSD1306.patch)

- Set "Optimize: Smallest (-Os) with LTO"
  [Link Time Optimization](https://gcc.gnu.org/wiki/LinkTimeOptimization) (LTO) saves 11828 bytes flash. But sometimes LTO drops too much code, so this option requires careful checking the program still works after LTO.

If LTO works, these measures save 23291 bytes flash, enough for the code to fit in 64K. (size: 63756 bytes). Personally, I think it's safer to buy a STM32F013 with 128 kbyte flash, and avoid use of LTO until flash is full.