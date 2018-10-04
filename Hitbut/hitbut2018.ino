#include <TinyWireS.h>
#include <Adafruit_NeoPixel.h>
#include <ADCTouch.h>

#define I2C_SLAVE_ADDRESS 0x01
#define NUM_LEDS 7
#define PIXPIN 4
#define EYE 1
#define TOUCHPIN A3

volatile Adafruit_NeoPixel gems = Adafruit_NeoPixel(NUM_LEDS, PIXPIN, NEO_GRB + NEO_KHZ800);

int ref;
volatile unsigned long startOfPress = 0L;
int8_t mode = 0;
int8_t eyeLoc = 0;
volatile unsigned long eyeUpdate = 0L;
volatile unsigned long colorUpdate = 0L;
uint8_t currentLed = 0;
int8_t red = 0x00;
int8_t green = 0x00;
int8_t blue = 0x30;
int8_t brightness = 50;
int8_t oldBright = 50;
volatile bool notify = false;
volatile unsigned long hitbutDisplay = 0L;


void setup() {
  ref = ADCTouch.read(TOUCHPIN);
  pinMode(EYE, OUTPUT);
  randomSeed(analogRead(0));

  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onRequest(requestEvent);
  TinyWireS.onReceive(receiveEvent);
  gems.begin();
}

void loop() {
  const uint8_t pattern[7] = {0, 2, 1, 3, 6, 4, 5};
  const int ledDelay[3][2] = {{1200, 150}, {50, 50}, {150, 300}};
  const bool code[125] = {
    0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 1, 1, 1, 0, 0, 0,
    1, 0, 1, 0, 0, 0,
    1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 1, 0, 0, 0,
    1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 1, 1, 1, 0, 0, 0,
    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0,
    1, 0, 0, 0,
    1, 0, 1, 0, 1, 0, 0, 0,
    1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0,
    1, 1, 1, 0, 1, 1, 1, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0,
  };
  const int eyeDelay = 200;
  const int touch = ADCTouch.read(TOUCHPIN) - ref;

  if (touch > 30 && millis() - startOfPress > 500) {
    if (mode == 0) {
      mode = 1;
    } else if (mode == 1) {
      mode = 0;
      randColor();
      currentLed = 0;
    }
    colorUpdate = 0L;
    startOfPress = millis();
  }

  if (notify) {
    red = 0x60 * (brightness / 100.);
    green = red;
    blue = red;
  }

  if (oldBright != brightness) {
    currentLed = 7;
    red++;
    green++;
    blue++;
    oldBright = brightness;
  }

  if (((millis() - colorUpdate >= ledDelay[mode][0] && currentLed == 0) || (millis() - colorUpdate >= ledDelay[mode][1] && currentLed > 0) && !notify) || (millis() - colorUpdate >= ledDelay[3][mode])) {
    if (mode == 1 || notify) {
      for ( int i = 0; i < NUM_LEDS; i++) {
        gems.setPixelColor(i, gems.Color((int8_t)red, (int8_t)green, (int8_t)blue));
      }
      if (!notify) {
        rainbow();
      }
    } else {
      gems.setPixelColor(pattern[currentLed], gems.Color((int8_t)red, (int8_t)green , (int8_t)blue));
    }
    gems.show();
    currentLed += 1;
    colorUpdate = millis();
    if (currentLed == 8 && notify) {
      notify = false;
    }
    if (currentLed > NUM_LEDS) {
      currentLed = 0;
      if (mode == 0) {
        randColor();
      }
    }
  }

  if (millis() - eyeUpdate >= eyeDelay) {
    digitalWrite(EYE, !code[eyeLoc]);
    eyeLoc++;
    if (eyeLoc > sizeof(code)) {
      eyeLoc = 0;
    }
    eyeUpdate = millis();
  }

  TinyWireS_stop_check();

}
void rainbow() {
  int inc = 6 * (brightness / 100.);
  if (inc < 1) {
    inc = 1;
  }
  if (red != 0 && blue != 0 && green != 0) {
    red = 0x60 * (brightness / 100.);
    green = 0x0;
    blue = 0x0;
  }
  if (red > 0 && blue == 0) {
    red -= inc;
    green += inc;
    if (red < 0) {
      red = 0;
    }
  } else if (green > 0 && red == 0) {
    green -= inc;
    blue += inc;
    if ( green < 0) {
      green = 0;
    }
  } else {
    blue -= inc;
    red += inc;
    if (blue < 0) {
      blue = 0;
    }
  }
}
void randColor() {
  int8_t colorThree = 0;
  uint8_t option = random(6);
  int8_t maxBright = 0x60 * (brightness / 100.);
  int8_t minBright = 0x20 * (brightness / 100.);
  int8_t colorOne = random(minBright, maxBright);
  int8_t colorTwo = random(maxBright);
  if (minBright > 0) {
    int8_t colorThree = random(minBright);
  }

  if (option == 0) {
    red = colorThree;
    green = colorTwo;
    blue = colorOne;
  } else if (option == 1) {
    red = colorThree;
    green = colorOne;
    blue = colorTwo;
  } else if (option == 2) {
    red = colorTwo;
    green = colorThree;
    blue = colorOne;
  } else if (option == 3) {
    red = colorTwo;
    green = colorOne;
    blue = colorThree;
  } else if (option == 4) {
    red = colorOne;
    green = colorThree;
    blue = colorTwo;
  } else if (option == 5) {
    red = colorOne;
    green = colorTwo;
    blue = colorThree;
  }
}

void receiveEvent() {
  int8_t option = TinyWireS.receive();
  if (option == 1) {
    while (TinyWireS.available()) {
      TinyWireS.receive();
    }
    notify = true;
  } else if (option == 2) {
    brightness = (TinyWireS.receive() / 128.) * 100;
  }
}

void requestEvent() {
  if (millis() - hitbutDisplay >= 60000) {
    const byte message[13] = {2, 11,
                              'H', 'a', 'k', 'i', 'n',
                              't', 'h', 'e', 'B', 'o', 'x'
                             };

    for (uint8_t i = 0; i < sizeof(message); i++) {
      TinyWireS.send(message[i]);
    }
    hitbutDisplay = millis();
  } else {
    TinyWireS.send(0);
  }
}

