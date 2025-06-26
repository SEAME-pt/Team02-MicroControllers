
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
TURN_DURATION = 2000
# time the Green light is kept on 10s
GREEN_DURATION = 10000
YELLOW_DURATION = 3000
# Radio Signal Strength Indicator
RSSI_THRESHOLD = -50

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


def on_received_string(token):
    global my_turn, last_token, turn_start_time
    last_token = token

    if my_turn == False:
        if token == ID:
            my_turn = True
            turn_start_time = input.running_time()


# Checking for proximity only if the message is "on" meaning it is the car not any signal
def check_car_nearby():
    global rssi, last_signal_time
    message = radio.receive_string()

    #only measure RSSI if the detected signal is the car
    if message == CAR_ID:
        rssi = radio.received_packet(RadioPacketProperty.SIGNAL_STRENGTH)
        if rssi > RSSI_THRESHOLD:
            last_signal_time = input.running_time()
            return True
    return False


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
    global my_turn, state, turn_start_time, detected
    now = input.running_time()

    if my_turn:
        elapsed = now - turn_start_time

        # Failsafe
        if elapsed > FAILSAFE_TIME:
            my_turn = False
            set_light(True, False, False)
            state = "red"
            basic.show_string("F")
            detected = False
            return

        # check for car during TURN_DURATION
        if elapsed < TURN_DURATION and detected == False:
            if check_car_nearby():
                detected = True
                set_light(False, False, True)
                state = "green"
                basic.pause(GREEN_DURATION)
                set_light(False, True, False)
                state = "yellow"
                basic.pause(YELLOW_DURATION)

        # After TURN_DURATION or after car detection loop end turn
        elif elapsed >= TURN_DURATION:
            set_light(True, False, False)
            state = "red"

            # send token NEXT_ID for x milliseconds (redundancy)
            send_start = input.running_time()
            while input.running_time() - send_start < 1000:
                radio.send_string(NEXT_ID)
                basic.pause(200)
            my_turn = False
            detected = False
            return

    basic.pause(50)

# button A starts the token loop manually
def on_button_a_pressed():
    global detected
    if not auto_mode:
        radio.send_string(ID)
        detected = False

# Button B toggles auto/manual mode
def on_button_b_pressed():
    global auto_mode
    auto_mode = not auto_mode
    show_mode()

# A and B pressed Shows RSSI
def on_button_ab_pressed():
    basic.show_number(rssi)

# Main Program:
# Set radio group and turn antena on.
radio.set_group(GROUP)
radio.on()
radio.on_received_string(on_received_string)

# User Input Functionality
input.on_button_pressed(Button.A, on_button_a_pressed)
input.on_button_pressed(Button.B, on_button_b_pressed)
input.on_button_pressed(Button.AB, on_button_ab_pressed)

# start showing current mode on boot
show_mode()
basic.forever(on_forever)
