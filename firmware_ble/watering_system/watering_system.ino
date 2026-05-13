// =============================================================
// Watering System — Seeed Tiny BLE (nRF51822)
// =============================================================
// Single solenoid valve, gravity-fed, 8-plant manifold.
// No WiFi. Valve alternates open/closed on a fixed cycle.
// =============================================================

// ---- Configuration (edit these) -----------------------------

// GPIO pin wired to the TIP121G base via 470 Ω resistor.
#define VALVE_PIN 4

// Valve on-time and off-time, in seconds.
#define VALVE_ON_S  20
#define VALVE_OFF_S 20

// Seeed Tiny BLE RGB LED pin numbers. Active LOW — write LOW
// to turn on, HIGH to turn off.
#define LED_RED   22
#define LED_GREEN 21
#define LED_BLUE  23

// ---- LED helpers --------------------------------------------

void ledOn(int pin)  { digitalWrite(pin, LOW);  }
void ledOff(int pin) { digitalWrite(pin, HIGH); }

void blink(int pin, int n, int onMs = 200, int offMs = 200) {
  for (int i = 0; i < n; i++) {
    ledOn(pin);  delay(onMs);
    ledOff(pin); delay(offMs);
  }
}

// ---- Valve control ------------------------------------------

void openValve()  { digitalWrite(VALVE_PIN, HIGH); }
void closeValve() { digitalWrite(VALVE_PIN, LOW);  }

// ---- Setup --------------------------------------------------

void setup() {
  Serial.begin(115200);
  Serial.println("=== Watering system boot ===");

  // Valve pin: set LOW before enabling output to ensure the
  // valve is closed from the very first clock cycle.
  digitalWrite(VALVE_PIN, LOW);
  pinMode(VALVE_PIN, OUTPUT);

  // The nRF51822 GPIO standard drive is ~0.5 mA — far too low
  // to source enough current through a 470 Ω base resistor.
  // High-drive mode (H0H1) raises the limit to ~5 mA, which
  // is the minimum needed to saturate the TIP121G at 500 mA
  // collector current. This must be set after pinMode().
  NRF_GPIO->PIN_CNF[VALVE_PIN] =
    (NRF_GPIO->PIN_CNF[VALVE_PIN] & ~GPIO_PIN_CNF_DRIVE_Msk)
    | (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos);

  // LED pins: outputs, start HIGH (off, active LOW).
  digitalWrite(LED_RED,   HIGH); pinMode(LED_RED,   OUTPUT);
  digitalWrite(LED_GREEN, HIGH); pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_BLUE,  HIGH); pinMode(LED_BLUE,  OUTPUT);

  blink(LED_GREEN, 3);    // 3 blinks: boot complete
  Serial.println("Cycling — valve open/closed every 20s");
}

// ---- Loop ---------------------------------------------------

void loop() {
  openValve();
  ledOn(LED_BLUE);
  Serial.println("Valve open");
  delay((unsigned long)VALVE_ON_S * 1000UL);

  closeValve();
  ledOff(LED_BLUE);
  ledOn(LED_GREEN);
  Serial.println("Valve closed");
  delay((unsigned long)VALVE_OFF_S * 1000UL);
  ledOff(LED_GREEN);
}
