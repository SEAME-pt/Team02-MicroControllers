radio.on()
radio.set_group(7)
radio.set_transmit_power(7)

rssi_thr = -45
last_check = input.running_time()
check_interval = 10
device_detected = ""

def on_received_string(receivedString):
    global device_detected
    rssi = radio.received_packet(RadioPacketProperty.SIGNAL_STRENGTH)
    if rssi > rssi_thr:
        device_detected = receivedString  # Save full string like "G01" or "L01"

radio.on_received_string(on_received_string)

def on_forever():
    global device_detected, last_check
    now = input.running_time()

    if now - last_check > check_interval:
        if device_detected:
            radio.send_string(device_detected + "ON")  # e.g., G01ON
            basic.show_string(device_detected.char_at(0))  # Show first letter like "G"
            device_detected = ""

        last_check = now

    basic.pause(5)

basic.forever(on_forever)