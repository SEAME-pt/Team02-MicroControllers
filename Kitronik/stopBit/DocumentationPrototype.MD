# 🚦 Decentralized Traffic Light System - Clusters

This project implements a decentralized traffic light system using Micro:bit devices and the Kitronik STOP:bit module. Traffic lights operate in clusters and coordinate turns via radio communication, ensuring, responsive traffic control with car detection through RSSI.

---

## Features

- Decentralized turn based control using token-passing protocol.
- Car detection via RSSI on unit listening.
- Auto/Manual modes.
- Manual triggers microbit to pass the token.
- Failsafe if token gets stuck.
- LED display for mode & signal strength.

---

## System Overview

- Each traffic light has a unique ID (e.g., `C1A`, `C1B`, `C1C`).
- A token is passed among them in sequence.
- Only the unit with the token can turn green.
- The green light activates only if a car is detected nearby (via strong RSSI).
- Fast listening Turns allow for all the units to cicle in a short period.

---

## Button Functions

| Button     | Function                       |
|------------|--------------------------------|
| A          | Send your ID (Manual mode only)|
| B          | Toggle Auto / Manual mode      |
| A + B      | Show RSSI strength             |

---

### Mode Display

- `"A"` shown on screen = Auto Mode
- `"M"` shown on screen = Manual Mode
- `"F"` = Failsafe activated

### 🛠️ Dependencies

- Micro:bit
- Kitronik STOP:bit
