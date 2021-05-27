# Developer Notes

## Project goals

Goal is stable, steady vacuum at low cost.

## Software

The software allows hot-plugging the sensors. 

Plugging in a sensor is detected by setting the chip select pin to INPUT_PULLDOWN and reading the pin status. When a sensor is present chip select pin will be pulled high.

A removed sensor is detected by trying to read the chip id over spi, and finding the chip id has changed.

## Hardware

The electronics have been kept simple, so there is little that can fail. The board can be soldered by hand.

I've experimented plugging and unplugging sensor and display when connecting over i2c and spi. With spi, worst case is the sensor does not work. With i2c, worst case is the system hangs. Because I want long term stability and unattended functioning I used spi *and* configured a watchdog timer.

The sensors do not have a reset pin. If this is a problem for long-term stability, power the sensors through a low drop voltage regulator with the voltage regulator enable connected to a processor pin. This way the processor can power cycle the sensors if the sensors need a hard reset.

## Footswitch

The footswitch connector has one digital/analog and two digital contacts. 

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

Total savings: 11463 bytes flash

- Use font ``Veranda24ptDigits.h`` for oled, as open source version of. This is the font *Veranda24pt*, characters ``-./01234567890:`` only. This is sufficient to print positive and negative integers and floats, and the time. Veranda24ptDigits font size is 1223 bytes. Saves 7643 bytes flash compared to the full *Veranda24pt* font. The open source *Veranda24pt* font is similar to the commercial *Verdana24pt*.

- Disable support for unused Serial1 port. The serial port is only used for initial firmware upload. 
  *Tools -> U(S)ART support -> Disabled (no Serial support)*
  saves 2468 bytes flash

- Remove Adafruit_SSD1306 flash screen.
  This removes the splash screen bitmap from the SSD1306 driver. Saves 1352 bytes flash.
  [patch](arduino/libraries/Adafruit_SSD1306.patch)

