#include <EEPROM.h>
#define EEPROM_SIZE 2000
#define E_ADD_1 100
#define E_ADD_2 300
#define E_ADD_3 600
#define E_ADD_4 900
#define E_ADD_5 1200

int adds[] = { E_ADD_1, E_ADD_2, E_ADD_3, E_ADD_4, E_ADD_5 };

#include <WiFiS3.h>

char ssid[] = "demodemo";   // your network SSID (name)
char pass[] = "123456789";  // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
unsigned int udpPort = 2390;
WiFiUDP Udp;

IPAddress remoteIP = IPAddress(192, 48, 56, 3);
IPAddress l_ip;

char receiveBuffer[1024];

typedef struct enc_message {
  int enc_val;
} enc_message;

enc_message encData;

#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>
FASTLED_USING_NAMESPACE

// Params for width and height
const uint8_t kMatrixWidth = 64;
const uint8_t kMatrixHeight = 11;
const bool kMatrixSerpentineLayout = true;

#define DATA_PIN 3
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
uint8_t colorIndex[NUM_LEDS];

uint16_t XY(uint8_t x, uint8_t y) {
  uint16_t i;
  if (kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }
  if (kMatrixSerpentineLayout == true) {
    if (y & 0x01) {

      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {

      i = (y * kMatrixWidth) + x;
    }
  }
  return i;
}


DEFINE_GRADIENT_PALETTE(heatmap_gp){
  0, 100, 255, 60,    //orange
  127, 0, 0, 0,       //black
  255, 100, 255, 60,  //red
};

DEFINE_GRADIENT_PALETTE(heatmap_bl){
  0, 100, 255, 60,    //orange
  20, 0, 0, 0,        //black
  235, 0, 0, 0,       //black
  255, 100, 255, 60,  //red
};


CRGBPalette16 myPal = heatmap_gp;
CRGBPalette16 myPalBlack = heatmap_bl;

#define MAX_DIMENSION ((kMatrixWidth > kMatrixHeight) ? kMatrixWidth : kMatrixHeight)

static uint16_t xxx;
static uint16_t yyy;
static uint16_t zzz;

int speed = 20;
uint16_t scaleX = 40;
uint16_t scaleY = 10;

uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];

CRGBPalette16 currentPalette(PartyColors_p);
uint8_t colorLoop = 1;

uint8_t hue = 0;
uint8_t maxBrightness = 0;
uint8_t hole = 0;
uint8_t loopIdx = 0;

unsigned long tFPS = 0;
unsigned long tMove = 0;
unsigned long dMove = 0;
unsigned long tStatus = 0;
unsigned long tDebug = 0;
unsigned long tRead = 0;
unsigned long tRainbow = 0;
unsigned long tFire = 0;
unsigned long tRain = 0;
unsigned long tFlick = 0;
unsigned long tWave = 0;

long dRainbow = 0;
long dFire = 10;
long dRain = 10;
long dFlick = 100;
long dWave = 65;

uint8_t rIdx = 0;

bool firstConnection = true;
bool firstDisconnection = true;
bool firstFlicker = true;
bool firstRain = true;
bool firstWave = true;
bool firstFire = true;
bool firstNoise = true;

int oldVal;
int state;

void setup() {

  Serial.begin(115200);
  delay(1000);

  random16_add_entropy(random());

  hue = 127;
  maxBrightness = 255;
  hole = 250;
  dMove = map(speed, 0, 255, 75, 0);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setTemperature(Candle);

  for (int i = 0; i < NUM_LEDS; i++) {
    colorIndex[i] = (int)random8() * 0.6 + (int)(i * 2);
  }


  FastLED.setBrightness(20);
  FastLED.clear();
  FastLED.show();

  digitalWrite(LED_BUILTIN, LOW);

  oldVal = encData.enc_val;

  SetupPurpleAndGreenPalette();
  speed = EEPROM.read(adds[0]);
  colorLoop = 1;
}

void loop() {

  debug();

  if ((millis() - tRead) > 33) {
    tRead = millis();
    readUdp();
  }

  if (status != WL_CONNECTED) {

    // Fire();
    // rain();
    // flicker();
    // wave();
    noise_wave();

  } else {
    if (state == 0) {
      noise_wave();
    } else if (state == 1) {
      wave();
    } else if (state == 2) {
      rain();
    } else if (state == 3) {
      Fire();
    } else if (state == 4) {
      flicker();
    }
  }

  if ((millis() - tFPS) > 33) {
    tFPS = millis();
    FastLED.show();
  }

  if ((millis() - tStatus) > 3000) {
    tStatus = millis();
    status = WiFi.status();
    if (status != WL_CONNECTED) {
      Serial.println("PROVO A CONNETTERMI");
      WiFi.config(remoteIP);
      WiFi.begin(ssid, pass);
      firstConnection = true;
    } else {
      if (firstConnection) {
        Serial.println("CONNESSO");
        Udp.begin(udpPort);
        l_ip = WiFi.localIP();
        Serial.println(l_ip);
        firstConnection = false;
        firstDisconnection = true;
      }
    }
  }
}

