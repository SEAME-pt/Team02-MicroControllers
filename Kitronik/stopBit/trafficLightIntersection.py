#################################################################################
#                                                                               #
#                      ****    RULES / STANDARDS    ****                        #
#                                                                               #
#################################################################################

# Cluster 1 is a group of Traffic lights that work together and comunicate.
# It is required for all the Units of this intersection to be on the same
#cluster and Radio Group.
# Each unit should have a unique name / ID.
# Ex: 3 Traffic lights:  A, B and C   in the cluster 1:
# A ID is "Cluster1 A " so we call it "C1A" and it wants to pass the turn to B
# thus NEXT_ID should be: "C1B"
# The last element of the group should pass the turn back to A.

#################################################################################

ID = "C1A"
NEXT_ID = "C1B"
CAR_ID = "on"


#radio Group = Radio Frequency Channel
GROUP = 7
# each listening for activation token turn duration 2s
TURN_DURATION = 600
# time the Green light is kept on 8s orange 2s
GREEN_DURATION = 2000
YELLOW_DURATION = 1000

# Radio Signal Strength Indicator
RSSI_THRESHOLD = -44
MANUAL_LIGHT = ""

# if after 15 seconds the turn is stuck this will terminate it
# security redundancy
FAILSAFE_TIME = 15000

# program variables
rssi = 0
last_token = ""
last_signal_time = 0
state = "red"
my_turn = False
turn_start_time = 0
auto_mode = True
#car detected status
detected = False
elapsed = 0

def on_received_string(token):
    global my_turn, last_token, turn_start_time, rssi, last_signal_time, detected
    last_token = token

    if my_turn == False and token == ID:
        my_turn = True
        turn_start_time = input.running_time()

    #only measure RSSI if the detected signal is the car
    if my_turn and last_token == CAR_ID:
        rssi = radio.received_packet(RadioPacketProperty.SIGNAL_STRENGTH)
        if rssi > RSSI_THRESHOLD:
            last_signal_time = input.running_time()
            detected = True


#traffic lights setter
def set_light(red: bool, yellow: bool, green: bool):
    Kitronik_STOPbit.traffic_light_led(Kitronik_STOPbit.LightColours.RED,
        Kitronik_STOPbit.DisplayLights.ON if red else Kitronik_STOPbit.DisplayLights.OFF)
    Kitronik_STOPbit.traffic_light_led(Kitronik_STOPbit.LightColours.YELLOW,
        Kitronik_STOPbit.DisplayLights.ON if yellow else Kitronik_STOPbit.DisplayLights.OFF)
    Kitronik_STOPbit.traffic_light_led(Kitronik_STOPbit.LightColours.GREEN,
        Kitronik_STOPbit.DisplayLights.ON if green else Kitronik_STOPbit.DisplayLights.OFF)

def show_mode():
    if auto_mode:
        basic.show_string("A")
    else:
        basic.show_string("M")

def on_forever():
    global my_turn, state, turn_start_time, detected, elapsed
    now = input.running_time()

    if my_turn:
        elapsed = now - turn_start_time

        # check for car during TURN_DURATION
        if elapsed < TURN_DURATION:
            if detected == True:
                set_light(False, False, True)
                state = "green"
                basic.pause(GREEN_DURATION)
                set_light(False, True, False)
                state = "yellow"
                basic.pause(YELLOW_DURATION)
                detected = False
            
            elapsed = now - turn_start_time
            
        # After TURN_DURATION or after car detection loop end turn
        if elapsed >= TURN_DURATION:
            set_light(True, False, False)
            state = "red"
            my_turn = False
            detected = False
                
            # send token NEXT_ID for x milliseconds (redundancy)
            send_start = input.running_time()
            while input.running_time() - send_start <= 150:
                radio.send_string(NEXT_ID)
                basic.pause(20)

    basic.pause(12)
    #show_mode()


last_switch_time = 0

def cycle_lights_non_blocking():
    global last_switch_time, state

    now = input.running_time()
    if now - last_switch_time >= 10000:
        if state == "red":
            set_light(False, False, True)
            state = "green"
        elif state == "green":
            set_light(False, True, False)
            state = "yellow"
        elif state == "yellow":
            set_light(True, False, False)
            state = "red"
        last_switch_time = now

# button A starts the token loop manually
def on_button_a_pressed():
    global detected
    detected = False
    basic.show_number(1)
    radio.send_string(NEXT_ID)
    show_mode()


# A&B pressed: Shows RSSI, then manually cycles traffic lights (green → yellow → red)
def on_button_ab_pressed():
    global MANUAL_LIGHT

    if MANUAL_LIGHT == "":
        basic.show_number(rssi)
        MANUAL_LIGHT = "rssi"
        return

    if MANUAL_LIGHT == "rssi":
        basic.show_string("L")
        cycle_lights_non_blocking()
        MANUAL_LIGHT = ""
        show_mode()

# Main Program:

# Set radio group and turn antena on.
radio.on()
radio.set_group(GROUP)
radio.on_received_string(on_received_string)

# User Input Functionality
input.on_button_pressed(Button.A, on_button_a_pressed)
input.on_button_pressed(Button.AB, on_button_ab_pressed)

# start showing current mode on boot
show_mode()
set_light(True, False, False)
basic.forever(on_forever)
