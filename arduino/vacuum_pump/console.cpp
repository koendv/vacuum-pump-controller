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
 * "ok"    - command successful
 * "how?"  - illegal value
 * "what?" - syntax error
 *
 * To stop pressure being logged to the console, enter 'l0'.
 */

namespace console {

enum err_code { ERR_OK,
                ERR_SYNTAX,
                ERR_VALUE };
enum err_code ok = ERR_OK;
bool echo_on = true;

String cmdline;

void reset() {
  Serial.println("ok");
  Serial.flush();
  motor::speed(0);
  motor::setswitch(2, false);
  motor::setswitch(3, false);
  watchdog::reboot();
}

// m-codes, for easy compatibility with openpnp.org

// print sensor s pressure in hPa as m-code response
void m_pressure(int m, int s, bool relative) {
  int32_t p;
  sensor::readSensors();
  p = sensor::ipressure[s]; // pressure in Pa
  if (relative) {
    // vacuum can be negative due to small measurement errors,
    // or if the sensor for measuring atmospheric pressure is missing.
    p = sensor::ipressure[0] - p; // relative vacuum
  }
  p = p / 100; // convert from Pa to hPa
  Serial.print("[read:");
  Serial.print(p);
  Serial.println(']');
}

// parse m-code
void m_code(int m) {
  switch (m) {
  case 115:
    Serial.println("FIRMWARE_NAME:vacuum pump");
    break;
  case 800: // M800 switch vacuum pump on
    PIDctrl::automatic();
    break;
  case 801: // M801 switch vacuum pump off
    PIDctrl::manual(0);
    break;
  case 802: // M802 switch nozzle 1 vacuum solenoid on
    motor::setswitch(2, true);
    break;
  case 803: // M803 switch nozzle 1 vacuum solenoid off
    motor::setswitch(2, false);
    break;
  case 804: // M804 switch nozzle 2 vacuum solenoid on
    motor::setswitch(3, true);
    break;
  case 805: // M805 switch nozzle 2 vacuum solenoid off
    motor::setswitch(3, false);
    break;
  case 900: // M900 read absolute pressure sensor0 (air)
    m_pressure(m, 0, false);
    break;
  case 901: // M901 read absolute pressure sensor1 (pump)
    m_pressure(m, 1, false);
    break;
  case 902: // M902 read absolute pressure sensor2 (nozzle1)
    m_pressure(m, 2, false);
    break;
  case 903: // M903 read absolute pressure sensor3 (nozzle2)
    m_pressure(m, 3, false);
    break;
  case 911: // M911 read relative vacuum sensor1 (pump)
    m_pressure(m, 1, true);
    break;
  case 912: // M912 read relative vacuum sensor2 (nozzle1)
    m_pressure(m, 2, true);
    break;
  case 913: // M913 read relative vacuum sensor3 (nozzle2)
    m_pressure(m, 3, true);
    break;
  default:
    ok = ERR_SYNTAX;
    break;
  }
  return;
}

void printHelp() {
  Serial.println("commands, ## = int, #.## = float:");
  Serial.println("? print status");
  Serial.println("s#.## setpoint");
  Serial.println("p#.## proportional gain");
  Serial.println("i#.## integral gain");
  Serial.println("d#.## derivative gain");
  Serial.println("o#.## manual mode");
  Serial.println("a autotune");
  Serial.println("l## logging on/off");
  Serial.println("m## m-code");
  Serial.println("v## valve on/off");
  Serial.println("w write settings");
  Serial.println("r reset");
  Serial.println("f firmware");
  Serial.println("h help");
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

// print useful firmware info
void printFirmware() {
  Serial.println("compiled " __DATE__); // print firmware version. helpful for support.
  uptime::print();                      // print system uptime
  Serial.print(bytes_free());           // print free ram. helpful for detecting memory leaks.
  Serial.println(" bytes free");
  Serial.print(loopstats::slowest_loop()); // time of slowest loop(), in ms
  Serial.println(" ms slowest loop");
}

void setValve(int i) {
  switch (i) {
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
    ok = ERR_VALUE;
    break;
  }
}

/*
   *  Execute command. Commands are a single character,
   *  optionally followed by a float or an int.
   */
void doCommand() {
  double fval = 0;
  int ival = 0;
  ok = ERR_OK;
  cmdline.trim();
  // check numeric argument
  if (cmdline.length() >= 2) {
    if (isDigit(cmdline[1])) {
      fval = cmdline.substring(1).toDouble();
      ival = cmdline.substring(1).toInt();
    } else
      cmdline[0] = ':'; // cause syntax error
  }

  // execute command
  if (echo_on)
    Serial.println();
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
    case 'o': // output manual mode
      if (cmdline.length() == 1)
        PIDctrl::automatic();
      else if ((fval >= 0) && (fval <= 100))
        PIDctrl::manual(fval / 100.0 * MAXPWM);
      else
        ok = ERR_VALUE;
      break;
    case 'a': // tune
      autotune::tune();
      loopstats::slowest_loop(); // reset cpu stats
      break;
    case 'l': // console pressure logging on/off
      settings::logging = (ival != 0);
      break;
    case 'v': // switch output pin on/off
      setValve(ival);
      break;
    case 'm': // openpnp.org m-codes
      m_code(ival);
      break;
    case 'w': // write settings to non-volatile
      settings::write();
      break;
    case 'r':  // reset sensors and pid controller
      reset(); // does not return
      break;
    case '?': // print all variables and settings
      printStatus();
      break;
    case 'f':
      printFirmware();
      break;
    case 'h': // help
      printHelp();
      break;
    default:
      ok = ERR_SYNTAX;
      break;
    }
  }
  switch (ok) {
  case ERR_OK:
    Serial.println("ok");
    break;
  case ERR_SYNTAX:
    Serial.println("what?");
    Serial.println("ko");
    break;
  case ERR_VALUE:
    Serial.println("how?");
    Serial.println("ko");
    break;
  default:
    break;
  }
  cmdline = "";
  if (echo_on)
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
  Serial.println("vacuum pump - type h for help");
}

void loop() {
  while (Serial.available()) {
    char ch = Serial.read();
    if (isUpperCase(ch))
      ch = ch - 'A' + 'a';
    switch (ch) {
    case '\b':
      if (cmdline.length() > 0) {
        cmdline.remove(cmdline.length() - 1);
        if (echo_on)
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
      if (ch == 'm')
        echo_on = false; // don't echo m-code
      else if (!isDigit(ch))
        echo_on = true; // echo interactive command
      if (echo_on)
        Serial.print(ch);
      break;
    }
  }
}
} // namespace console

// not truncated
