#include "console.h"
#include "autotune.h"
#include "bytesfree.h"
#include "display.h"
#include "loopstats.h"
#include "motor.h"
#include "pidctrl.h"
#include "sensor.h"
#include "settings.h"
#include "uptime.h"
#include "watchdog.h"
#include <Arduino.h>

#define MAXCMDLEN 64

/* console command line interpreter.
 * A command is a character, optionally followed by an integer or floating-point number.
 * Status messages are:
 * "init"  - non-volatile storage reset to defaults
 * "ready" - running
 * "ok"    - command successful
 * "how?"  - illegal value
 * "what?" - syntax error
 *
 * To stop pressure being logged to the console, enter 'l0'.
 */

namespace console {

String cmdline;

void reset() {
  motor::speed(0);
  watchdog::reboot();
}

void printStatus() {

  Serial.print("vacuum hPa: ");
  Serial.print(sensor::vacuum);
  Serial.print(" motor: ");
  Serial.print(motor::pwma_percent);
  Serial.print("% mode: ");
  if (PIDctrl::isAuto())
    Serial.println("auto");
  else
    Serial.println("manual");

  Serial.print("setpoint hPa: ");
  Serial.print(settings::setpoint);
  Serial.print(" Kp: ");
  Serial.print(settings::Kp);
  Serial.print(" Ki: ");
  Serial.print(settings::Ki);
  Serial.print(" Kd: ");
  Serial.print(settings::Kd);
  Serial.print(" logging: ");
  Serial.println(settings::logging);

  Serial.print("pressure hPa: ");
  for (int i = 0; i < NUM_SENSOR; i++) {
    Serial.print(sensor::fpressure[i]);
    Serial.print(' ');
  }
  Serial.println();
  sensor::printSensors();
}

/*
   *  Execute command. Commands are a single character,
   *  optionally followed by a float or an int.
   */
void doCommand() {
  double fval = 0;
  int ival = 0;
  bool ok = true;

  Serial.println();
  cmdline.trim();
  cmdline.toLowerCase();

  // check numeric argument
  if (cmdline.length() >= 2) {
    if (isDigit(cmdline[1])) {
      fval = cmdline.substring(1).toDouble();
      ival = cmdline.substring(1).toInt();
    } else
      cmdline[0] = ':'; // cause syntax error
  }

  // execute command
  if (cmdline.length() != 0) {
    switch (cmdline[0]) {
    case 'p': // set proportional gain
      settings::Kp = fval;
      PIDctrl::reload();
      break;
    case 'i': // set integral gain
      settings::Ki = fval;
      PIDctrl::reload();
      break;
    case 'd': // set derivative gain
      settings::Kd = fval;
      PIDctrl::reload();
      break;
    case 's': // set setpoint
      settings::setpoint = fval;
      PIDctrl::reload();
      break;
    case 'm': // manual mode
      if (cmdline.length() == 1)
        PIDctrl::automatic();
      else if ((fval >= 0) && (fval <= 100))
        PIDctrl::manual(fval / 100.0 * MAXPWM);
      else
        Serial.println("how?");
      break;
    case 'a': // tune
      autotune::tune();
      break;
    case 'l': // console pressure logging on/off
      settings::logging = (ival != 0);
      break;
    case 'v': // switch output pin on/off
      switch (ival) {
      case 00:
        motor::setswitch(2, false); // v00: TB6612 pin B01 off
        break;
      case 01:
        motor::setswitch(2, true); // v01: TB6612 pin B01 on
        break;
      case 10:
        motor::setswitch(3, false); // v10: TB6612 pin B02 off
        break;
      case 11:
        motor::setswitch(3, true); // v11: TB6612 pin B02 on
        break;
      default:
        Serial.println("how?");
        ok = false;
        break;
      }
      break;
    case 'w': // write settings to non-volatile
      settings::write();
      break;
    case 'r': // reset sensors and pid controller
      reset();
      ok = false;
      break;
    case '?': // print all variables and settings
      printStatus();
      ok = false;
      break;
    case 'f':
      Serial.println("compiled " __DATE__); // print firmware version. helpful for support.
      uptime::print();                      // print system uptime
      Serial.print(bytes_free());           // print free ram. helpful for detecting memory leaks.
      Serial.println(" bytes free");
      Serial.print(loopstats::slowest_loop()); // time of slowest loop(), in ms
      Serial.println(" ms / loop");
      ok = false;
      break;
    case 'h': // help
      Serial.println("commands, ## = int, #.## = float:");
      Serial.println("? print status");
      Serial.println("s#.## setpoint");
      Serial.println("p#.## proportional gain");
      Serial.println("i#.## integral gain");
      Serial.println("d#.## derivative gain");
      Serial.println("m#.## manual mode");
      Serial.println("a autotune");
      Serial.println("l## logging on/off");
      Serial.println("v## valve on/off");
      Serial.println("w write settings");
      Serial.println("r reset");
      Serial.println("f firmware");
      Serial.println("h help");
      ok = false;
      break;
    default:
      Serial.println("what?");
      ok = false;
      break;
    }
    if (ok)
      Serial.println("ok");
  }
  cmdline = "";
  Serial.print(">");
  return;
}

void setup() {
  cmdline.reserve(MAXCMDLEN);
  Serial.begin(115200);
  // wait up to 1 second for usb serial to connect
  for (int i = 0; i < 100; i++)
    if (Serial)
      break;
    else
      delay(10);
  Serial.println();
  Serial.println("vacuum controller - type h for help");
}

void loop() {
  while (Serial.available()) {
    char ch = Serial.read();
    switch (ch) {
    case '\b':
      if (cmdline.length() > 0) {
        cmdline.remove(cmdline.length() - 1);
        Serial.print("\b \b");
      }
      break;
    case '\r':
    case '\n':
      doCommand();
      break;
    default:
      if (cmdline.length() != MAXCMDLEN)
        cmdline += ch;
      Serial.print(ch);
      break;
    }
  }
}
} // namespace console

// not truncated
