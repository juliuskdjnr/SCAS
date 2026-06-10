# Smart Classroom Automation System
 
An embedded systems project that automates classroom lighting and ventilation based on real-time occupancy detection — built on a bare **ATmega328P** microcontroller with a custom PCB, no Arduino framework.
 
---
 
## Problem
 
Classrooms in many institutions waste energy by keeping lights and fans running whether or not students are present. Manual control relies on individuals remembering to switch systems off — which rarely happens consistently. This project replaces that manual process with an automated, sensor-driven system.
 
---
 
## Solution
 
A self-contained embedded system that:
- Detects occupancy using PIR sensors
- Automatically switches classroom lighting via a relay module
- Controls ventilation speed via a DC motor
- Displays real-time system status on an I2C LCD
- Runs a fully non-blocking firmware state machine — no `delay()` calls
---
 
## System Architecture
 
```
PIR Sensor(s)
     │
     ▼
ATmega328P ──── Relay Module ──── Lighting Load
     │
     ├────────── DC Motor (Ventilation)
     │
     └────────── I2C LCD (Status Display)
```
 
---
 
## Hardware Components
 
| Component | Description |
|---|---|
| ATmega328P | Main microcontroller (bare chip, no Arduino bootloader) |
| PIR Sensor | Passive infrared occupancy detection |
| Relay Module | Switches mains-level lighting load |
| DC Motor | Drives classroom ventilation/fan |
| I2C LCD (16x2) | Real-time status display |
| Custom PCB | Designed and fabricated for this project |
 
---
 
## Firmware Overview
 
The firmware is written in **C/C++** and runs directly on the ATmega328P. Key design decisions:
 
### Non-blocking State Machine
Instead of using `delay()` — which blocks the entire MCU and prevents parallel task handling — the firmware uses a `millis()`-based timing approach. Each subsystem (occupancy check, relay control, motor control, LCD update) runs independently on its own timer interval.
 
```c
// Example: non-blocking task scheduling pattern
uint32_t lastOccupancyCheck = 0;
const uint32_t OCCUPANCY_INTERVAL = 500; // ms
 
void loop() {
    uint32_t now = millis();
 
    if (now - lastOccupancyCheck >= OCCUPANCY_INTERVAL) {
        lastOccupancyCheck = now;
        checkOccupancy();
    }
 
    updateRelay();
    updateMotor();
    updateLCD();
}
```
 
### Occupancy Logic
- PIR sensor output is read at regular intervals
- On occupancy detected → relay closes (lights ON), motor activates (fan ON)
- On vacancy (after a configurable hold time) → relay opens, motor stops
- LCD updates to reflect current state in real time
### I2C LCD Communication
The LCD is driven over I2C (2-wire interface), freeing up GPIO pins compared to a parallel connection. Status messages are written on state transitions only, avoiding unnecessary bus traffic.
 
---
 
## PCB Design
 
The system was designed on a custom PCB rather than a breadboard or development shield, covering:
- ATmega328P with supporting passive components (crystal, decoupling caps, reset circuit)
- Relay driver circuit with flyback diode protection
- Motor driver stage
- I2C header for LCD connection
- PIR sensor interface headers
> 📁 Schematic and PCB layout exports are in the [`/schematics`](./schematics) folder.
 
---
 
## Project Structure
 
```
smart-classroom-automation/
├── firmware/
│   ├── main.c           # Entry point and main loop
│   ├── occupancy.c/h    # PIR sensor logic
│   ├── relay.c/h        # Relay switching logic
│   ├── motor.c/h        # DC motor control
│   ├── lcd.c/h          # I2C LCD driver
│   └── timer.c/h        # millis() implementation
├── schematics/
│   ├── schematic.png    # Circuit schematic export
│   └── pcb_layout.png   # PCB layout export
├── docs/
│   └── ieee_report.pdf  # Full IEEE-format project report
├── images/
│   └── ...              # Build photos
└── README.md
```
 
---
 
## Build & Flash
 
**Toolchain:** AVR-GCC (manual compilation)  
**Programmer:** GALEP-5 universal device programmer
 
```bash
# Compile
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -o main.elf firmware/main.c
 
# Convert to hex
avr-objcopy -O ihex main.elf main.hex
 
# Flash to chip (USBasp programmer)
avrdude -c usbasp -p m328p -U flash:w:main.hex
```
 
---

### Flash
The `.hex` file was flashed onto the ATmega328P using the **GALEP-5** universal device programmer via its accompanying software (Conitec GALEP). Fuse bits were also configured through GALEP-5 to set the correct clock source and disable the default clock divider.
 
---
 
## Team
 
Built by a team of 4 students as a semester capstone project at **Academic City University, Accra, Ghana** (April 2026).
 
---
 
## Documentation
 
The full system design, methodology, testing, and results are documented in an IEEE-format technical report — see [`/docs/ieee_report.pdf`](./docs/ieee_report.pdf).
