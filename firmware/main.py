import time
import machine
from machine import Pin
import config
import wifi_connect
import ntp_sync
import scheduler


led = Pin(config.STATUS_LED_GPIO, Pin.OUT, value=0)


def blink(n, on_s=0.2, off_s=0.2):
    for _ in range(n):
        led.value(1)
        time.sleep(on_s)
        led.value(0)
        time.sleep(off_s)


try:
    print("=== Watering system boot ===")
    blink(3)                  # 3 blinks: boot started

    wifi_connect.connect()
    blink(2)                  # 2 blinks: WiFi connected

    ntp_sync.sync()
    blink(1)                  # 1 blink: time synced

    led.value(1)              # solid on: running normally
    scheduler.run()           # blocks forever

except Exception as e:
    # On any unhandled error: log it, turn off LED, wait, then hard-reset.
    # The 5-second delay prevents a tight reboot loop when the fault is
    # persistent (e.g. wrong WiFi password).
    print("FATAL:", e)
    led.value(0)
    time.sleep(5)
    machine.reset()
