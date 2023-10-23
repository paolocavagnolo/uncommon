
#define NUM_BTN 5
#define DEBOUNCE 50
#define MAX_GAME_TIME 5000
#define MAX_SCORE 20
#define PING_TIME 1000
#define COUNTDOWN_TIME 3000
#define END_TIME 5000

// Player A
uint8_t btns_A[] = { 4, 4, 4, 4, 4 };
uint8_t leds_A[] = { 5, 5, 5, 5, 5 };
bool a[] = { false, false, false, false, false };
bool aLong[] = { false, false, false, false, false };
bool aChanged[] = { false, false, false, false, false };
unsigned long tA[] = { 0, 0, 0, 0, 0 };

// Player B
uint8_t btns_B[] = { 2, 3, 3, 3, 3 };
uint8_t leds_B[] = { 6, 6, 6, 6, 6 };
bool b[] = { false, false, false, false, false };
bool bLong[] = { false, false, false, false, false };
int bChanged[] = { false, false, false, false, false };
unsigned long tB[] = { 0, 0, 0, 0, 0 };

// Global
unsigned long tGame = 0;
unsigned long tBlink = 0;
unsigned long tPing = 0;
bool blink = false;
int state = -1;
bool P = true;
bool E = false;

int scoreA = 0;
int scoreB = 0;
char winner = "";


void setup() {
  Serial.begin(9600);

  delay(100);

  for (uint8_t i = 0; i < NUM_BTN; i++) {
    pinMode(btns_A, INPUT);
    pinMode(btns_B, INPUT);
    pinMode(leds_A, OUTPUT);
    pinMode(leds_B, OUTPUT);
  }

  delay(100);

  for (uint8_t i = 0; i < NUM_BTN; i++) {
    a[i] = !digitalRead(btns_A[i]);
    b[i] = !digitalRead(btns_B[i]);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
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
      digitalWrite(LED_BUILTIN, HIGH);
      sendState();
    }

    checkButtons();

    for (uint8_t i = 0; i < NUM_BTN; i++) {
      if (aChanged[i]) {
        if (!a[i]) {
          // PLAYER A STARTS THE GAME
          P = true;
          state = 1;
          tGame = millis();
          aChanged[i] = false;
        }
      }
      if (bChanged[i]) {
        if (!b[i]) {
          // PLAYER B STARTS THE GAME
          P = true;
          state = 1;
          tGame = millis();
          bChanged[i] = false;
        }
      }
    }

  }
  // ----------------------------- STATE == 1 ------ COUNTDOWN
  else if (state == 1) {
    if (P) {
      P = false;
      //Serial.println("COUNTDOWN");
      sendState();
    }

    blinkLed(100, 900);

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
      //Serial.println("GAME");
      sendState();
    }

    blinkLed(250, 250);

    //checkButtons();

    if ((millis() - tGame) > MAX_GAME_TIME) {
      E = true;
      if (scoreA > scoreB) {
        winner = "A";
      } else if (scoreB > scoreA) {
        winner = "B";
      } else {
        // PAREGGIO
        winner = "C";
      }
    }

    if (scoreA >= MAX_SCORE) {
      winner = "A";
      E = true;
    }
    if (scoreB >= MAX_SCORE) {
      winner = "B";
      E = true;
    }

    if (E) {
      E = false;
      P = true;
      state = 3;
      tGame = millis();
    }


  }
  // ----------------------------- STATE == 3 ------ END GAME
  else if (state == 3) {
    if (P) {
      P = false;
      //Serial.println("end game");
      sendState();
    }

    if ((millis() - tGame) > END_TIME) {
      P = true;
      state = 0;
      tGame = millis();
    }

    blinkLed(500, 500);
  }
}




// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------



void checkButtons() {

  for (uint8_t i = 0; i < NUM_BTN; i++) {
    // PLAYER A
    if (!a[i]) {
      if ((millis() - tA[i]) > DEBOUNCE) {
        if (!digitalRead(btns_A[i])) {
          tA[i] = millis();
          a[i] = true;
          aChanged[i] = true;
        }
      }
    }

    if (a[i]) {
      if ((millis() - tA[i]) > DEBOUNCE) {
        if (digitalRead(btns_A[i])) {
          tA[i] = millis();
          a[i] = false;
          aChanged[i] = true;
        }
      }
    }

    // PLAYER B
    if (!b[i]) {
      if ((millis() - tB[i]) > DEBOUNCE) {
        if (!digitalRead(btns_B[i])) {
          tB[i] = millis();
          b[i] = true;
          bChanged[i] = true;
        }
      }
    }

    if (b[i]) {
      if ((millis() - tB[i]) > DEBOUNCE) {
        if (digitalRead(btns_B[i])) {
          tB[i] = millis();
          b[i] = false;
          bChanged[i] = true;
        }
      }
    }
  }
}


void blinkLed(int dtOn, int dtOff) {
  if (!blink) {
    if ((millis() - tBlink) > dtOff) {
      tBlink = millis();
      blink = true;
      digitalWrite(LED_BUILTIN, blink);
    }
  } else {
    if ((millis() - tBlink) > dtOn) {
      tBlink = millis();
      blink = false;
      digitalWrite(LED_BUILTIN, blink);
    }
  }
}

void sendState() {
  Serial.print("z");
  Serial.println(state);
}
