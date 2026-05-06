# Automatic Plant Watering System — Project Snapshot

**Last updated:** 2026-05-06
**Status:** Electronics assembly in progress; enclosure CAD started

---

## What this is

An automatic watering system for 8 potted plants in one room. A single 12V solenoid valve releases water from a gravity-fed reservoir into a drip manifold. Each plant has its own Luer-slip syringe intermediate reservoir — sized to hold exactly that plant's target water dose. When the valve opens, all reservoirs fill simultaneously; when it closes, water drips slowly from each reservoir to its plant over 1–3 minutes. A Seeed Tiny BLE (nRF51822) controls the valve on a fixed 24-hour interval.

Per-plant water volume is controlled **physically** (by syringe plunger position), not by software. The firmware only opens and closes the valve.

---

## Architecture decision: single valve vs. per-plant valves

| | Option A — single valve (chosen) | Option B — per-plant valves |
|---|---|---|
| Valves | 1 | 8 |
| Electronics complexity | Low | Medium |
| Adjusting per-plant amounts | Slide syringe plunger | Change a config value |
| Plumbing complexity | Medium (intermediate reservoirs) | Low (direct drip lines) |
| Firmware complexity | Low | Low–Medium |

**Why Option A was chosen:** fewer failure points, simpler electronics, and volume adjustment doesn't require reflashing firmware — just sliding a syringe plunger. The intermediate reservoir approach is also visible and debuggable: you can watch each reservoir fill and drain.

---

## System diagram

```
Gravity reservoir (2.5-gal HDPE bucket, ~1m above plants)
  [copper mesh basket inside, over bulkhead inlet]
  → bulkhead fitting (1/2" NPT)
  → [1/2" NPT-F × 1/2" barb] → 1/2" ID silicone → [1/2" NPT-M × 1/2" barb]
  → solenoid valve (12V NC, 1/2" NPT)
  → [1/2" NPT-M × 1/2" barb] → 1/2" ID silicone → [1/2" NPT-M × 1/2" barb]
  → 8-port drip manifold (Orbit 67000, 1/2" FPT inlet)
  → 8× 1/4" ID fill lines → Luer-slip syringe intermediate reservoirs
  → 8× 1/8" ID silicone drip lines → plants
```

Electronics:
```
120V AC → Meanwell 12V/5A PSU → 12V rail
  → solenoid valve (via ZTX650/651 transistor switch)
  → LM2596 buck converter → 5V → Seeed Tiny BLE (USB injection)
```

---

## Key design decisions

### Microcontroller — Seeed Tiny BLE (nRF51822)

The original plan targeted an ESP32 with WiFi and NTP scheduling. The Seeed Tiny BLE was substituted because it was already on hand.

- ARM Cortex-M0, 16MHz, 3.3V logic
- Firmware: Arduino C++ (`firmware_ble/watering_system/watering_system.ino`)
- Timing: `millis()`-based 24-hour interval — no WiFi, no NTP, no RTC
- Drift: ~2–4s/day, acceptable for daily plant watering
- Programmed via CMSIS-DAP drag-and-drop: compile in Arduino IDE, `cp` the `.bin` to the MBED drive (macOS Finder drag-and-drop is unreliable — use Terminal `cp`)
- GPIO P4 drives the valve; nRF51822 requires H0H1 high-drive mode set via direct NRF_GPIO register write — firmware handles this
- RGB LED (active LOW): RED=21, GREEN=22, BLUE=23
- Powered via 5V USB injection from LM2596 output

The ESP32 / MicroPython / NTP path is preserved in `firmware/` and deferred until an ESP32 is acquired.

---

### Intermediate reservoirs — Luer-slip syringes for all plants

All 8 plants use Luer-slip syringes with the plunger as an adjustable volume limiter.

