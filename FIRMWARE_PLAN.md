# Firmware Plan: Plant Watering System

MicroPython on ESP32. Single solenoid valve, gravity-fed, NTP-scheduled, WiFi-accessible.

---

## 1. One-Time Machine Setup

Install the two CLI tools you'll use throughout development:

```bash
pip install esptool mpremote
```

Download the MicroPython `.bin` for ESP32 from micropython.org → Downloads → ESP32 → **ESP32_GENERIC** (latest stable). Save it somewhere accessible, e.g. `~/Downloads/micropython-esp32.bin`.

---

## 2. Flash MicroPython onto the ESP32

Do this once. After this the board runs MicroPython permanently until you reflash it.

### 2.1 Find your serial port

Plug in the ESP32 via USB, then:

```bash
ls /dev/tty.*
```

Look for something like `/dev/tty.usbserial-XXXX` or `/dev/tty.SLAB_USBtoUART`. That's your port. Set it as a variable for convenience:

```bash
PORT=/dev/tty.usbserial-XXXX
```

### 2.2 Erase the flash

```bash
esptool.py --chip esp32 --port $PORT erase_flash
```

### 2.3 Flash MicroPython

```bash
esptool.py --chip esp32 --port $PORT --baud 460800 \
  write_flash -z 0x1000 ~/Downloads/micropython-esp32.bin
```

### 2.4 Verify

```bash
mpremote connect $PORT
```

You should see a `>>>` Python REPL prompt. Press Enter if nothing appears. Type `import sys; sys.version` to confirm MicroPython is running. Press Ctrl-X to exit.

---

## 3. Project File Structure

Create a `firmware/` directory in this repo. All files in `firmware/` get uploaded to the ESP32's root filesystem.

```
watering_system/
└── firmware/
    ├── config.py       # all user-configurable values (WiFi, GPIO, schedule, timing)
    ├── wifi_connect.py # connect to WiFi with retry
    ├── ntp_sync.py     # sync clock via NTP after WiFi connects
    ├── valve.py        # open/close the solenoid valve via GPIO
    ├── scheduler.py    # watering loop: check time, trigger valve
    └── main.py         # entry point — called automatically on boot
```

---

## 4. Module Design

### 4.1 `config.py`

All values that need tuning live here. Nothing else is hardcoded.

```python
WIFI_SSID     = "your_network"
WIFI_PASSWORD = "your_password"

VALVE_GPIO    = 18        # GPIO pin driving the transistor base
FILL_DURATION = 10        # seconds to hold valve open (tune during calibration)
VALVE_MAX_ON  = 60        # hard safety cap: valve never stays open longer than this

WATER_HOUR    = 7         # water at 07:00 local time
WATER_MINUTE  = 0
UTC_OFFSET    = -5        # hours offset from UTC (e.g. -5 for EST, -7 for MST)

STATUS_LED_GPIO = 2       # onboard LED GPIO (typically GPIO2 on most ESP32 dev boards)
```

### 4.2 `wifi_connect.py`

Connects to WiFi with a retry loop. Returns when connected or raises after timeout.

```python
import network
import time
import config

def connect():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(config.WIFI_SSID, config.WIFI_PASSWORD)
    for _ in range(20):           # 10-second timeout
        if wlan.isconnected():
            return
        time.sleep(0.5)
    raise RuntimeError("WiFi connection failed")
```

### 4.3 `ntp_sync.py`

Syncs the ESP32's internal clock to NTP. Must be called after WiFi is connected.

```python
import ntptime
import time
import config

def sync():
    ntptime.settime()             # sets RTC to UTC
    # MicroPython's utime.localtime() does not apply UTC_OFFSET automatically.
    # Add the offset when comparing in scheduler.py.
```

Note: MicroPython's `utime.localtime()` always returns UTC after `ntptime.settime()`. Apply `config.UTC_OFFSET` in the scheduler when checking the hour.

### 4.4 `valve.py`

Controls the solenoid valve GPIO. Enforces the safety cap from config.

```python
from machine import Pin
import time
import config

_valve = Pin(config.VALVE_GPIO, Pin.OUT, value=0)

def open_valve():
    _valve.value(1)

def close_valve():
    _valve.value(0)

def water_cycle():
    duration = min(config.FILL_DURATION, config.VALVE_MAX_ON)
    open_valve()
    time.sleep(duration)
    close_valve()
```

### 4.5 `scheduler.py`

Main loop. Checks time every minute; fires a watering cycle once per day at the configured hour.

```python
import utime
import config
import valve

def local_hour_minute():
    t = utime.localtime()
    hour = (t[3] + config.UTC_OFFSET) % 24
    minute = t[4]
    return hour, minute

def run():
    last_watered_day = -1
    while True:
        hour, minute = local_hour_minute()
        day = utime.localtime()[2]
        if hour == config.WATER_HOUR and minute == config.WATER_MINUTE:
            if day != last_watered_day:
                valve.water_cycle()
                last_watered_day = day
        utime.sleep(30)           # check twice per minute
```

### 4.6 `main.py`

Entry point. Runs on every boot. Connects WiFi, syncs NTP, starts scheduler.

