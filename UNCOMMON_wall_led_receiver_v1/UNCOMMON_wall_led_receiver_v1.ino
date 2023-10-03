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
  uint16_t enc_val[4];
} enc_message;

enc_message encData;

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
FASTLED_USING_NAMESPACE

#define DATA_PIN 3
#define NUM_LEDS 694
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
uint8_t colorIndex[NUM_LEDS];


DEFINE_GRADIENT_PALETTE(heatmap_gp){
  0, 120, 255, 60,    //orange
  127, 0, 0, 0,       //black
  255, 120, 255, 60,  //red
};

DEFINE_GRADIENT_PALETTE(heatmap_bl){
  0, 120, 255, 60,    //orange
  20, 0, 0, 0,        //black
  235, 0, 0, 0,       //black
  255, 120, 255, 60,  //red
};

DEFINE_GRADIENT_PALETTE(heatmap_cl){
  0, 255, 255, 60,    //orange
  127, 120, 255, 60,
  255, 120, 255, 255,  //red
};

CRGBPalette16 myPal = heatmap_gp;
CRGBPalette16 myPalBlack = heatmap_bl;
CRGBPalette16 myPalColored = heatmap_cl;

uint8_t hue = 0;
uint8_t maxBrightness = 0;
uint8_t speed = 0;
uint8_t hole = 0;
uint8_t loopIdx = 0;

unsigned long tFPS = 0;
unsigned long tMove = 0;
unsigned long dMove = 0;
unsigned long tStatus = 0;
unsigned long tDebug = 0;

bool firstConnection = true;
bool firstDisconnection = true;

void setup() {

  Serial.begin(115200);
  delay(1000);

  random16_add_entropy(random());

  hue = 127;
  maxBrightness = 255;
  speed = 50;
  hole = 200;
  dMove = map(speed, 0, 255, 10000, 500);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setTemperature(Candle);

  for (int i = 0; i < NUM_LEDS; i++) {
    colorIndex[i] = (int)random8() * 0.6 + (int)(i * 2);
  }

  FastLED.clear();
  FastLED.show();

  digitalWrite(LED_BUILTIN, LOW);
}


void loop() {

  debug();

  if ((micros() - tMove) > dMove) {
    tMove = micros();
    Fire();
  }

  if ((millis() - tFPS) > 33) {
    tFPS = millis();
    readUdp();

    FastLED.setBrightness(maxBrightness);
    FastLED.show();
  }

  if ((millis() - tStatus) > 1000) {
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
  if ((millis() - tDebug) > 1000) {
    tDebug = millis();
    Serial.print(hue);
    Serial.print(",");
    Serial.print(maxBrightness);
    Serial.print(",");
    Serial.print(speed);
    Serial.print(",");
    Serial.print(hole);
    Serial.println();
  }
}

void readUdp() {
  if (Udp.parsePacket()) {
    Udp.read(receiveBuffer, sizeof(encData));
    memcpy(&encData, receiveBuffer, sizeof(encData));
    hue = encData.enc_val[0];
    maxBrightness = encData.enc_val[1];
    speed = encData.enc_val[2];
    dMove = map(speed, 0, 255, 10000, 500);
    hole = encData.enc_val[3];
  }
}


void Fire() {

  // COLOR THE SEQUENCE
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = blend(ColorFromPalette(myPal, colorIndex[i]), ColorFromPalette(myPalBlack, colorIndex[i]), hole);
    if (hue > 127) {
      leds[i].r *= ((hue - 128.0)/128.0)+1.0;
    } else {
      leds[i].b *= ((127 - hue)/128.0)+1.0;
    }
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