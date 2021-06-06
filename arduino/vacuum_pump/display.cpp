#include "display.h"
#include "Fonts/Veranda24ptDigits.h"
#include "motor.h"
#include "sensor.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

/*
 * Display pressure and motor pwm on an 128x64 oled with ssd1306 controller.
 */

// spi hardware connection
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_DC PB1
#define OLED_CS PA4
#define OLED_RESET PB0

namespace display {

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);
int indent_negative_number = 0; // width of '0' - width of '-', 13 in Veranda24pt
uint32_t last_millis = 0;       // timestamp display update
int last_vacuum = -1;           // values at last display update
int last_pwm_bargraph = -1;
bool last_blink = false;
sensor::state_enum last_state[NUM_SENSOR] = {sensor::NO_SENSOR};

// calculate how much to indent negative numbers so the digits of positive
// and negative numbers align.

void inline calcIndent() {
  oled.setCursor(0, 0);
  oled.print('0');
  indent_negative_number = oled.getCursorX();
  oled.setCursor(0, 0);
  oled.print('-');
  indent_negative_number -= oled.getCursorX();
  if (indent_negative_number < 0)
    indent_negative_number = 0;
}

// print number i right-aligned on pixel row y.
void drawRightInt(int32_t i, uint8_t y) {
  if (i >= 0)
    oled.setCursor(0, y);
  else
    oled.setCursor(indent_negative_number, y);
  // right align number
  oled.setTextColor(SSD1306_BLACK);
  if ((i < 1000) && (i > -100))
    oled.print('0');
  if ((i < 100) && (i > -10))
    oled.print('0');
  if ((i < 10) && (i > -1))
    oled.print('0');
  oled.setTextColor(SSD1306_WHITE);
  oled.print(i);
}

void setup() {
  if (oled.begin(SSD1306_SWITCHCAPVCC)) {
    oled.clearDisplay();
    oled.setFont(&Veranda24ptDigits);
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    calcIndent();
    oled.clearDisplay();
    oled.display();
  } else
    Serial.println("oled?");
  return;
}

void reset() {
  setup();
}

// print display on console
// copy and paste output into .pbm file and read using gimp
void printScreen() {
  int16_t w = oled.width();
  int16_t h = oled.height();
  Serial.println('#');
  Serial.println("P1");
  Serial.print(w);
  Serial.print(' ');
  Serial.println(h);
  for (int16_t y = 0; y < h; y++) {
    for (int16_t x = 0; x < w; x++)
      Serial.print(oled.getPixel(x, y));
    Serial.println();
  }
  Serial.println('#');
}

void loop() {
  int vacuum = sensor::vacuum;
  int pwm_bargraph = SCREEN_WIDTH * motor::pwma / MAXPWM;
  uint32_t now = millis();
  bool blink = now & (1 << 12);
  bool display_changed = (vacuum != last_vacuum) || (pwm_bargraph != last_pwm_bargraph) || (blink != last_blink);
  for (int i = 0; i < NUM_SENSOR; i++) {
    display_changed |= last_state[i] != sensor::state[i];
  }
  // check if display needs updating
  if (((now < last_millis) || (now - last_millis > 500)) && display_changed) {
    // bar graph of motor pwm
    oled.clearDisplay();
    oled.fillRect(0, 0, pwm_bargraph, 8, SSD1306_WHITE);
    // sensor state
    for (int i = 0; i < NUM_SENSOR; i++) {
      // on a yellow/blue 128x64 oled, the first 16 rows are yellow, the last 48 blue
      const int16_t Y_YELLOW = 11;
      const int16_t Y_BLUE = 16;
      const int16_t bar_w = 20;
      const int16_t bar_h = 4;
      int16_t x = 30 * i + 5;

      switch (sensor::state[i]) {
      case sensor::NO_SENSOR:
        break;
      case sensor::SENSOR_OK:
        oled.fillRect(x, Y_BLUE, bar_w, bar_h, SSD1306_WHITE);
        break;
      case sensor::SENSOR_ERR:
        oled.fillRect(x, Y_YELLOW, bar_w, bar_h, SSD1306_WHITE);
        break;
      default:
        break;
      }
    }
    // print vacuum in hPa
    drawRightInt(vacuum, SCREEN_HEIGHT - 2); // print number right aligned
    // alive indicator
    oled.fillRect(SCREEN_WIDTH-4, SCREEN_HEIGHT-5, 4, 4, blink ? SSD1306_BLACK : SSD1306_WHITE);
    // update display
    oled.display();
    // update status
    last_millis = now;
    last_vacuum = vacuum;
    last_pwm_bargraph = pwm_bargraph;
    last_blink = blink;
    for (int i = 0; i < NUM_SENSOR; i++)
      last_state[i] = sensor::state[i];
  }
}
} // namespace display
// not truncated