void debug() {
  if ((millis() - tDebug) > 250) {
    tDebug = millis();
    Serial.print(state);
    Serial.print(",");
    Serial.print(speed);
    Serial.print(",");
    Serial.print(dWave);
    Serial.print(",");
    Serial.print(dRain);
    Serial.print(",");
    Serial.print(dFire);
    Serial.print(",");
    Serial.print(dFlick);
    Serial.print("  -  ");
    Serial.print(encData.enc_val);
    Serial.print(",");
    Serial.print(oldVal);
    Serial.println();
  }
}

void readUdp() {

  if (Udp.parsePacket()) {
    Udp.read(receiveBuffer, sizeof(encData));
    memcpy(&encData, receiveBuffer, sizeof(encData));

    if (encData.enc_val != -100001) {

      if (state == 0) {
        speed += (encData.enc_val - oldVal);
        if (speed < 1) {
          speed = 1;
        }
        if (speed > 50) {
          speed = 50;
        }
        EEPROM.write(adds[0], speed);
      } else if (state == 1) {
        dWave += (encData.enc_val - oldVal);
        if (dWave < 1) {
          dWave = 1;
        }
        if (dWave > 100) {
          dWave = 100;
        }
        EEPROM.write(adds[1], dWave);
      } else if (state == 2) {
        dRain += (encData.enc_val - oldVal);
        if (dRain < 1) {
          dRain = 1;
        }
        if (dRain > 100) {
          dRain = 100;
        }
        EEPROM.write(adds[2], dRain);
      } else if (state == 3) {
        dFire += (encData.enc_val - oldVal);
        if (dFire < 1) {
          dFire = 1;
        }
        if (dFire > 50) {
          dFire = 50;
        }
        EEPROM.write(adds[3], dFire);
      } else if (state == 4) {
        dFlick += (encData.enc_val - oldVal);
        if (dFlick < 1) {
          dFlick = 1;
        }
        if (dFlick > 100) {
          dFlick = 100;
        }
        EEPROM.write(adds[4], dFlick);
      }
      oldVal = encData.enc_val;
    } else {
      switch (state) {
        case 0:
          firstWave = true;
          dWave = EEPROM.read(adds[1]);
          state = 1;
          break;
        case 1:
          firstRain = true;
          dRain = EEPROM.read(adds[2]);
          state = 2;
          break;
        case 2:
          firstFire = true;
          dFire = EEPROM.read(adds[3]);
          state = 3;
          break;
        case 3:
          firstFlicker = true;
          dFlick = EEPROM.read(adds[4]);
          state = 4;
          break;
        case 4:
          firstNoise = true;
          speed = EEPROM.read(adds[0]);
          state = 0;
          break;
        default:
          break;
      }
    }
  }
}

void noise_wave() {

  fillnoise8();
  mapNoiseToLEDsUsingPalette();
}

void rainbow() {

  if ((millis() - tRainbow) > 100) {
    tRainbow = millis();
    fill_solid(leds, NUM_LEDS, CHSV(rIdx, 255, 255));
    rIdx++;
  }
}

int rainIdx[kMatrixHeight];
CRGB c = CRGB::LimeGreen;

void rain() {
  if ((millis() - tRain) > dRain) {

    if (firstRain) {
      firstRain = false;
      FastLED.clear();
      for (uint8_t i = 0; i < kMatrixHeight; i++) {
        rainIdx[i] = random(kMatrixWidth);
      }
    }

    for (uint8_t y = 0; y < kMatrixHeight; y++) {
      tRain = millis();
      if (y % 2 == 0) {
        fadeToBlackBy(&leds[XY(0, y)], kMatrixWidth, 50);
      } else {
        fadeToBlackBy(&leds[XY(kMatrixWidth - 1, y)], kMatrixWidth, 50);
      }
      leds[XY(rainIdx[y], y)] = c;
      if (rainIdx[y] > 0) {
        rainIdx[y]--;
      } else {
        rainIdx[y] = kMatrixWidth;
      }
    }
  }
}