```python
from machine import Pin
import config
import wifi_connect
import ntp_sync
import scheduler

led = Pin(config.STATUS_LED_GPIO, Pin.OUT)

# Blink to indicate boot
for _ in range(3):
    led.value(1); __import__('time').sleep(0.2)
    led.value(0); __import__('time').sleep(0.2)

wifi_connect.connect()
ntp_sync.sync()

led.value(1)      # solid on = running
scheduler.run()   # blocks forever
```

---

## 5. Uploading Files to the ESP32

Upload all files in `firmware/` to the board's root:

```bash
PORT=/dev/tty.usbserial-XXXX

mpremote connect $PORT cp firmware/config.py       :config.py
mpremote connect $PORT cp firmware/wifi_connect.py :wifi_connect.py
mpremote connect $PORT cp firmware/ntp_sync.py     :ntp_sync.py
mpremote connect $PORT cp firmware/valve.py        :valve.py
mpremote connect $PORT cp firmware/scheduler.py    :scheduler.py
mpremote connect $PORT cp firmware/main.py         :main.py
```

Or upload everything at once:

```bash
for f in firmware/*.py; do
  mpremote connect $PORT cp "$f" ":$(basename $f)"
done
```

Verify what's on the board:

```bash
mpremote connect $PORT ls
```

### Iterative workflow

During development, re-upload only the file you changed:

```bash
mpremote connect $PORT cp firmware/scheduler.py :scheduler.py
```

Then soft-reboot to reload:

```bash
mpremote connect $PORT exec "import machine; machine.reset()"
```

---

## 6. Testing Sequence

Work through these in order. Do not proceed to the next step until the current one passes.

### Step 1 — REPL sanity check

```bash
mpremote connect $PORT
>>> import config
>>> config.VALVE_GPIO
18
```

### Step 2 — WiFi

```bash
>>> import wifi_connect
>>> wifi_connect.connect()
>>> import network; network.WLAN(network.STA_IF).ifconfig()
```
Should return an IP address.

### Step 3 — NTP

```bash
>>> import ntp_sync; ntp_sync.sync()
>>> import utime; utime.localtime()
```
Confirm the time matches current UTC.

### Step 4 — Valve GPIO (no water connected)

Probe GPIO pin with a multimeter or LED:

```bash
>>> import valve
>>> valve.open_valve()   # pin should go HIGH (3.3V)
>>> valve.close_valve()  # pin should go LOW (0V)
```

### Step 5 — Valve GPIO with transistor circuit wired

Repeat Step 4 with the transistor driver circuit connected to the GPIO pin but the valve not yet plumbed. Confirm the transistor switches (measure collector voltage: ~0V when on, ~12V when off).

### Step 6 — First live water test (short fill duration)

Set `FILL_DURATION = 2` in config, upload, then trigger manually:

```bash
>>> import valve; valve.water_cycle()
```

Confirm valve opens (clicks), water flows into manifold, valve closes. Check for leaks.

### Step 7 — Fill duration calibration

Increase `FILL_DURATION` until all intermediate reservoirs reach capacity without overflow. Time it visually. Set this value in config.

### Step 8 — Full scheduled cycle

Set `WATER_HOUR` and `WATER_MINUTE` to 2–3 minutes from now. Upload, reboot, watch LED go solid. Wait for the trigger and confirm the cycle fires once and only once.

### Step 9 — Boot reliability

Unplug and replug the USB power. Confirm `main.py` runs automatically, WiFi reconnects, LED goes solid, and scheduler resumes without any manual intervention.

---

## 7. Deployment Checklist

Before leaving the system running unattended:

- [ ] `FILL_DURATION` calibrated and confirmed
- [ ] `WATER_HOUR` / `WATER_MINUTE` set to intended time
- [ ] `UTC_OFFSET` correct for your timezone (check DST if applicable)
- [ ] Valve GPIO pin confirmed correct for your specific ESP32 board
- [ ] Transistor driver circuit assembled and tested
- [ ] All plumbing leak-tested
- [ ] Gravity reservoir full
- [ ] Power supply connected to 12V rail and ESP32
- [ ] ESP32 boots to solid LED without USB attached (powered from LM2596)
- [ ] One complete unattended cycle observed and verified

---

## 8. Ongoing Maintenance

**Updating firmware:** Edit file locally → `mpremote cp` → `mpremote exec "import machine; machine.reset()"`.

**Changing the schedule:** Edit `WATER_HOUR` / `WATER_MINUTE` in `config.py`, re-upload `config.py` only, reboot.

**Changing fill duration:** Edit `FILL_DURATION` in `config.py`, re-upload, reboot.

**Checking logs:** `mpremote connect $PORT` gives a live REPL. The scheduler loop runs in the foreground so you'll see print output in real time during development.

**WiFi drops:** The current design does not auto-reconnect if WiFi drops after boot. If this proves to be an issue, add a reconnect check inside the scheduler loop before each time comparison.

---

## 9. Future Extensions (Not In Scope Now)

- Web interface served from ESP32 (`uasyncio` + `microdot` or raw socket) to view status and update schedule without re-uploading files
- Auto-reconnect loop in `wifi_connect.py`
- Watchdog timer (`machine.WDT`) to auto-reboot if firmware hangs
- Soil moisture sensor inputs to skip watering if soil is still wet
- Low-reservoir alert via WiFi notification
