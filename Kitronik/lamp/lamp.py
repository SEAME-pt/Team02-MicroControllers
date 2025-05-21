# def on_received_string(receivedString):
#     global lamp_on, lamp_on_time
#     if receivedString == "L" + lamp_id + "ON":
#         pins.digital_write_pin(DigitalPin.P0, 1)
#         # turn lamp ON
#         lamp_on = True
#         lamp_on_time = input.running_time()
#         basic.show_leds("""
#             # # # # #
#             # # # # #
#             # # # # #
#             # # # # #
#             # # # # #
#             """)
# radio.on_received_string(on_received_string)

# lamp_on_time = 0
# lamp_on = False
# lamp_id = ""
# radio.on()
# radio.set_group(7)
# # If you're setting this up in a new lamp, increase the ID
# # Lamp send format is L01  (01 is the ID)
# # Lamp receive format is L01ON to turn the lamp on
# lamp_id = "01"
# # Initially off
# pins.digital_write_pin(DigitalPin.P1, 0)
# basic.show_leds("""
#     # # # # #
#     # . . . #
#     # . . . #
#     # . . . #
#     # # # # #
#     """)

# def on_forever():
#     global lamp_on
#     # Broadcast identity
#     radio.send_string("L" + lamp_id)
#     # Auto-off after 5 seconds
#     if lamp_on and input.running_time() - lamp_on_time > 2000:
#         pins.digital_write_pin(DigitalPin.P0, 0)
#         # turn lamp OFF
#         lamp_on = False
#         basic.clear_screen()
#         basic.show_leds("""
#             # # # # #
#             # . . . #
#             # . . . #
#             # . . . #
#             # # # # #
#             """)
#     basic.pause(1)
# basic.forever(on_forever)



radio.on()
radio.set_group(7)

lamp_on_time = 0
lamp_on = False
lamp_id = "01"
rssi_thr = -45

# Initially off
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
        pins.digital_write_pin(DigitalPin.P0, 1)
        lamp_on = True
        lamp_on_time = input.running_time()
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
    # Broadcast identity
    radio.send_string("L" + lamp_id)

    # Auto-off after 2 seconds
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
