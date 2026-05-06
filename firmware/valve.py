import time
from machine import Pin
import config


# value=0 in the constructor sets the pin LOW before enabling the output driver,
# ensuring the valve is closed for the entire boot sequence, not just after
# this module finishes initializing.
_valve = Pin(config.VALVE_GPIO, Pin.OUT, value=0)


def open_valve():
    _valve.value(1)


def close_valve():
    _valve.value(0)


def water_cycle():
    duration = min(config.FILL_DURATION, config.VALVE_MAX_ON)
    print("Opening valve for {}s (cap: {}s)".format(duration, config.VALVE_MAX_ON))
    try:
        open_valve()
        time.sleep(duration)
    finally:
        # finally guarantees the valve closes even if a KeyboardInterrupt
        # or other exception interrupts the sleep.
        close_valve()
    print("Valve closed — cycle complete")
