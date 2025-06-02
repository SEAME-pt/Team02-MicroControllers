def on_received_string(receivedString):
    global rssi, last_signal_time
    rssi = radio.received_packet(RadioPacketProperty.SIGNAL_STRENGTH)
    if receivedString == "on" and rssi > rssi_thr:
        last_signal_time = input.running_time()

radio.on_received_string(on_received_string)

def set_light(red: bool, yellow: bool, green: bool):
    Kitronik_STOPbit.traffic_light_led(Kitronik_STOPbit.LightColours.RED,
        Kitronik_STOPbit.DisplayLights.ON if red else Kitronik_STOPbit.DisplayLights.OFF)
    Kitronik_STOPbit.traffic_light_led(Kitronik_STOPbit.LightColours.YELLOW,
        Kitronik_STOPbit.DisplayLights.ON if yellow else Kitronik_STOPbit.DisplayLights.OFF)
    Kitronik_STOPbit.traffic_light_led(Kitronik_STOPbit.LightColours.GREEN,
        Kitronik_STOPbit.DisplayLights.ON if green else Kitronik_STOPbit.DisplayLights.OFF)

# Setup
rssi = 0
rssi_thr = -50
last_signal_time = 0
state = ""

radio.on()
radio.set_group(7)

def on_forever():
    global state
    now = input.running_time()
    time_since = now - last_signal_time

    if time_since < 5000 and state != "green":
        set_light(False, False, True)
        state = "green"

    elif time_since < 8000 and time_since >= 5000 and state != "yellow":
        set_light(False, True, False)
        state = "yellow"

    elif time_since >= 11000 and state != "red":
        set_light(True, False, False)
        state = "red"

    basic.pause(10)

basic.forever(on_forever)