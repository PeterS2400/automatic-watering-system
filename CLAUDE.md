# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Automatic plant watering system for 8 potted plants in the same room. A single solenoid valve is controlled by an ESP32 to release water from a gravity-fed reservoir into an 8-port manifold that fills a set of intermediate reservoirs — one per plant. Different-sized intermediate reservoirs provide different water volumes per plant. Thin tubing from each intermediate reservoir drips slowly into its plant after the valve closes. All plants are watered simultaneously on a fixed schedule. WiFi is built in for remote schedule access and status monitoring.

## Hardware

- **Microcontroller:** ESP32 development board (built-in WiFi)
- **Valve:** 12V solenoid valve, NC, 1/2" NPT, VITON seal (~500mA coil)
- **Transistor driver:** ZTX650/651 NPN BJT, 470Ω base resistor, driven from GPIO (3.3V logic)
  - 10kΩ pull-down resistor from base to GND — holds base at 0V when GPIO is not driving it; prevents floating base from partially turning on the transistor and causing destructive inductive spikes
  - 1N4007 flyback diode across solenoid coil (cathode to 12V, anode to GND)
- **Power supply:** Meanwell GST60A12 12V/5A desktop brick (UL/CE, continuous duty, 5.5×2.5mm barrel jack)
  - LM2596 buck converter steps 12V down to 5V for ESP32
- **Water source:** Gravity-fed reservoir — black food-grade HDPE 2.5-gal bucket + Gamma-seal lid + 1/2" NPT bulkhead fitting (EPDM gasket)
- **Distribution:**
  - Copper mesh basket (100–200 mesh) inside bucket over bulkhead inlet — filters sediment and provides antimicrobial Cu²⁺ release; no inline strainer
  - Supply runs: 1/2" ID × 3/4" OD silicone, bulkhead → valve inlet and valve outlet → manifold inlet (full-bore 1/2" throughout)
  - 8-port off-shelf drip manifold (e.g. Orbit/Rain Bird, 1/2" FPT inlet, 1/4" barb outlets)
  - Fill lines: 1/4" ID × 1/2" OD silicone, manifold 1/4" barb outlets → intermediate reservoir tops (fast fill, ~8s for 500ml at 1m head)
  - Drip lines: 1/8" ID × 1/4" OD silicone, intermediate reservoir outlet → plant (1–3 min delivery)
- **Intermediate reservoirs (per-plant volume control):**
  - Luer-slip syringes for all plants: smaller syringes (≤60ml) for lower-volume plants, 150ml syringes for higher-volume plants
  - Plunger acts as adjustable fill ceiling; 1/4" ID fill tube passed directly through drilled hole in plunger disc (interference fit + silicone sealant); plunger locked with cable tie through barrel ears; 1/8" ID drip line push-fits onto Luer nozzle tip (no barb fittings)
- **Enclosure:** 3D-printed

## Firmware

Two implementations exist — same valve logic, different microcontroller:

- **`firmware_ble/`** — Arduino C++ (`.ino`) targeting Seeed Tiny BLE (nRF51822). Interval-based timing (24h); no WiFi, no NTP. Currently in use.
- **`firmware/`** — MicroPython (`.py`) targeting ESP32. NTP-scheduled, WiFi-connected. Deferred until ESP32 is acquired.

Core logic (both): open valve → wait for intermediate reservoirs to fill → close valve. Schedule is configurable; soil moisture sensors are a planned future addition.

## Key Design Constraint

Per-plant water volume is controlled physically by intermediate reservoir size, not by software. The firmware only controls valve open/close timing — it does not need to differentiate between plants.

## Open Items

- Determine per-plant water volumes → finalize syringe sizes and quantities (6× 150ml ordered; small syringes still needed)
- ~~Confirm manifold product outlet type (1/4" barb vs. 1/4" NPT-F)~~ **Resolved: 1/4" barb universally.** Selected: Orbit 67000 (see manifold_options.md).
- ~~Confirm barrel jack adapter need (5.5×2.5mm Meanwell → 5.5×2.1mm if needed)~~ **Resolved: not applicable.** LM2596 module uses screw terminals for 12V input — the Meanwell barrel jack connects via a pigtail wire to the perfboard, no socket adapter needed.
