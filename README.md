# Automatic Plant Watering System

An automatic watering system for 8 potted plants. A single 12V solenoid valve releases water from a gravity-fed reservoir into a drip manifold. Each plant has its own Luer-slip syringe intermediate reservoir sized to hold exactly that plant's target water dose. When the valve opens all reservoirs fill simultaneously; when it closes, water drips slowly from each reservoir to its plant over 1–3 minutes. A Seeed Tiny BLE (nRF51822) controls the valve on a fixed 24-hour interval.

Per-plant water volume is controlled **physically** by syringe plunger position, not by software. The firmware only opens and closes the valve.

**Status:** Electronics assembled and firmware running. Valve cycle confirmed. Power path via barrel jack still being resolved. Enclosure CAD started.

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
  → solenoid valve (via TIP121G NPN Darlington transistor switch)
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
- Programmed via CMSIS-DAP drag-and-drop: Sketch → Export Compiled Binary in Arduino IDE, drag `.hex` to the MBED drive in Finder. Board package: sandeepmistry nRF5, Generic nRF51.
- GPIO P4 drives the valve; nRF51822 requires H0H1 high-drive mode via direct NRF_GPIO register write (firmware handles this)
- LED pins (confirmed on hardware): GREEN = 21, RED = 22, BLUE = 23. Active LOW.

### Transistor driver

**TIP121G NPN Darlington** (TO-220), 470Ω base resistor, **10kΩ base pull-down to GND**, 1N4007 flyback diode.

- TIP121G is a Darlington with h_FE ≥ 1000 — nRF51822 H0H1 GPIO sources ~5mA, which saturates the TIP121G at 5A; the 500mA solenoid is trivial. The original ZTX650 was replaced because its h_FE of ~50–80 at 500mA was insufficient for saturation with 5mA base drive, causing it to operate in the linear region and overheat.
- Pinout (TO-220, leads facing you): B – C – E left to right; metal tab = collector
- 470Ω limits base current and accounts for the Darlington's two V_BE drops (~1.4V total): I_B = (3.3 − 1.4) / 470 ≈ 4mA
- **10kΩ pull-down** holds the base at 0V when the GPIO is inactive — a floating base picks up noise, partially enables the transistor, and generates destructive inductive spikes (this destroyed the first ZTX650 on initial power-up with the MCU disconnected)
- 1N4007 flyback diode clamps the inductive spike when the valve closes (cathode to 12V rail)
- No heatsink required — V_CE(sat) ≈ 1V at 500mA gives ~500mW dissipation, well within TO-220 limits

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

**`firmware_ble/`** — Arduino C++ targeting Seeed Tiny BLE (nRF51822). Currently set to alternating open/closed cycle for calibration.

Key firmware constants (edit in `firmware_ble/watering_system/watering_system.ino`):

```cpp
#define VALVE_PIN    4   // GPIO pin to transistor base
#define VALVE_ON_S  20   // seconds valve stays open each cycle
#define VALVE_OFF_S 20   // seconds valve stays closed each cycle
```

LED: blue while valve is open, green while closed. 3 green blinks on boot.

---

## Bill of materials

### Electronics

| Item | Cost | Notes |
|---|---|---|
| Seeed Tiny BLE (nRF51822) | $0 (owned) | Microcontroller |
| 12V DC solenoid valve, NC, 1/2" NPT, VITON | $35 | U.S. Solid brass, direct-acting |
| TIP121G NPN Darlington transistor (TO-220) | $0 (stash) | Valve driver |
| 470Ω resistor (1/4W) | $0 (stash) | Base resistor |
| 10kΩ resistor (1/4W) | $0 (stash) | Base pull-down to GND |
| 1N4007 diode | $0 (stash) | Flyback protection |
| Meanwell GST60A12 12V/5A PSU | $0 (reused) | Continuous duty, UL/CE |
| LM2596 buck converter module (5-pack) | $9 | 12V→5V; screw terminal inputs |
| IEC C8 panel-mount socket | $0 (stash) | Power entry on enclosure |
| Perfboard, jumper wire, screw terminals | $0 (stash) | Driver circuit board |

### Plumbing

| Item | Cost | Notes |
|---|---|---|
| HDPE 2.5-gal black bucket + Gamma-seal lid | $18 | Food-grade, opaque |
| 1/2" NPT bulkhead fitting + EPDM gasket | — | Reservoir outlet (included in fittings total) |
| Copper mesh, 100–200 mesh, food-safe | $13 | Basket inside bucket over inlet |
| 1/2" NPT-F × 1/2" barb adapter, brass (×1) | — | Bulkhead → supply tube (included in fittings total) |
| 1/2" NPT-M × 1/2" barb adapter, brass (×3) | — | Valve inlet, valve outlet, manifold inlet (included in fittings total) |
| Bulkhead + barb adapters ×4 + PTFE tape | $25 | All fittings combined |
| Orbit 67000 8-port drip manifold | $18 | 1/2" FPT inlet, 1/4" barb outlets (bought 2-pack, 1 spare) |
| Silicone tubing 1/2" ID × 3/4" OD, 10 ft | — | Main supply runs (included in tubing total) |
| Silicone tubing 1/4" ID × 1/2" OD, 25 ft | — | Fill lines (included in tubing total) |
| Silicone tubing 1/8" ID × 1/4" OD, 50 ft | — | Drip lines (included in tubing total) |
| Silicone tubing (all 3 spools) | $35 | |

### Intermediate reservoirs

| Item | Cost | Notes |
|---|---|---|
| 150ml Luer-slip syringes (×6) | $23 | All plants; no small syringes needed |
| Cable zip ties (600pc) | $13 | Lock syringe plungers |
| Aquarium-grade silicone sealant | — | Included in sealant + pegboard total |
| Pegboard (syringe mounting, replaces rack) | — | Included in sealant + pegboard total |
| Silicone sealant + pegboard | $35 | |

### Enclosure

| Item | Cost | Notes |
|---|---|---|
| 3D-printed body + lid | $2 | Filament only; own printer |

### Cost summary

| Category | Cost |
|---|---|
| Electronics | $44 |
| Plumbing | $109 |
| Intermediate reservoirs | $71 |
| Enclosure | $2 |
| **Total** | **~$226** |

---

## Open items

- Resolve barrel-jack power path: LM2596 currently outputs 5V but Tiny BLE VCC pin expects 3.7–4.2V (LiPo range); either trim LM2596 to 4V or route 5V via USB injection permanently
- Update firmware from calibration cycle (20s on/off) to production schedule (24h interval) once fill duration is confirmed
- Determine per-plant water volumes → select small syringe sizes and quantities
- Calibrate valve open duration: increase VALVE_ON_S until all intermediate reservoirs fill without overflow
- Measure LM2596 module dimensions → model corner extrusions in CAD
- Finalize solenoid-side detachable connector (DIN 43650A preferred)
- Complete enclosure CAD and print
- Full system test
