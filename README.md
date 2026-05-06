# Automatic Plant Watering System

An automatic watering system for 8 potted plants. A single 12V solenoid valve releases water from a gravity-fed reservoir into a drip manifold. Each plant has its own Luer-slip syringe intermediate reservoir sized to hold exactly that plant's target water dose. When the valve opens all reservoirs fill simultaneously; when it closes, water drips slowly from each reservoir to its plant over 1–3 minutes. A Seeed Tiny BLE (nRF51822) controls the valve on a fixed 24-hour interval.

Per-plant water volume is controlled **physically** by syringe plunger position, not by software. The firmware only opens and closes the valve.

**Status:** Electronics assembly in progress; enclosure CAD started.

---

## How it works

```
Gravity reservoir (2.5-gal HDPE bucket, ~1m above plants)
  [copper mesh basket inside, over bulkhead inlet]
  → 1/2" bulkhead → 1/2" ID silicone supply line
  → 12V solenoid valve (normally closed)
  → 1/2" ID silicone feed line
  → Orbit 67000 8-port drip manifold
  → 8× 1/4" ID fill lines → Luer-slip syringe intermediate reservoirs
  → 8× 1/8" ID silicone drip lines → plants
```

```
120V AC → Meanwell 12V/5A PSU → 12V rail
  → solenoid valve (via ZTX650/651 NPN transistor switch)
  → LM2596 buck converter → 5V → Seeed Tiny BLE (USB injection)
```

---

## Architecture

Two approaches were evaluated before committing to hardware.

| | Option A — single valve (chosen) | Option B — per-plant valves |
|---|---|---|
| Valves | 1 | 8 |
| Electronics complexity | Low | Medium |
| Adjusting per-plant amounts | Slide syringe plunger | Change a config value |
| Plumbing complexity | Medium (intermediate reservoirs) | Low (direct drip lines) |

**Why Option A:** fewer failure points, simpler electronics, and volume adjustment doesn't require reflashing firmware. The intermediate reservoir approach is visible and debuggable — you can watch each reservoir fill and drain.

---

## Hardware

### Microcontroller

**Seeed Tiny BLE (nRF51822)** — ARM Cortex-M0, 16MHz, 3.3V logic.

- Timing: `millis()`-based 24-hour interval; no WiFi, no NTP, no RTC
- Drift: ~2–4s/day — acceptable for daily plant watering
- Programmed via CMSIS-DAP drag-and-drop (compile in Arduino IDE, `cp` the `.bin` to the MBED drive — macOS Finder drag-and-drop is unreliable)
- GPIO P4 drives the valve; nRF51822 requires H0H1 high-drive mode via direct NRF_GPIO register write (firmware handles this)

### Transistor driver

ZTX650/651 NPN BJT, 470Ω base resistor, **10kΩ base pull-down to GND**, 1N4007 flyback diode.

- 470Ω limits GPIO current to ~7mA and drives the transistor into saturation
- **10kΩ pull-down** holds the base at 0V when the GPIO is inactive — a floating base picks up noise, partially enables the transistor, and generates destructive inductive spikes (this destroyed the first ZTX650 on initial power-up with the MCU disconnected)
- 1N4007 flyback diode clamps the inductive spike when the valve closes (cathode to 12V rail)

### Power

**Meanwell GST60A12** — 12V/5A desktop brick, UL/CE certified, continuous duty.

- 12V feeds solenoid coil (~500mA peak) and LM2596 input
- LM2596 buck converter steps down to 5V for Tiny BLE via USB injection; module uses screw terminals — Meanwell barrel jack connects via pigtail, no adapter needed

### Solenoid valve

12V DC, normally closed, 1/2" NPT, VITON seal (U.S. Solid brass body).

- **Direct-acting** — opens at zero pressure; works with gravity feed at any head height (pilot-operated valves require ~0.3 bar minimum pressure — avoid those)
- 1/2" NPT chosen over 1/4": the 1/2" valve has a ~10–12mm orifice so tubing, not the valve, is the flow bottleneck

### Intermediate reservoirs

All 8 plants use Luer-slip syringes with the plunger as an adjustable volume limiter.

