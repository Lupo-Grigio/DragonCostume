//Digital Rain implementation
//fastled 16x16 matrix demo
//Yaroslaw Turbin 24.08.2020
//https://vk.com/ldirko
//https://www.reddit.com/user/ldirko/

#include "FastLED.h"

// Matrix size
#define NUM_ROWS 14
#define NUM_COLS 33
// LEDs pin
#define DATA_PIN 3
// LED brightness
#define BRIGHTNESS 255
#define NUM_LEDS NUM_ROWS * NUM_COLS
// Define the array of leds
CRGB leds[NUM_LEDS];
byte rain[NUM_LEDS];
byte counter = 1;
int speed = 1;

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  raininit();
}

void loop() {
  EVERY_N_MILLISECONDS(100) {
    updaterain();
    FastLED.show();
  }
  EVERY_N_MILLISECONDS(30) {
    changepattern();
  }
} //loop

void changepattern () {
  int rand1 = random16 (NUM_LEDS);
  int rand2 = random16 (NUM_LEDS);
  if ((rain[rand1] == 1) && (rain[rand2] == 0) )   //simple get two random dot 1 and 0 and swap it,
  {
    rain[rand1] = 0;  //this will not change total number of dots
    rain[rand2] = 1;
  }
} //changepattern

void raininit() {                               //init array of dots. run once
  for (int i = 0; i < NUM_LEDS; i++) {
    if (random8(20) == 0) {
      rain[i] = 1;  //random8(20) number of dots. decrease for more dots
    }
    else {
      rain[i] = 0;
    }
  }
} //raininit

void updaterain() {
  for (byte i = 0; i < NUM_COLS; i++) {
    for (byte j = 0; j < NUM_ROWS; j++) {
      byte layer = rain[XY(i, ((j + speed + random8(2) + NUM_ROWS) % NUM_ROWS))];   //fake scroll based on shift coordinate
      // random8(2) add glitchy look
      if (layer) {
        leds[XY((NUM_COLS - 1) - i, (NUM_ROWS - 1) - j)] = CHSV(110, 255, BRIGHTNESS);
      }
    }
  }

  speed ++;
  fadeToBlackBy(leds, NUM_LEDS, 40);
  blurRows(leds, NUM_COLS, NUM_ROWS, 16);      //if you want
} //updaterain

uint16_t XY (uint8_t x, uint8_t y) {
  return (y * NUM_COLS + x);
}
