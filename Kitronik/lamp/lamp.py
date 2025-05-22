radio.on()
radio.set_group(7)

lamp_on_time = 0
lamp_on = False
lamp_id = "01"
rssi_thr = -50


pins.digital_write_pin(DigitalPin.P0, 0)
basic.show_leds("""
    # # # # #
    # . . . #
    # . . . #
    # . . . #
    # # # # #
""")

def on_received_string(receivedString):
    global lamp_on, lamp_on_time
    rssi = radio.received_packet(RadioPacketProperty.SIGNAL_STRENGTH)
    if receivedString == "on" and rssi > rssi_thr:
        lamp_on_time = input.running_time()
        if not lamp_on:
            pins.digital_write_pin(DigitalPin.P0, 1)
            lamp_on = True
            basic.show_leds("""
                # # # # #
                # # # # #
                # # # # #
                # # # # #
                # # # # #
            """)

radio.on_received_string(on_received_string)

def on_forever():
    global lamp_on


    if lamp_on and input.running_time() - lamp_on_time > 2000:
        pins.digital_write_pin(DigitalPin.P0, 0)
        lamp_on = False
        basic.clear_screen()
        basic.show_leds("""
            # # # # #
            # . . . #
            # . . . #
            # . . . #
            # # # # #
        """)

    basic.pause(10)

basic.forever(on_forever)
