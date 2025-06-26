# Team02-MicroControllers

This repository contains code and resources developed by Team 02 for microcontroller-based systems intalled on the Jetson Car. 
The project includes both Arduino logic (written in C++) and micro:bit components with working python code.

## Arduino Folder

The `Arduino/` folder contains the C++ source code for microcontrollers using the PlatformIO environment along with testing code. The main logic is located in `src/main.cpp`:

- Read sensor inputs
- Control of LED Car lights beams and blinkers.
- Uses CAN-Bus comunication protocol.

This part of the project handles the embedded logic that runs on Arduino boards.

---

## Kitronik Folder

The `Kitronik/` folder includes Python scripts and HEX files for micro:bit devices that simulate or control different modules:

- **accessGate**: Controls a gate barrier checking car proximity.
- **lamp**: Simulates a road lamp that turns on when a car is nearby.
- **car**: Continuosly broadcasts a signal to simulate a car approaching the system units.
- **stopBit**: Manages the traffic lights logic and comunication for intersections.

Precompiled `.hex` files are included for direct flashing to micro:bit devices, and images help illustrate how the system is wired and connected.

---


## Repo Folder Structure

```
.
├── Arduino/                  # Arduino codebase (PlatformIO project)
│   ├── include/              # Header files (currently placeholder)
│   ├── lib/                  # External libraries (currently placeholder)
│   ├── src/                  # Main Arduino code
│   │   └── main.cpp
│   ├── test/                 # Tests (currently placeholder)
│   └── platformio.ini        # PlatformIO configuration
│
├── Kitronik/                 # Python scripts and micro:bit hex files
│   ├── accessGate/          # Controls for barrier
│   │   └── barrier.py
│   │
│   ├── car/                 # Car broadcaster logic
│   │   └── car.py
│   │
│   ├── hex/                 # Precompiled micro:bit HEX files
│   │   ├── microbit-accessGate.hex
│   │   ├── microbit-carBroadcaster.hex
│   │   ├── microbit-lampReceiver.hex
│   │   └── microbit-stopbit.hex
│   │
│   ├── lamp/                # Lamp receiver logic
│   │   └── lamp.py
│   │
│   ├── stopBit/             # Stop bit controller
│   │   └── stopbit.py
│   │
│   └── images/              # System diagrams and images
│       ├── accessgate.jpg.webp
│       ├── lamp.webp
│       ├── microbitscheme.png
│       ├── mirobitlegend.png
│       └── stop.jpg.webp
│
├── CMakeLists.txt            # Project configuration (placeholder)
└── README.md                 # This file
```

