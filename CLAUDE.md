# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Automatic plant watering system for 5–8 potted plants in the same room. A single solenoid valve is controlled by an Arduino to release water from a gravity-fed reservoir into a manifold that fills a set of intermediate reservoirs — one per plant. Different-sized intermediate reservoirs provide different water volumes per plant. Thin tubing from each intermediate reservoir drips slowly into its plant after the valve closes. All plants are watered simultaneously on a fixed schedule.

## Hardware

- **Microcontroller:** Arduino (Uno or Nano)
- **Valve:** 12V solenoid valve (~500mA coil)
- **Transistor driver:** ZTX650/651 NPN BJT, 470Ω base resistor, driven from Arduino GPIO (5V logic)
  - ΔU = 4V across resistor → iB = ~5mA, β_typ = 100, iC = 500mA
  - A flyback diode across the solenoid coil is required
- **Water source:** Gravity-fed reservoir (no pump)
- **Distribution:** Single manifold → individual intermediate reservoirs (sized per plant) → thin drip tubes to each plant
- **Power:** Mains (wall outlet); 12V DC for solenoid, 5V for Arduino

## Firmware

- Language: Arduino C++ (`.ino`)
- Scheduling: RTC module (e.g. DS3231) for time-based triggering
- Core logic: open valve → wait for intermediate reservoirs to fill → close valve
- Schedule is fixed (configurable in code); soil moisture sensors are a planned future addition

## Key Design Constraint

Per-plant water volume is controlled physically by intermediate reservoir size, not by software. The firmware only controls valve open/close timing — it does not need to differentiate between plants.