- **Small plants (≤60ml):** oral syringes, 10–60ml
- **Large plants:** 150ml large syringes (6× ordered; quantities of small sizes TBD)
- Fill tube (1/4" ID) drilled through center of plunger disc — interference fit + silicone sealant
- Plunger locked at desired volume mark with a cable tie through the barrel ears
- Drip line (1/8" ID) push-fits onto Luer nozzle — no barb fitting needed
- Volume adjustment: loosen cable tie, slide plunger to new mark, re-tighten

*Earlier snapshot noted PP wide-mouth lab bottles for large plants — that approach was replaced by 150ml syringes for all plants.*

---

### Tubing — three sizes

| Use | Spec | Why |
|---|---|---|
| Supply runs (bulkhead → valve, valve → manifold) | 1/2" ID × 3/4" OD silicone, 10 ft | Full-bore 1/2" throughout; covers both segments with margin |
| Fill lines (manifold → reservoirs) | 1/4" ID × 1/2" OD silicone, 25 ft | Direct fit on manifold 1/4" barb outlets; fills 500ml in <10s at 1m head |
| Drip lines (reservoirs → plants) | 1/8" ID × 1/4" OD silicone, 50 ft | ~200ml drains in 40–60s at 0.2m head; 1–3 min delivery range |

All silicone — stays flexible indefinitely; vinyl/PVC hardens over 2–3 years.

---

### Manifold — off-shelf drip irrigation

**Selected:** Orbit 67000 (8-port, ~$8). 1/2" FPT inlet, eight 1/4" barb outlets. All major off-shelf 8-port drip manifolds use 1/4" barb outlets — direct fit for 1/4" ID fill lines. Individual rotary flow controls; open fully for gravity feed.

---

### Solenoid valve

**Selected:** 12V DC, normally closed, 1/2" NPT, VITON seal (U.S. Solid brass body).

- Normally closed = safe default (stays shut on power loss)
- **Direct-acting:** opens at zero pressure — compatible with gravity feed at any head height
- VITON seal: compatible with hydrogen peroxide water treatment
- 1/2" NPT chosen over 1/4": the 1/2" valve has a ~10–12mm orifice so tubing, not the valve, is the flow bottleneck

---

### Transistor driver

**Circuit:** ZTX650/651 NPN BJT, 470Ω base resistor, 10kΩ base pull-down to GND, 1N4007 flyback diode.

- 470Ω limits GPIO current to ~7mA; drives transistor into saturation at 3.3V
- **10kΩ pull-down** (base to GND): holds base firmly at 0V when GPIO is inactive. Without it, a floating base picks up noise and partially enables the transistor, causing erratic switching and destructive inductive spikes — this destroyed the first ZTX650 during initial power-up with the microcontroller disconnected
- 1N4007 flyback diode clamps the inductive spike when the valve closes (cathode to 12V, anode to GND side of coil)
- nRF51822 GPIO must be set to H0H1 high-drive mode — standard drive (~0.5mA) is insufficient to saturate the transistor at 500mA collector current; firmware handles this

---

### Power supply

**Selected:** Meanwell GST60A12, 12V/5A desktop brick.

- 12V feeds solenoid coil (~500mA peak) and LM2596 buck converter input
- LM2596 steps down to 5V for Tiny BLE via USB injection; module uses screw terminals (no barrel jack socket)
- Meanwell barrel jack (5.5×2.5mm) connects via pigtail wire to perfboard screw terminal — no adapter needed
- UL/CE certified, designed for continuous unattended operation

---

### Filtration — copper mesh basket

Copper mesh basket (100–200 mesh) inside the bucket over the bulkhead inlet. Filters sediment and provides slow Cu²⁺ antimicrobial release. No separate inline strainer.

---

### Reservoir

**Selected:** Black food-grade HDPE 2.5-gal bucket + Gamma-seal lid + 1/2" NPT HDPE bulkhead fitting with EPDM gasket.

- Black HDPE blocks light → prevents algae
- Gamma-seal: dust-tight, insect-proof, easy to open for refilling
- EPDM gasket: compatible with hydrogen peroxide treatments

---

### Enclosure

3D-printed, two-part: body + lid. Lid secured with screws. Single parting line shared between the lid joint and both cable glands.

**External interfaces:**
- Power entry: IEC C8 panel-mount socket; 12V DC pigtail from Meanwell barrel jack inside
- Solenoid cable entry: 2-wire, 12V switched; detachable connector inside (DIN 43650A preferred — same family as connector on valve coil)
- USB: no panel port; remove lid to access Tiny BLE USB

**Cable glands:** Fully integrated into the 3D print — no purchased glands. Lower gland half is part of the body; upper half is part of the lid. Screwing the lid down clamps the cable. Bore ~0.3–0.5mm under cable jacket OD. Interior shoulder prevents inward pull; exterior chamfer sheds water. Optional silicone bead at assembly.

- Power cable OD: 6.35mm (1/4") → bore 5.9mm
- Solenoid cable OD: 3.175mm (1/8") → bore 2.8mm

**Internal components and mounting:**

| Component | Orientation | Mounting |
|---|---|---|
| Seeed Tiny BLE (38.5×42mm) | Sideways | 4 corner extrusions from wall; lid presses top edge down |
| LM2596 buck converter (TBD) | Sideways | 4 corner extrusions from wall; lid presses top edge down |
| Perfboard (40×60mm) | Flat | Standoffs from floor + M3 screws |

Corner extrusion detail: bottom 2 corners are full L-shape (side + bottom ledge, supports weight); top 2 corners are side tab only (no top lip — allows straight insertion from above, avoids overhangs in print). Boards sit slightly lower than their natural wire resting point so wire tension presses them into the supports — no rattling without extra fasteners.

**Dimensions:**
- Interior floor: 65mm × 85mm
- Interior height (floor to parting line): 45mm

---

## Bill of materials (current)

### Electronics

| Item | Notes | Approx. cost |
|---|---|---|
| Seeed Tiny BLE (nRF51822) | Already owned; active microcontroller | — |
| 12V DC solenoid valve, NC, 1/2" NPT, VITON | U.S. Solid brass, direct-acting | ~$15–25 |
| ZTX650 or ZTX651 NPN transistor | Valve driver | ~$0.50–1 |
| 470Ω resistor (1/4W) | Base resistor | <$0.10 |
| 10kΩ resistor (1/4W) | Base pull-down to GND | <$0.10 |
| 1N4007 diode | Flyback protection | <$0.10 |
| DIN 43650A connector with lead wire | Mates with solenoid coil (TBD — preferred option) | ~$3–5 |
| Meanwell GST60A12 12V/5A PSU | Continuous duty, UL/CE | ~$25–30 |
| LM2596 buck converter module | 12V→5V for Tiny BLE; screw terminal inputs | ~$2–4 |
| IEC C8 panel-mount socket | Power entry on enclosure | ~$2–4 |
| Perfboard, jumper wire, screw terminals | Driver circuit board | ~$3–5 |

### Plumbing

| Item | Notes | Approx. cost |
|---|---|---|
| HDPE 2.5-gal black bucket | Food-grade, opaque | ~$8–12 |
| Gamma-seal lid | Fits 2.5-gal bucket | ~$8–10 |
| 1/2" NPT HDPE bulkhead fitting + EPDM gasket | Reservoir outlet | ~$5–8 |
| Copper mesh, 100–200 mesh, food-safe | Basket inside bucket over bulkhead inlet | ~$5–10 |
| 1/2" NPT-F × 1/2" barb adapter, brass (×1) | Bulkhead → supply tube to valve | ~$1–2 |
| 1/2" NPT-M × 1/2" barb adapter, brass (×3) | Valve inlet, valve outlet, manifold inlet | ~$3–6 |
| Orbit 67000 8-port drip manifold | 1/2" FPT inlet, 1/4" barb outlets | ~$8 |
| Silicone tubing 1/2" ID × 3/4" OD, 10 ft | Bulkhead → valve, valve → manifold | ~$8–12 |
| Silicone tubing 1/4" ID × 1/2" OD, 25 ft | Fill lines: manifold → reservoirs | ~$10–15 |
| Silicone tubing 1/8" ID × 1/4" OD, 50 ft | Drip lines: reservoirs → plants | ~$12–18 |
| PTFE tape, 1/2" roll | Seal NPT threads | ~$1–2 |

### Intermediate reservoirs

| Item | Notes | Approx. cost |
|---|---|---|
| Oral Luer-slip syringes, 10–60ml (qty TBD) | Small plants; plunger-as-volume-limiter | ~$0.20–1.50 ea |
| 150ml Luer-slip syringes (qty 6, ordered) | Large plants; same plunger approach | ~$1–3 ea |
| Cable zip ties | Lock syringe plungers | negligible |
| Aquarium-grade silicone sealant | Seal fill tube through plunger hole | ~$5 (tube) |
| Syringe holder rack | Mount syringes vertically above plants | ~$5–15 |

---

## Open items

- [ ] Determine per-plant water volumes → select small syringe sizes and quantities
- [ ] Measure LM2596 buck converter module dimensions → model corner extrusions in CAD
- [ ] Finalize solenoid-side detachable connector (DIN 43650A is preferred)
- [ ] Determine lid screw size and count; decide on heat-set brass inserts
- [ ] Complete enclosure CAD and print
- [ ] Calibrate fill time (valve open duration for all reservoirs to fill)
- [ ] Full system test

---

## What's been resolved

- Architecture: single valve + intermediate reservoirs (Option A)
- Microcontroller: Seeed Tiny BLE (nRF51822), not ESP32; already owned
- Firmware: Arduino C++ with 24h millis() interval; no WiFi, no NTP
- Valve: 1/2" NPT, VITON, direct-acting (U.S. Solid brass)
- Manifold: Orbit 67000; 1/4" barb outlets confirmed; 1/2" FPT inlet
- Filtration: copper mesh basket inside bucket; no inline strainer
- Tubing: silicone, three sizes (1/2" ID main, 1/4" ID fill, 1/8" ID drip)
- Intermediate reservoirs: Luer-slip syringes for all plants (150ml for large; small sizes TBD)
- Power: Meanwell 12V/5A; LM2596 screw terminals, no barrel adapter needed
- Transistor driver: ZTX650/651 + 470Ω base resistor + 10kΩ pull-down + 1N4007 flyback
- Pull-down resistor: added after first ZTX650 destroyed by floating base on initial power-up
- Firmware upload: `cp` command to MBED drive (not Finder drag-and-drop); nRF51822 H0H1 high-drive mode required for valve pin
- Enclosure: 3D-printed body + lid; integrated split glands at body/lid parting line; cable ODs measured; internal dimensions set (65×85mm floor, 45mm height to parting line)
- Power entry: IEC C8 panel-mount socket; USB access via lid removal only
