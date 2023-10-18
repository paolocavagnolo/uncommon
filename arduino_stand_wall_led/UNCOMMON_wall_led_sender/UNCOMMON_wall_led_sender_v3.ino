#include <Encoder.h>

Encoder myEnc(5, 6);
int oldEnc;
int encVal;

#define btnPin 7

#include <WiFiS3.h>

char ssid[] = "jsk_wla";   // your network SSID (name)
char pass[] = "v7h349G£";  // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;

unsigned int udpPort = 2390;

// 192.48.56.1    MASTER
// 192.48.56.11   WL_1
// 192.48.56.12   WL_2
// 192.48.56.13   WL_3
// 192.48.56.14   WL_4
// 192.48.56.15   WL_5
// 192.48.56.16   WL_6

IPAddress serverIP = IPAddress(192, 48, 56, 1);
IPAddress remoteIP = IPAddress(192, 48, 56, 255);
IPAddress gatewayIP = IPAddress(192, 48, 56, 1);
IPAddress subnetIP = IPAddress(255, 255, 255, 0);

WiFiUDP Udp;

typedef struct enc_message {
  int enc_val;
} enc_message;

enc_message encData;

// GLOBAL VARIABLES

int state = 0;
bool btnEn = false;
unsigned long tPress = 0;
bool btnGo = false;
bool btnLong = false;

bool change = false;
unsigned long tChange = 0;



void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(btnPin, INPUT);

  WiFi.config(serverIP, gatewayIP, subnetIP);
  status = WiFi.beginAP(ssid, pass);
  delay(1000);

  Udp.begin(udpPort);
  IPAddress IP = WiFi.localIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  encVal = 0;
  oldEnc = encVal;
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

  if (btnLong) {
    btnLong = false;
    encData.enc_val = -100002;
    Serial.println("long");
    sendUdp();
  }

  if (oldEnc != encVal) {
    encData.enc_val = encVal;

    change = true;
    tChange = millis();

    oldEnc = encVal;
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
    if ((millis() - tPress) > 2000) {
      if (digitalRead(btnPin)) {
        btnLong = true;
        btnEn = false;
        tPress = millis();
      }
    } else if ((millis() - tPress) > 200) {
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
  encVal = myEnc.read()/4;
}
