
#include <Controllino.h>

#define bA1 CONTROLLINO_A0
#define bA2 CONTROLLINO_A1
#define bA3 CONTROLLINO_A2
#define bA4 CONTROLLINO_A3
#define bA5 CONTROLLINO_A4

#define bB1 CONTROLLINO_A8
#define bB2 CONTROLLINO_A9
#define bB3 CONTROLLINO_A10
#define bB4 CONTROLLINO_A11
#define bB5 CONTROLLINO_A12

#define lA1 CONTROLLINO_D0
#define lA2 CONTROLLINO_D1
#define lA3 CONTROLLINO_D2
#define lA4 CONTROLLINO_D3
#define lA5 CONTROLLINO_D4

#define lB1 CONTROLLINO_D8
#define lB2 CONTROLLINO_D9
#define lB3 CONTROLLINO_D10
#define lB4 CONTROLLINO_D11
#define lB5 CONTROLLINO_D12

#define LED CONTROLLINO_D23

#define NUM_BTN 5
#define DEBOUNCE 50
#define MAX_GAME_TIME 31500
#define COUNTDOWN_TIME 15000
#define PRE_END_TIME 5000
#define END_TIME 20000

// Player A
uint8_t btns_A[] = { bA1, bA2, bA3, bA4, bA5 };
uint8_t leds_A[] = { lA1, lA2, lA3, lA4, lA5 };
bool a[] = { false, false, false, false, false };
bool aLong[] = { false, false, false, false, false };
bool aChanged[] = { false, false, false, false, false };
unsigned long tA[] = { 0, 0, 0, 0, 0 };

// Player B
uint8_t btns_B[] = { bB1, bB2, bB3, bB4, bB5 };
uint8_t leds_B[] = { lB1, lB2, lB3, lB4, lB5 };
bool b[] = { false, false, false, false, false };
bool bLong[] = { false, false, false, false, false };
int bChanged[] = { false, false, false, false, false };
unsigned long tB[] = { 0, 0, 0, 0, 0 };

// Global
unsigned long tGame = 0;
unsigned long tBlink = 0;
unsigned long tBlinkL = 0;
unsigned long tPing = 0;
bool blink = false;
bool blinkL = false;
int state = -1;
bool P = true;
bool E = false;

int scoreA = 0;
int scoreB = 0;
char winner;

unsigned long tLedSeq = 0;
int iLedSeq = 0;
bool fLedSeq = 0;

unsigned long tDebug = 0;

int ssA = 0;
int ssB = 0;
int vA = 5;
int old_vA = 5;
int vB = 5;
int old_vB = 5;

unsigned long tScoreTimeA = 0;
unsigned long tScoreTimeB = 0;

int maxScoreTimeA = 3000;
int maxScoreTimeB = 3000;

void setup() {

  for (uint8_t i = 0; i < NUM_BTN; i++) {
    pinMode(btns_A[i], INPUT);
    pinMode(btns_B[i], INPUT);
    pinMode(leds_A[i], OUTPUT);
    pinMode(leds_B[i], OUTPUT);
  }

  leds_off();

  Serial.begin(115200);

  delay(100);

  for (uint8_t i = 0; i < NUM_BTN; i++) {
    a[i] = !digitalRead(btns_A[i]);
    b[i] = !digitalRead(btns_B[i]);
  }

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
}


