#include <EEPROM.h>
#define EEPROM_SIZE 2000
#define E_ADD_1 100
#define E_ADD_2 300
#define E_ADD_3 600
#define E_ADD_4 900

int adds[] = { E_ADD_1, E_ADD_2, E_ADD_3, E_ADD_4 };

#include <WiFi.h>

const char* ssid = "demodemo";
const char* password = "123456789";
int status = WL_IDLE_STATUS;

unsigned int udpPort = 2390;

IPAddress serverIP = IPAddress(192, 48, 56, 2);
IPAddress remoteIP = IPAddress(192, 48, 56, 3);
IPAddress gatewayIP = IPAddress(192, 48, 56, 2);
IPAddress subnetIP = IPAddress(255, 255, 255, 0);

WiFiUDP Udp;

typedef struct enc_message {
  uint16_t enc_val[4];
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
int encMax[] = { 255 };
int encMin[] = { 0 };

int state = 0;
bool btnEn = false;
unsigned long tPress = 0;
bool btnGo = false;

bool change = false;
unsigned long tChange = 0;

void init_enc(int num) {
  if (ENC_TYPE == 1) {
    enc[num].attachHalfQuad(clk[num], dt[num]);
  } else if (ENC_TYPE == 2) {
    enc[num].attachSingleEdge(clk[num], dt[num]);
  } else if (ENC_TYPE == 3) {
    enc[num].attachFullQuad(clk[num], dt[num]);
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);

  pinMode(btnPin, INPUT);

  EEPROM.begin(EEPROM_SIZE);
  // INIT ENCODER
  ESP32Encoder::useInternalWeakPullResistors = UP;

  for (int i = 0; i < NUM_ENC; i++) {
    init_enc(i);
  }


  WiFi.softAPConfig(serverIP, gatewayIP, subnetIP);
  status = WiFi.softAP(ssid, password);
  delay(1000);

  encVal[0] = EEPROM.read(adds[0]);
  enc[0].setCount(encVal[0]/5);
  encData.enc_val[0] = encVal[0];
  encData.enc_val[1] = EEPROM.read(adds[1]);
  encData.enc_val[2] = EEPROM.read(adds[2]);
  encData.enc_val[3] = EEPROM.read(adds[3]);

  Udp.begin(udpPort);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  oldEnc[0] = encVal[0];
}

uint8_t buffer[1024];

void loop() {

  readEnc();

  readBtn();

  if (btnGo) {
    btnGo = false;

    EEPROM.write(adds[state], encVal[0]);
    EEPROM.commit();

    if (state >= 3) {
      state = 0;
    } else {
      state++;
    }

    encVal[0] = EEPROM.read(adds[state]);

    Serial.print("STATE: ");
    Serial.println(state);

    if (state == 0) {
      encStep = 5;
      encData.enc_val[0] = encVal[0];
    }
    if (state == 1) {
      encStep = 5;
      encData.enc_val[1] = encVal[0];
    }
    if (state == 2) {
      encStep = 5;
      encData.enc_val[2] = encVal[0];
    }
    if (state == 3) {
      encStep = 5;
      encData.enc_val[3] = encVal[0];
    }

    change = true;
    tChange = millis();

    enc[0].setCount(encVal[0] / encStep);
    oldEnc[0] = encVal[0];
  }

  if (encVal[0] != oldEnc[0]) {

    if (state == 0) {
      encStep = 5;
      encData.enc_val[0] = encVal[0];
    }
    if (state == 1) {
      encStep = 5;
      encData.enc_val[1] = encVal[0];
    }
    if (state == 2) {
      encStep = 5;
      encData.enc_val[2] = encVal[0];
    }
    if (state == 3) {
      encStep = 5;
      encData.enc_val[3] = encVal[0];
    }

    Serial.print(encData.enc_val[0]);
    Serial.print(",");
    Serial.print(encData.enc_val[1]);
    Serial.print(",");
    Serial.print(encData.enc_val[2]);
    Serial.print(",");
    Serial.print(encData.enc_val[3]);
    Serial.println();

    change = true;
    tChange = millis();

    oldEnc[0] = encVal[0];
  }

  if ((millis() - tChange) > 250) {
    if (change) {
      sendUdp();
      EEPROM.write(adds[state], encVal[0]);
      EEPROM.commit();
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
