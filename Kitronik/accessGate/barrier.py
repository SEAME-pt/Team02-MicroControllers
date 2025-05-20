radio.on()
radio.set_group(7)

gate_open = False
gate_open_time = 0

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
    if receivedString == "G01ON":
        pins.servo_write_pin(AnalogPin.P0, 0)  # open
        gate_open = True
        gate_open_time = input.running_time()
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
    # Broadcast identity
    radio.send_string("G01")

    # Auto-close after 5 seconds
    if gate_open and input.running_time() - gate_open_time > 2000:
        pins.servo_write_pin(AnalogPin.P0, 90)  # close
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