void loop() {

  // ----------------------------- STATE == -1 ------ BOOTING
  if (state == -1) {
    if (P) {
      P = false;
      //Serial.println("BOOTING");
    }
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '?') {
        state = 0;
        P = true;
      }
    }

  }
  // ----------------------------- STATE == 0 ------ IDLE
  else if (state == 0) {
    if (P) {
      P = false;
      //Serial.println("IDLE");
      digitalWrite(LED, HIGH);
      sendState();
      resetButtons();
    }

    checkButtons();
    leds_seq();

    for (uint8_t i = 0; i < NUM_BTN; i++) {
      if (aChanged[i]) {
        if (!a[i]) {
          // PLAYER A STARTS THE GAME
          E = true;
          aChanged[i] = false;
        }
      }
      if (bChanged[i]) {
        if (!b[i]) {
          // PLAYER B STARTS THE GAME
          E = true;
          bChanged[i] = false;
        }
      }
    }

    if (E) {

      for (int i = 0; i < 4; i++) {
        leds_on();
        delay(100);
        leds_off();
        delay(100);
      }

      P = true;
      state = 1;
      tGame = millis();
      E = false;
    }

  }
  // ----------------------------- STATE == 1 ------ COUNTDOWN
  else if (state == 1) {
    if (P) {
      P = false;
      //Serial.println("COUNTDOWN");
      sendState();
    }

    blinkStateLed(100, 900);


    if ((millis() - tGame) > COUNTDOWN_TIME) {
      state = 2;
      P = true;
      tGame = millis();
    }

  }
  // ----------------------------- STATE == 2 ------ GAME
  else if (state == 2) {
    if (P) {
      P = false;
      scoreA = 0;
      scoreB = 0;
      //Serial.println("GAME");
      sendState();
      resetButtons();
    }

    blinkStateLed(250, 250);

    checkButtons();

    // genera 2 numeri randomici
    if (ssA == 0) {
      ledsA_off();
      vA = 2;
      ssA = 1;
      // if (vA != old_vA) {
      //   ssA = 1;
      //   old_vA = vA;
      // }
    }
    if (ssB == 0) {
      ledsB_off();
      vB = random(5);
      if (vB != old_vB) {
        ssB = 1;
        old_vB = vB;
      }
    }

    // accende la luce corrispondente
    if (ssA == 1) {
      digitalWrite(leds_A[vA], LOW);
      tScoreTimeA = millis();
      ssA = 2;
    }
    if (ssB == 1) {
      digitalWrite(leds_B[vB], LOW);
      tScoreTimeB = millis();
      ssB = 2;
    }

    // controlla quale pulsante viene schiacciato
    if (ssA == 2) {
      if ((millis() - tScoreTimeA) > (maxScoreTimeA - scoreA * 25)) {
        ssA = 0;
      }

      for (uint8_t i = 0; i < NUM_BTN; i++) {
        if (aChanged[i]) {
          if (a[i]) {
            if (i == vA) {
              scoreA++;
              Serial.print("a,");
              Serial.println(scoreA);
              ssA = 0;
              aChanged[i] = false;
            }
          }
        }
      }
    }

    if (ssB == 2) {
      if ((millis() - tScoreTimeB) > (maxScoreTimeB - scoreB * 25)) {
        ssB = 0;
      }

      for (uint8_t i = 0; i < NUM_BTN; i++) {
        if (bChanged[i]) {
          if (b[i]) {
            if (i == vB) {
              scoreB++;
              Serial.print("b,");
              Serial.println(scoreB);
              ssB = 0;
              bChanged[i] = false;
            }
          }
        }
      }
    }


    // EXIT CONDITIONS
    if ((millis() - tGame) > MAX_GAME_TIME) {
      E = true;
    }

    // EXIT
    if (E) {
      E = false;
      P = true;
      state = 3;
      tGame = millis();
    }


  }
  // ----------------------------- STATE == 3 ------ PRE END
  else if (state == 3) {
    if (P) {
      P = false;
      //Serial.println("end game");
      sendState();
    }

    blinkLeds(500, 500);
    blinkStateLed(500, 500);

    if ((millis() - tGame) > PRE_END_TIME) {
      P = true;
      state = 4;
      tGame = millis();
    }
  }
  // ----------------------------- STATE == 4 ------ END GAME
  else if (state == 4) {
    if (P) {
      P = false;
      //Serial.println("end game");
      sendState();
      leds_off();
    }

    blinkStateLed(500, 500);

    if ((millis() - tGame) > END_TIME) {
      P = true;
      state = 0;
      tGame = millis();
    }
  }
}




// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

void resetButtons() {
  for (uint8_t i = 0; i < NUM_BTN; i++) {
    aChanged[i] = false;
    bChanged[i] = false;
    a[i] = false;
    b[i] = false;
  }
}

void debug() {
  if ((millis() - tDebug) > 500) {
    tDebug = millis();
    Serial.print("aChanged ");
    for (int i = 0; i < 5; i++) {
      Serial.print(aChanged[i]);
      Serial.print(",");
    }
    Serial.println();
    Serial.print("bChanged ");
    for (int i = 0; i < 5; i++) {
      Serial.print(bChanged[i]);
      Serial.print(",");
    }
    Serial.println();
  }
}

