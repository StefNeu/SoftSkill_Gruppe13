#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN1 2
#define NUM1 24

#define PIN2 0
#define NUM2 8

//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM1, PIN1, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUM2, PIN2, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  strip1.begin();
  strip1.setBrightness(50);
  strip1.clear(); // Initialize all pixels to 'off'
  strip2.begin();
  strip2.setBrightness(50);
  strip2.clear(); // Initialize all pixels to 'off'
}

void loop() {
  strip1.show();
  strip2.show();
}