int flickIdx[20];
int old_flickIdx[20];
bool flickFlag[20];

void flicker() {
  if (firstFlicker) {
    firstFlicker = false;
    for (uint8_t i = 0; i < 20; i++) {
      flickIdx[i] = random(NUM_LEDS);
      flickFlag[i] = random(2);
      old_flickIdx[i] = flickIdx[i];
    }
    FastLED.clear();
  }

  if ((millis() - tFlick) > dFlick*10) {
    tFlick = millis();

    for (uint8_t i = 0; i < 20; i++) {
      flickIdx[i] = random(NUM_LEDS - 3);
      flickFlag[i] = !flickFlag[i];

      if (flickFlag[i]) {
        leds[flickIdx[i]] = c;
        leds[flickIdx[i] + 1] = c;
        leds[flickIdx[i] + 2] = c;
      } else {
        leds[old_flickIdx[i]] = CRGB(0, 0, 0);
        leds[old_flickIdx[i] + 1] = CRGB(0, 0, 0);
        leds[old_flickIdx[i] + 2] = CRGB(0, 0, 0);
      }

      old_flickIdx[i] = flickIdx[i];
    }
  }
}

uint8_t xj = 0;
uint8_t w[8];

void wave() {

  if ((millis() - tWave) > dWave) {
    tWave = millis();
    if (firstWave) {
      firstWave = false;
      for (uint8_t i = 0; i < 14; i++) {
        w[i] = dim8_raw(sin8(i * 16));
      }
    }

    FastLED.clear();

    for (uint8_t y = 0; y < 11; y++) {
      for (uint8_t i = 0; i < 14; i++) {
        if ((xj + i) >= 63) {
          leds[XY(xj + i - 63, y)] = CHSV(80, 194, w[i]);
        } else {
          leds[XY(xj + i, y)] = CHSV(80, 194, w[i]);
        }
      }
    }

    if (xj >= 63) {
      xj = 0;
    } else {
      xj++;
    }
  }
}


void Fire() {
  if ((millis() - tFire) > dFire) {
    tFire = millis();
    // COLOR THE SEQUENCE
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
      leds[i] = blend(ColorFromPalette(myPal, colorIndex[i]), ColorFromPalette(myPalBlack, colorIndex[i]), hole);
      // if (hue > 127) {
      //   leds[i].r *= ((hue - 128.0) / 128.0) + 1.0;
      // } else {
      //   leds[i].b *= ((127 - hue) / 128.0) + 1.0;
      // }
    }


    // ANIMATE THE SEQUENCE
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
      colorIndex[i]++;
    }

    // AVOID ANIMATION REPETITION
    if (loopIdx < 255) {
      loopIdx++;
    } else {
      loopIdx = 0;
      for (uint16_t i = 0; i < NUM_LEDS; i++) {
        colorIndex[i] += random(8);
      }
    }
  }
}

void fillnoise8() {
  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".
  uint8_t dataSmoothing = 0;
  if (speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = scaleX * i;
    for (int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = scaleY * j;

      uint8_t data = inoise8(xxx + ioffset, yyy + joffset, zzz);

      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      data = qsub8(data, 16);
      data = qadd8(data, scale8(data, 39));

      if (dataSmoothing) {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8(olddata, dataSmoothing) + scale8(data, 256 - dataSmoothing);
        data = newdata;
      }

      noise[i][j] = data;
    }
  }

  zzz += speed;

  // apply slow drift to X and Y, just for visual variation.
  xxx += speed / 8;
  yyy -= speed / 16;
}

void mapNoiseToLEDsUsingPalette() {
  static uint8_t ihue = 0;

  for (int i = 0; i < kMatrixWidth; i++) {
    for (int j = 0; j < kMatrixHeight; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's index into the color palette.

      uint8_t index = noise[j][i];
      uint8_t bri = noise[i][j];

      // if this palette is a 'loop', add a slowly-changing base value
      if (colorLoop) {
        index += ihue;
      }

      // brighten up, as the color palette itself often contains the
      // light/dark dynamic range desired
      if (bri > 127) {
        bri = 255;
      } else {
        bri = dim8_raw(bri * 2);
      }

      CRGB color = ColorFromPalette(currentPalette, index, bri);
      leds[XY(i, j)] = color;
    }
  }

  ihue += 1;
}

void SetupPurpleAndGreenPalette() {
  CRGB green = CHSV(80, 194, 255);
  CRGB black = CRGB::Black;

  currentPalette = CRGBPalette16(
    green, green, green, green,
    green, green, green, green,
    black, black, black, black,
    black, black, black, black);
}