void checkButtons() {

  for (uint8_t i = 0; i < NUM_BTN; i++) {
    // PLAYER A
    if (!a[i]) {
      if ((millis() - tA[i]) > DEBOUNCE) {
        if (digitalRead(btns_A[i])) {
          tA[i] = millis();
          a[i] = true;
          aChanged[i] = true;
        }
      }
    }

    if (a[i]) {
      if ((millis() - tA[i]) > DEBOUNCE) {
        if (!digitalRead(btns_A[i])) {
          tA[i] = millis();
          a[i] = false;
          aChanged[i] = true;
        }
      }
    }

    // PLAYER B
    if (!b[i]) {
      if ((millis() - tB[i]) > DEBOUNCE) {
        if (digitalRead(btns_B[i])) {
          tB[i] = millis();
          b[i] = true;
          bChanged[i] = true;
        }
      }
    }

    if (b[i]) {
      if ((millis() - tB[i]) > DEBOUNCE) {
        if (!digitalRead(btns_B[i])) {
          tB[i] = millis();
          b[i] = false;
          bChanged[i] = true;
        }
      }
    }
  }
}


void blinkStateLed(int dtOn, int dtOff) {
  if (!blink) {
    if ((millis() - tBlink) > dtOff) {
      tBlink = millis();
      blink = true;
      digitalWrite(LED, blink);
    }
  } else {
    if ((millis() - tBlink) > dtOn) {
      tBlink = millis();
      blink = false;
      digitalWrite(LED, blink);
    }
  }
}

void blinkLeds(int dtOn, int dtOff) {
  if (!blinkL) {
    if ((millis() - tBlinkL) > dtOff) {
      tBlinkL = millis();
      blinkL = true;
      ledsA_on();
      ledsB_off();
    }
  } else {
    if ((millis() - tBlinkL) > dtOn) {
      tBlinkL = millis();
      blinkL = false;
      ledsB_on();
      ledsA_off();
    }
  }
}

void sendState() {
  Serial.print("z,");
  Serial.println(state);
}

void leds_off() {
  for (uint8_t i = 0; i < NUM_BTN; i++) {
    digitalWrite(leds_A[i], HIGH);
    digitalWrite(leds_B[i], HIGH);
  }
}

void ledsA_off() {
  digitalWrite(leds_A[0], HIGH);
  digitalWrite(leds_A[1], HIGH);
  digitalWrite(leds_A[2], HIGH);
  digitalWrite(leds_A[3], HIGH);
  digitalWrite(leds_A[4], HIGH);
}

void ledsB_off() {
  digitalWrite(leds_B[0], HIGH);
  digitalWrite(leds_B[1], HIGH);
  digitalWrite(leds_B[2], HIGH);
  digitalWrite(leds_B[3], HIGH);
  digitalWrite(leds_B[4], HIGH);
}

void ledsA_on() {
  digitalWrite(leds_A[0], LOW);
  digitalWrite(leds_A[1], LOW);
  digitalWrite(leds_A[2], LOW);
  digitalWrite(leds_A[3], LOW);
  digitalWrite(leds_A[4], LOW);
}

void ledsB_on() {
  digitalWrite(leds_B[0], LOW);
  digitalWrite(leds_B[1], LOW);
  digitalWrite(leds_B[2], LOW);
  digitalWrite(leds_B[3], LOW);
  digitalWrite(leds_B[4], LOW);
}
void leds_on() {
  for (uint8_t i = 0; i < NUM_BTN; i++) {
    digitalWrite(leds_A[i], LOW);
    digitalWrite(leds_B[i], LOW);
  }
}

void leds_seq() {

  if ((millis() - tLedSeq) > 400) {
    tLedSeq = millis();

    leds_off();
    digitalWrite(leds_A[iLedSeq], fLedSeq);
    digitalWrite(leds_B[iLedSeq], !fLedSeq);

    if (iLedSeq >= NUM_BTN - 1) {
      iLedSeq = 0;
      fLedSeq = !fLedSeq;
    } else {
      iLedSeq++;
    }
  }
}
