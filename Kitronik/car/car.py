radio.on()
radio.set_group(7)

def on_forever():
    radio.send_string("on")
    basic.pause(5)

basic.forever(on_forever)