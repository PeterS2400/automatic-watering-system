# WiFi credentials
WIFI_SSID     = "your_network"
WIFI_PASSWORD = "your_password"
WIFI_TIMEOUT_S = 20

# GPIO pins
# GPIO 18 is a safe output pin on all common ESP32 dev boards.
# GPIO 2 is the onboard LED on most 30-pin/38-pin DOIT dev boards (active HIGH).
# Verify both against your specific board's pinout before wiring.
VALVE_GPIO      = 18
STATUS_LED_GPIO = 2

# Valve timing (seconds)
# FILL_DURATION: how long the valve stays open each watering cycle.
# Set to 2 initially; calibrate upward until all intermediate reservoirs reach capacity.
FILL_DURATION = 2
VALVE_MAX_ON  = 60   # hard safety cap — valve never stays open longer than this

# Schedule (local time)
WATER_HOUR   = 7    # 0–23
WATER_MINUTE = 0    # 0–59

# UTC offset for your timezone.
# Examples: EST -5, CST -6, MST -7, PST -8, CET +1, JST +9
# Adjust for DST manually when clocks change.
UTC_OFFSET = -5
