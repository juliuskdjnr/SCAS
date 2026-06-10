#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- LCD Configuration ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Pin Definitions ---
const int pirEntryPin   = 4; // Entry/exit PIR
const int pirCeilingPin = 2; // Ceiling PIR
const int relayPin1     = 5; // IN1 - Lights
const int relayPin2     = 6; // IN2 - Projector
const int relayPin3     = 7; // IN3 - Fan

// --- Timer Settings ---
const unsigned long TIMEOUT_MS = 2000UL;  // 2 second off-delay
const unsigned long WARMUP_MS  = 0UL;     // No PIR warmup

// --- State ---
enum State { WARMUP, STANDBY, ACTIVE };
State currentState = WARMUP;

unsigned long timerStart    = 0;
unsigned long lastLCDUpdate = 0;
unsigned long warmupStart   = 0;

void setOutputs(bool on) {
  digitalWrite(relayPin1, on ? HIGH: LOW);
  digitalWrite(relayPin2, on ? HIGH: LOW);
  digitalWrite(relayPin3, on ? HIGH: LOW);
}

void setup() {
  pinMode(relayPin1,     OUTPUT);
  pinMode(relayPin2,     OUTPUT);
  pinMode(relayPin3,     OUTPUT);
  pinMode(pirEntryPin,   INPUT);
  pinMode(pirCeilingPin, INPUT);
  setOutputs(false);

  // Explicitly start all relays OFF
  digitalWrite(relayPin1, LOW);
  digitalWrite(relayPin2, LOW);
  digitalWrite(relayPin3, LOW);

  delay(500); // give LCD time to power up
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("PIR Warmup      ");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...  ");

  warmupStart = millis();
}

void loop() {
  unsigned long now = millis();

  // --- WARMUP phase (non-blocking) ---
  if (currentState == WARMUP) {
    if (now - lastLCDUpdate >= 1000) {
      unsigned long remaining = WARMUP_MS - (now - warmupStart);
      unsigned int secsLeft = remaining / 1000;

      lcd.setCursor(0, 1);
      lcd.print("Ready in: ");
      lcd.print(secsLeft);
      lcd.print("s   ");

      lastLCDUpdate = now;
    }

    if (now - warmupStart >= WARMUP_MS) {
      currentState = STANDBY;
      setOutputs(false);
      lcd.setCursor(0, 0);
      lcd.print("Status: STANDBY ");
      lcd.setCursor(0, 1);
      lcd.print("Room Empty      ");
    }
    return;
  }

  // --- Read both PIRs ---
  bool entryMotion    = digitalRead(pirEntryPin)   == HIGH;
  bool ceilingMotion  = digitalRead(pirCeilingPin) == HIGH;
  bool motionDetected = entryMotion || ceilingMotion;

  // --- Either PIR triggered: reset timer ---
  if (motionDetected) {
    timerStart = now;

    if (currentState != ACTIVE) {
      currentState = ACTIVE;
      setOutputs(true);
      lcd.setCursor(0, 0);
      lcd.print("Status: ACTIVE  ");
      lastLCDUpdate = 0; // force immediate LCD update
    }
  }

  // --- Active: countdown or timeout ---
  if (currentState == ACTIVE) {
    unsigned long elapsed   = now - timerStart;
    unsigned long remaining = (elapsed >= TIMEOUT_MS) ? 0 : TIMEOUT_MS - elapsed;

    if (elapsed >= TIMEOUT_MS) {
      currentState = STANDBY;
      setOutputs(false);
      lcd.setCursor(0, 0);
      lcd.print("Status: STANDBY ");
      lcd.setCursor(0, 1);
      lcd.print("Room Empty      ");
    }
    else if (now - lastLCDUpdate >= 100) {
      unsigned int secsLeft = remaining / 1000;
      unsigned int msLeft   = (remaining % 1000) / 100;

      lcd.setCursor(0, 1);
      if (motionDetected) {
        if (entryMotion && ceilingMotion) {
          lcd.print("Both: Reset!    ");
        } else if (entryMotion) {
          lcd.print("Entry: Reset!   ");
        } else {
          lcd.print("Ceiling: Reset! ");
        }
      } else {
        lcd.print("Off in: ");
        lcd.print(secsLeft);
        lcd.print(".");
        lcd.print(msLeft);
        lcd.print("s   ");
      }

      lastLCDUpdate = now;
    }
  }
}