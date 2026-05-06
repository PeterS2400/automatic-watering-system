// =============================================================
// Watering System — Seeed Tiny BLE (nRF51822)
// =============================================================
// Single solenoid valve, gravity-fed, 8-plant manifold.
// No WiFi. Interval-based timing: waters every 24 hours.
// Waters immediately on first boot so you can confirm the
// valve works, then every WATER_INTERVAL_MS after that.
// =============================================================

// ---- Configuration (edit these) -----------------------------

// GPIO pin wired to the ZTX650/651 base via 470 Ω resistor.
// Any free digital output pin works. Verify against your wiring.
#define VALVE_PIN 4

// How long to hold the valve open each cycle, in seconds.
// Start at 2 and calibrate upward until all intermediate
// reservoirs reach capacity without overflow.
#define FILL_DURATION_S 2

// Hard safety cap. Valve will never stay open longer than this
// regardless of FILL_DURATION_S.
#define VALVE_MAX_ON_S 60

// Watering interval. 24 hours expressed in milliseconds.
// millis() is an unsigned long (32-bit), so UL suffixes are
// required here — without them the multiplication overflows
// before the compiler widens the type.
#define WATER_INTERVAL_MS (24UL * 60UL * 60UL * 1000UL)

// Seeed Tiny BLE RGB LED pin numbers. Active LOW — write LOW
// to turn on, HIGH to turn off. Verify against your board's
// silkscreen if colours are wrong.
#define LED_RED   21
#define LED_GREEN 22
#define LED_BLUE  23

// ---- Globals ------------------------------------------------

unsigned long lastWatered = 0;

// ---- LED helpers --------------------------------------------

void ledOn(int pin)  { digitalWrite(pin, LOW);  }
void ledOff(int pin) { digitalWrite(pin, HIGH); }

// Blink a single LED n times. Non-blocking alternative not
// needed here — valve is the only time-sensitive operation.
void blink(int pin, int n, int onMs = 200, int offMs = 200) {
  for (int i = 0; i < n; i++) {
    ledOn(pin);  delay(onMs);
    ledOff(pin); delay(offMs);
  }
}

// ---- Valve control ------------------------------------------

void openValve()  { digitalWrite(VALVE_PIN, HIGH); }
void closeValve() { digitalWrite(VALVE_PIN, LOW);  }

void waterCycle() {
  int duration = min(FILL_DURATION_S, VALVE_MAX_ON_S);

  Serial.print("Opening valve for ");
  Serial.print(duration);
  Serial.println("s");

  ledOn(LED_BLUE);
  openValve();
  delay((unsigned long)duration * 1000UL);
  closeValve();
  ledOff(LED_BLUE);

  Serial.println("Valve closed — cycle complete");
}

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
  // is the minimum needed to saturate the ZTX650/651 at 500 mA
  // collector current. This must be set after pinMode().
  NRF_GPIO->PIN_CNF[VALVE_PIN] =
    (NRF_GPIO->PIN_CNF[VALVE_PIN] & ~GPIO_PIN_CNF_DRIVE_Msk)
    | (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos);

  // LED pins: outputs, start HIGH (off, active LOW).
  digitalWrite(LED_RED,   HIGH); pinMode(LED_RED,   OUTPUT);
  digitalWrite(LED_GREEN, HIGH); pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_BLUE,  HIGH); pinMode(LED_BLUE,  OUTPUT);

  blink(LED_GREEN, 3);    // 3 blinks: boot complete

  waterCycle();           // immediate first-boot watering
  lastWatered = millis();

  ledOn(LED_GREEN);       // solid green: running normally
  Serial.println("Running — next water cycle in 24h");
}

// ---- Loop ---------------------------------------------------

void loop() {
  // Unsigned subtraction is rollover-safe: millis() wraps
  // after ~49 days, but the difference remains correct as long
  // as the interval is less than 49 days. 24h is well within
  // that limit.
  if (millis() - lastWatered >= WATER_INTERVAL_MS) {
    waterCycle();
    lastWatered = millis();
  }

  delay(1000); // check once per second; accuracy is ±1s, fine for daily watering
}
