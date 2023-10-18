#include <WiFi.h>

const char* ssid = "demodemo";
const char* password = "123456789";
int status = WL_IDLE_STATUS;

unsigned int udpPort = 2390;

IPAddress serverIP = IPAddress(192, 48, 56, 2);
IPAddress remoteIP = IPAddress(192, 48, 56, 255);
IPAddress gatewayIP = IPAddress(192, 48, 56, 2);
IPAddress subnetIP = IPAddress(255, 255, 255, 0);

WiFiUDP Udp;

typedef struct enc_message {
  int enc_val;
} enc_message;

enc_message encData;

#include <ESP32Encoder.h>

#define NUM_ENC 1
#define btnPin 35
#define ENC1_CLK 14
#define ENC1_DT 27

#define ENC_TYPE 2  // 1: HalfQuad  2: SingleEdge  3: FullQuad

ESP32Encoder enc[NUM_ENC];
int encVal[NUM_ENC];
int encStep = 1;
int oldEnc[NUM_ENC];
int clk[] = { ENC1_CLK };
int dt[] = { ENC1_DT };
int encMax[] = { +100000 };
int encMin[] = { -100000 };

void init_enc(int num) {
  if (ENC_TYPE == 1) {
    enc[num].attachHalfQuad(clk[num], dt[num]);
  } else if (ENC_TYPE == 2) {
    enc[num].attachSingleEdge(clk[num], dt[num]);
  } else if (ENC_TYPE == 3) {
    enc[num].attachFullQuad(clk[num], dt[num]);
  }
}

// GLOBAL VARIABLES

int state = 0;
bool btnEn = false;
unsigned long tPress = 0;
bool btnGo = false;

bool change = false;
unsigned long tChange = 0;


void setup() {

  Serial.begin(115200);
  delay(1000);

  pinMode(btnPin, INPUT);

  ESP32Encoder::useInternalWeakPullResistors = UP;

  for (int i = 0; i < NUM_ENC; i++) {
    init_enc(i);
  }

  WiFi.softAPConfig(serverIP, gatewayIP, subnetIP);
  status = WiFi.softAP(ssid, password);
  delay(1000);

  Udp.begin(udpPort);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  encVal[0] = 0;
  oldEnc[0] = encVal[0];
}

uint8_t buffer[1024];

void loop() {

  readEnc();
  readBtn();

  if (btnGo) {
    btnGo = false;
    encData.enc_val = -100001;
    sendUdp();
  }

  if (oldEnc[0] != encVal[0]) {
    encData.enc_val = encVal[0];

    change = true;
    tChange = millis();

    oldEnc[0] = encVal[0];
  }

  if ((millis() - tChange) > 100) {
    if (change) {
      sendUdp();
      change = false;
    }
  }
}

void readBtn() {
  if (btnEn) {
    if ((millis() - tPress) > 200) {
      if (digitalRead(btnPin)) {
        btnGo = true;
        btnEn = false;
        tPress = millis();
      }
    }
  } else {
    if ((millis() - tPress) > 200) {
      if (!digitalRead(btnPin)) {
        btnEn = true;
        tPress = millis();
      }
    }
  }
}

void sendUdp() {
  Udp.beginPacket(remoteIP, udpPort);
  Serial.println(encData.enc_val);
  memcpy(buffer, &encData, sizeof(encData));
  Udp.write(buffer, sizeof(encData));
  Udp.endPacket();
}

void readEnc() {
  for (uint8_t i = 0; i < NUM_ENC; i++) {
    int e = enc[i].getCount() * encStep;
    if (e > encMax[i]) {
      encVal[i] = encMax[i];
      enc[i].setCount(encMax[i] / encStep);
    } else if (e < encMin[i]) {
      encVal[i] = encMin[i];
      enc[i].setCount(encMin[i] / encStep);
    } else {
      encVal[i] = e;
    }
  }
}
