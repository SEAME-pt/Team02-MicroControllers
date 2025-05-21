# from microbit import radio

# def on_received_string(receivedString):
#     global rssi, lamp, gate, strongest_rssi
#     rssi = radio.received_packet(RadioPacketProperty.SIGNAL_STRENGTH)
#     if (receivedString == "I am lamp1") and rssi > rssi_thr:
#         lamp = 1
#     elif(receivedString == "I am gate1") and rssi > rssi_thr:
#         gate = 1
# radio.on_received_string(on_received_string)

# current_time = 0
# lamp = 0
# gate = 0
# rssi = 0
# rssi_thr = 0
# strongest_rssi = 0
# radio.on()
# radio.set_transmit_power(7)
# radio.set_group(7)
# strongest_rssi = -100
# rssi_thr = -45
# last_check = input.running_time()
# check_interval = 10

# def on_forever():
#     global current_time, strongest_rssi, last_check, lamp, gate
#     current_time = input.running_time()
#     if current_time - last_check > check_interval:
#         if lamp == 1:
#             basic.show_string("L")
#             radio.send_string("" + "lamp1" + " on")
#         elif gate == 1:
#             basic.show_string("G")
#             radio.send_string("" + "gate1" + " on")
#         # Zera para próxima medição
#         strongest_gate = ""
#         strongest_rssi = -100
#         gate = 0
#         lamp = 0
#         last_check = current_time
# basic.forever(on_forever)


radio.on()
radio.set_group(7)
radio.set_transmit_power(7)

def on_forever():
    radio.send_string("on")
    basic.pause(10)

basic.forever(on_forever)