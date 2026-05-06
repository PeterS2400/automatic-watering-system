import utime
import config
import valve


def _local_time():
    """Return (local_hour, local_minute, utc_day).

    utime.localtime() always returns UTC after ntptime.settime().
    We apply UTC_OFFSET to get local hour for schedule comparisons.
    utc_day is used as the deduplicate key for last_watered_day — this is
    correct in practice because the watering time (07:00 local) is far
    from midnight UTC in any timezone likely to be used.
    """
    t = utime.localtime()
    local_hour = (t[3] + config.UTC_OFFSET) % 24
    local_minute = t[4]
    utc_day = t[2]
    return local_hour, local_minute, utc_day


def run():
    last_watered_day = -1
    print("Scheduler running — will water at {:02d}:{:02d} local time".format(
        config.WATER_HOUR, config.WATER_MINUTE))

    while True:
        hour, minute, day = _local_time()

        if hour == config.WATER_HOUR and minute == config.WATER_MINUTE:
            if day != last_watered_day:
                print("Watering trigger: local {:02d}:{:02d}, UTC day {}".format(
                    hour, minute, day))
                valve.water_cycle()
                last_watered_day = day

        utime.sleep(30)  # poll twice per minute; window is 1 minute wide
