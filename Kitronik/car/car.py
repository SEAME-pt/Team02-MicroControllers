radio.on()
radio.set_group(7)


#play with radio transmission power - strenght rssi increrses more easly detectable maybe too easly 
# radio.set_transmit_power(7)

def on_forever():
    radio.send_string("on")
    basic.pause(50)

basic.forever(on_forever)