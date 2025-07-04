# User Input Functionality

def on_button_pressed_a():
    global lightColor
    if lightColor == "":
        set_light(True, False, False)
        lightColor = "RED"
        return
    if lightColor == "RED":
        set_light(False, True, False)
        lightColor = "YELLOW"
        return
    if lightColor == "YELLOW":
        set_light(False, False, True)
        lightColor = "GREEN"
        return
    if lightColor == "GREEN":
        set_light(False, False, False)
        lightColor = "OFF"
        return
    if lightColor == "OFF":
        set_light(True, True, True)
        lightColor = "ALL"
        return
    if lightColor == "ALL":
        set_light(True, False, True)
        lightColor = "RG"
        return
    if lightColor == "RG":
        set_light(True, True, False)
        lightColor = "RY"
        return
    if lightColor == "RY":
        set_light(False, True, True)
        lightColor = "YG"
        return
    else:
        lightColor = ""
input.on_button_pressed(Button.A, on_button_pressed_a)

def set_light(red: bool, yellow: bool, green: bool):
    Kitronik_STOPbit.traffic_light_led(Kitronik_STOPbit.LightColours.RED,
        Kitronik_STOPbit.DisplayLights.ON if red else Kitronik_STOPbit.DisplayLights.OFF)
    Kitronik_STOPbit.traffic_light_led(Kitronik_STOPbit.LightColours.YELLOW,
        Kitronik_STOPbit.DisplayLights.ON if yellow else Kitronik_STOPbit.DisplayLights.OFF)
    Kitronik_STOPbit.traffic_light_led(Kitronik_STOPbit.LightColours.GREEN,
        Kitronik_STOPbit.DisplayLights.ON if green else Kitronik_STOPbit.DisplayLights.OFF)
lightColor = ""
basic.show_string("Press A to change lights on")

def on_forever():
    basic.pause(30)
basic.forever(on_forever)
