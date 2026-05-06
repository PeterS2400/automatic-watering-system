import network
import time
import config


def connect():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)

    if wlan.isconnected():
        print("WiFi already connected:", wlan.ifconfig()[0])
        return

    print("Connecting to WiFi:", config.WIFI_SSID)
    wlan.connect(config.WIFI_SSID, config.WIFI_PASSWORD)

    checks = config.WIFI_TIMEOUT_S * 2  # poll every 0.5 s
    for _ in range(checks):
        if wlan.isconnected():
            print("WiFi connected:", wlan.ifconfig()[0])
            return
        time.sleep(0.5)

    raise RuntimeError("WiFi connection timed out after {}s".format(config.WIFI_TIMEOUT_S))