- Fill tube (1/4" ID) drilled through center of plunger disc — interference fit + silicone sealant
- Plunger locked at desired volume mark with a cable tie through the barrel ears
- Drip line (1/8" ID) push-fits onto Luer nozzle — no barb fitting needed
- Volume adjustment: loosen cable tie, slide plunger to new mark, re-tighten

### Tubing

| Use | Spec |
|---|---|
| Supply runs (bulkhead → valve, valve → manifold) | 1/2" ID × 3/4" OD silicone, 10 ft |
| Fill lines (manifold → reservoirs) | 1/4" ID × 1/2" OD silicone, 25 ft |
| Drip lines (reservoirs → plants) | 1/8" ID × 1/4" OD silicone, 50 ft |

### Enclosure

3D-printed, two-part: body + lid. Lid secured with screws. Single parting line shared between the lid joint and both cable glands.

- **Power entry:** IEC C8 panel-mount socket; 12V DC pigtail from Meanwell barrel jack inside
- **Solenoid cable entry:** 2-wire, 12V switched; detachable connector inside (DIN 43650A preferred)
- **USB access:** remove lid — no panel port

Cable glands are fully integrated into the print. The body/lid split is the gland split — screwing the lid down clamps the cable. No purchased glands or separate clamp parts.

- Power cable bore: 5.9mm (cable OD 6.35mm)
- Solenoid cable bore: 2.8mm (cable OD 3.175mm)

Interior dimensions: 65mm × 85mm floor, 45mm height to parting line.

Boards mount via L-shaped corner extrusions from the walls. Bottom 2 corners are full L-shape (supports weight); top 2 corners are side tabs only (allows straight insertion from above, no overhangs to print). Boards sit slightly lower than their natural wire resting point so wire tension presses them into the supports.

---

## Firmware

Two implementations exist — same valve logic, different microcontroller:

- **`firmware_ble/`** — Arduino C++ targeting Seeed Tiny BLE (nRF51822). 24h interval timing. Currently in use.
- **`firmware/`** — MicroPython targeting ESP32. NTP-scheduled, WiFi-connected. Deferred until an ESP32 is acquired.

Key firmware constants (edit in `firmware_ble/watering_system/watering_system.ino`):

```cpp
#define VALVE_PIN        4      // GPIO pin to transistor base
#define FILL_DURATION_S  2      // seconds to hold valve open — calibrate upward
#define VALVE_MAX_ON_S   60     // hard safety cap
```

The valve opens immediately on first boot (confirms the circuit works), then every 24 hours after that.

---

## Bill of materials

### Electronics

| Item | Notes |
|---|---|
| Seeed Tiny BLE (nRF51822) | Microcontroller |
| 12V DC solenoid valve, NC, 1/2" NPT, VITON | U.S. Solid brass, direct-acting |
| ZTX650 or ZTX651 NPN transistor | Valve driver |
| 470Ω resistor (1/4W) | Base resistor |
| 10kΩ resistor (1/4W) | Base pull-down to GND |
| 1N4007 diode | Flyback protection |
| DIN 43650A connector + lead wire | Mates with solenoid coil |
| Meanwell GST60A12 12V/5A PSU | Continuous duty, UL/CE |
| LM2596 buck converter module | 12V→5V; screw terminal inputs |
| IEC C8 panel-mount socket | Power entry on enclosure |
| Perfboard, jumper wire, screw terminals | Driver circuit board |

### Plumbing

| Item | Notes |
|---|---|
| HDPE 2.5-gal black bucket | Food-grade, opaque |
| Gamma-seal lid | Fits 2.5-gal bucket |
| 1/2" NPT HDPE bulkhead fitting + EPDM gasket | Reservoir outlet |
| Copper mesh, 100–200 mesh, food-safe | Basket inside bucket over inlet |
| 1/2" NPT-F × 1/2" barb adapter, brass (×1) | Bulkhead → supply tube |
| 1/2" NPT-M × 1/2" barb adapter, brass (×3) | Valve inlet, valve outlet, manifold inlet |
| Orbit 67000 8-port drip manifold | 1/2" FPT inlet, 1/4" barb outlets |
| Silicone tubing 1/2" ID × 3/4" OD, 10 ft | Main supply runs |
| Silicone tubing 1/4" ID × 1/2" OD, 25 ft | Fill lines |
| Silicone tubing 1/8" ID × 1/4" OD, 50 ft | Drip lines |
| PTFE tape | Seal NPT threads |

### Intermediate reservoirs

| Item | Notes |
|---|---|
| Oral Luer-slip syringes, 10–60ml | Small plants |
| 150ml Luer-slip syringes (×6 ordered) | Large plants |
| Cable zip ties | Lock syringe plungers |
| Aquarium-grade silicone sealant | Seal fill tube through plunger hole |
| Syringe holder rack | Mount syringes above plants |

---

## Open items

- Determine per-plant water volumes → select small syringe sizes and quantities
- Measure LM2596 module dimensions → model corner extrusions in CAD
- Finalize solenoid-side detachable connector (DIN 43650A preferred)
- Complete enclosure CAD and print
- Calibrate valve open duration for all reservoirs to fill without overflow
- Full system test
