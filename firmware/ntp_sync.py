import ntptime
import utime


def sync(retries=3, retry_delay_s=3):
    """Sync ESP32 RTC to UTC via NTP. Requires WiFi to be connected first.

    After this call, utime.localtime() returns UTC. Apply config.UTC_OFFSET
    in the scheduler when comparing against local WATER_HOUR/WATER_MINUTE.
    """
    for attempt in range(1, retries + 1):
        try:
            ntptime.settime()
            t = utime.localtime()
            print("NTP sync OK — UTC: {:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}".format(
                t[0], t[1], t[2], t[3], t[4], t[5]))
            return
        except Exception as e:
            print("NTP sync attempt {}/{} failed: {}".format(attempt, retries, e))
            if attempt < retries:
                utime.sleep(retry_delay_s)

    raise RuntimeError("NTP sync failed after {} attempts".format(retries))
