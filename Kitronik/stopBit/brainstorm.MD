**Decentralized Aproach for traffic lights**

 - Intersection Cluster of Traffic Lights named "cluster 1" , token = "C1" each traffic light ID in front = "C1A"

 **Logic:**
 - All traffic lights start 🔴.
 - All traffic lights start listning for cluster Token and own ID.
 - Each traffic light A, B, C, are waiting to start a turn (listening).
 - [A] by default starts its own turn in the first run.
 - Traffic light(s) with the matching ID to Token received starts Turn.
 - Turn takes 2 seconds listening for the Car brodcasted "on" signal.
 - When the Turn ends Broadcasts cluster Token and next ID in line.

 - During a turn it checks for the Car broadcasted "on" RSSI (BOOL) and - IF:
    ⤷ True -> Turns 🟢 for a period ( 10* seconds) then 🟠 -> 🔴 and passes the turn.
    ⤷ False -> listens remaining time then passes Turn.


 - Activation [token received] -> [token sent] 
    A: ["C1A"] -> ["C1B"]
    B: ["C1B"] -> ["C1C"]
    C: ["C1C"] -> ["C1A"]

    *(A starts turn when turned ON then needs the activation as all others)*

- For multiple intersections on the LAB:
    - Each cluster token should be different.
    - String like: "Cluster 1 start A turn" = "C1A"
    
**How:**
    - Current logic is checking for RSSI strength all the time.
    - Change all traffic lights to listen for a token(ID) (string via radio) all the time.
    - Traffic light receives token -> checks it and in case there is no match keeps waiting for the correct token.
    - Turn all trafficlight microbits the last should be A and start its own Turn by default (otherwise no turn is ever activated).
    - During its turn, checks for CAR RSSI either going 🟢 or passing the turn after 2 seconds without activation.
    - Looping through all trafic lights in that given intersection Group.
    - 2 Traffic lights can have the same Token and ID to go 🟢 in parallel.

**Current:**
    - Only one traffic light 🟢 at time.
    - Better for lab settings and simpler for first testing aproach.
    - Allows for further improvement, knowing wich traffic lights can be green at the same time and other schemes,
    - All Traffic lights 🔴 otherwise.
    - We can add the functionality for timed intervals of 🔴 and 🟢 lights in coordination for each specific trafficlight in the intersection when pressing the microbit buttons (should show on the leds the current operating Mode).

    Turn order example:
    *A -> B -> C -> A*   or    A/C -> B/D and other combinations for different intersection types and needs.

**Improvements:**
    - Add a failsafe/watchdog - to handles dropped tokens or Micro:bits going offline mid-turn.
    - Track last token time - Detect stalls and re-inject the token if loop breaks. 
    - Manual override mode, `button A` to start the loop sending token `button B` to cycle operating modes.
    - Mode visualization on the Microbit Leds.
