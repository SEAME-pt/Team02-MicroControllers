radio.on()
radio.set_group(7)

gate_open = False
gate_open_time = 0
rssi_thr = -45

# Initially closed
pins.servo_write_pin(AnalogPin.P0, 90)
basic.show_leds("""
    . # # # .
    # # # # #
    # . . . #
    # # # # #
    . # # # .
""")

def on_received_string(receivedString):
    global gate_open, gate_open_time
    rssi = radio.received_packet(RadioPacketProperty.SIGNAL_STRENGTH)
    if receivedString == "on" and rssi > rssi_thr:
        gate_open_time = input.running_time()
        if not gate_open:
            pins.servo_write_pin(AnalogPin.P0, 0) 
            gate_open = True
            basic.show_leds("""
                . . # . .
                . # # # .
                . . # . .
                . . # . .
                . . # . .
            """)

radio.on_received_string(on_received_string)

def on_forever():
    global gate_open, gate_open_time
    if gate_open and input.running_time() - gate_open_time > 1500:
        pins.servo_write_pin(AnalogPin.P0, 90)
        gate_open = False
        basic.clear_screen()
        basic.show_leds("""
            . # # # .
            # # # # #
            # . . . #
            # # # # #
            . # # # .
        """)
    basic.pause(5)

basic.forever(on_forever)
