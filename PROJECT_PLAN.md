# Project Plan: Automatic Plant Watering System

---

## Architecture Decision: Single Valve vs. Per-Plant Valves

Two viable approaches. Choose one before proceeding with hardware procurement.

### Option A — Single Valve + Intermediate Reservoirs
- One solenoid valve on the main line; valve opens to fill all intermediate reservoirs simultaneously
- Per-plant volume controlled **physically** by reservoir size
- Simpler electronics (1 transistor circuit)
- Adjusting water amounts requires physical modification (swap/resize a reservoir)
- No firmware changes needed to change per-plant amounts

### Option B — Per-Plant Valves
- One solenoid valve per plant (5–8 valves), all fed from a common manifold
- Per-plant volume controlled **by software** (each valve open for a different duration)
- More complex electronics (1 transistor circuit per valve, or a relay/MOSFET board)
- Adjusting water amounts is a config/firmware change only — no plumbing changes
- Enables watering plants at different times or skipping individual plants
- Higher component cost; Arduino Uno has enough digital GPIO pins for 5–8 outputs

**Tradeoffs summary:**

| | Option A | Option B |
|---|---|---|
| Valves | 1 | 5–8 |
| Electronics complexity | Low | Medium |
| Adjusting amounts | Physical (swap reservoir) | Software (change duration) |
| Plumbing complexity | Medium (intermediate reservoirs) | Low (direct drip lines) |
| Firmware complexity | Low | Low–Medium |

---

## Bill of Materials

> **Architecture selected: Option A — single valve + intermediate reservoirs, 8 plants, Seeed Tiny BLE (nRF51822).**
> Intermediate reservoirs: Luer-slip syringes for all 8 plants — smaller syringes (≤60ml) for lower-volume plants, 150ml syringes for higher-volume plants. Final sizes TBD per plant.

### Electronics

| Qty | Item | Notes |
|---|---|---|
| 1 | 12V DC Solenoid Valve, NC, 1/2" NPT, VITON seal | U.S. Solid brass — direct-acting, gravity-compatible. Option A uses 1 valve only. |
| 1 | Seeed Tiny BLE (nRF51822) | ARM Cortex-M0, 3.3V logic. Interval-based 24h timing via millis(). No WiFi, no NTP. |
| 1 | ZTX650 or ZTX651 NPN transistor | Single valve driver |
| 1 | 470Ω resistor (1/4W) | Base resistor for transistor |
| 1 | 10kΩ resistor (1/4W) | Pull-down from transistor base to GND — prevents floating base from partially enabling transistor when GPIO is inactive |
| 1 | 1N4007 diode (flyback) | Across solenoid coil — cathode to 12V, anode to GND |
| 1 | Meanwell GST60A12 12V/5A desktop brick power supply | UL/CE certified, continuous duty. Output: 5.5mm × 2.5mm barrel jack (center positive). 12V rail feeds solenoid coil and LM2596 module input. |
| 5 | **LM2596 DC to DC Buck Converter 3.0-40V to 1.5-35V Power Supply Step Down Module** (5-pack) | Input and output via **screw terminals** — no barrel jack. 12V IN+/IN− screws to Meanwell rail; 5V OUT+/OUT− screws to Tiny BLE USB injection. 1 used; 4 spares. |
| 1 | IEC C8 panel-mount socket | Power entry on enclosure; mates with C8 plug on Meanwell barrel jack pigtail |
| — | Perfboard, jumper wires, screw terminals | For transistor driver circuit board |

**Timekeeping:** millis()-based 24h interval. No RTC, no NTP, no WiFi required. Drift ~2–4s/day — acceptable for daily plant watering.

### Electrical Interfaces

All connections between major components. No barrel jack adapter required — the Meanwell output barrel jack is not plugged into any module socket; 12V is distributed from the barrel jack via a pigtail cable to screw terminals on the perfboard.

| Interface | From | To | Connection type | Signal / voltage |
|---|---|---|---|---|
| AC mains → PSU | Wall outlet | Meanwell GST60A12 | IEC C14 / country plug | 120V AC |
| PSU → 12V rail | Meanwell barrel jack (5.5×2.5mm, center+) | Perfboard power bus (pigtail wire to screw terminal) | Barrel jack → pigtail → screw terminal | 12V DC, up to 5A |
| 12V rail → solenoid | Perfboard 12V bus | Solenoid coil (+), via DIN 43650A connector | Wire | 12V DC, ~500mA when on |
| 12V rail → LM2596 IN | Perfboard 12V bus | LM2596 module IN+ screw terminal | Wire to screw terminal | 12V DC |
| LM2596 GND | Perfboard GND bus | LM2596 module IN− screw terminal | Wire to screw terminal | GND (common) |
| LM2596 OUT → ESP32 | LM2596 module OUT+ screw terminal | ESP32 5V pin (or VIN) | Wire | 5V DC, ≤1A |
| LM2596 OUT GND → ESP32 | LM2596 module OUT− screw terminal | ESP32 GND pin | Wire | GND (common) |
| LM2596 OUT → Tiny BLE | LM2596 module OUT+ screw terminal | Tiny BLE USB 5V injection | Wire | 5V DC, ≤1A |
| LM2596 OUT GND → Tiny BLE | LM2596 module OUT− screw terminal | Tiny BLE GND | Wire | GND (common) |
| Tiny BLE GPIO → transistor | Tiny BLE P4 (3.3V logic, H0H1 high-drive mode) | ZTX650/651 base via 470Ω resistor | Wire on perfboard | 3.3V logic, ~7mA base current |
| Base pull-down | ZTX650/651 base | GND | 10kΩ resistor | Holds base at 0V when GPIO inactive |
| Transistor → solenoid (−) | ZTX650/651 collector | Solenoid coil (−), via DIN 43650A connector | Wire | Switched 12V return path, ~500mA |
| Flyback diode | Solenoid coil (−) / collector node | Solenoid coil (+) / 12V rail | 1N4007 across coil | Clamps inductive spike to ~12.7V |
| Common GND | Meanwell GND (barrel jack sleeve) | Perfboard GND bus → LM2596 IN− → Tiny BLE GND → transistor emitter | Wire | All grounds tied together |

### Plumbing

#### Main Reservoir & Valve

| Qty | Item | Notes |
|---|---|---|
| 1 | Black food-grade HDPE bucket, 2.5-gallon (9.5L) | Opaque; blocks light to prevent algae |
| 1 | Gamma-seal lid (fits 2.5-gal bucket) | Dust-tight, insect-proof, easy refilling |
| 1 | 1/2" NPT bulkhead fitting, HDPE or nylon, EPDM gasket | Drilled into side wall near bottom. Avoid natural rubber (H₂O₂ incompatible) |
| 1 | **3-Meter Copper Mesh Woven Filter Column Packing, 4-wire, sanitary/food-grade** | Cut and form into a cup/basket inside bucket over the bulkhead inlet opening. Filters sediment and provides slow Cu²⁺ antimicrobial release — replaces both inline strainer and separate copper insert. |
| 1 | 1/2" NPT-F × 1/2" barb adapter, brass | Bulkhead external thread (1/2" NPT-M) → 1/2" ID supply tube to valve. Female NPT onto bulkhead, barb accepts hose. |
| 3 | 1/2" NPT-M × 1/2" barb adapter, brass | One at valve inlet (1/2" NPT-F) ← supply tube from bulkhead; one at valve outlet → main feed line; one at manifold inlet ← main feed line. Same part all three. |
| 1 | PTFE tape, 1/2" roll | Seal all NPT male threads; 2–3 wraps |

#### Distribution Manifold

| Qty | Item | Notes |
|---|---|---|
| 1 | **Orbit 67000 8-Port Adj Flow Drip Manifold** | 1/2" FPT inlet, eight 1/4" barb outlets — direct fit for 1/4" ID silicone fill lines. Individual rotary flow controls (open fully for gravity feed). Built-in filter screen. Non-pressure-compensating — no minimum pressure required. |

#### Intermediate Reservoirs (per-plant volume control)

One reservoir per plant, sized to hold that plant's exact target water dose. Valve opens → all reservoirs fill simultaneously → valve closes → water drips slowly from each reservoir to its plant.

**All plants — Luer-slip syringes with plunger volume limiter, sized per plant:**

| Qty | Item | Notes |
|---|---|---|
| 4+ | Oral syringe, Luer-slip, with plunger, 10–60ml | For lower-volume plants. e.g. Norm-Ject all-PP syringes. ~$0.20–1.50 each. Source: pharmacy or Amazon ("oral syringe no needle Luer slip") |
| 6 | **150ml Large Syringes for Liquid, Oral, Scientific Labs, Measurement, Dispensing, with Cap** | For higher-volume plants. Plunger-as-volume-limiter approach (same as small syringes). Luer nozzle is the drip outlet — no drilling required. |
| 1 pack | **Cable Zip Ties, 600pc Self-Locking Nylon, Assorted 4/6/8/10/12-inch** | One per syringe. Thread through barrel ears (thumb ring) to lock plunger at desired volume mark. Slide plunger to adjust, re-tie to lock. |
| 1 | Aquarium-grade silicone sealant (e.g. GE Silicone I, 100% silicone, no mildewcide) | Seal around 1/4" ID fill tube where it passes through the drilled plunger hole. Drill hole slightly undersized (~7/16") for interference fit — tube friction is the primary seal; silicone fills the gap. 24h before water contact, 7-day full cure. |
| 1 | Syringe holder rack or clip | Mount syringes vertically above plants; lab-style 6–8 position racks ~$5–15 on Amazon |

**Syringe construction:** Drill ~7/16" hole through center of plunger disc; force 1/4" ID × 1/2" OD fill tube through (interference fit seals by friction); apply silicone around tube at plunger face; cure before use. Luer nozzle at tip accepts 1/8" ID drip line by push-fit — no barb or sealant needed.

**Volume adjustment procedure:** Loosen cable tie → slide plunger to desired volume mark on barrel → re-tighten cable tie. Volume = space between plunger face and Luer nozzle tip. Readable directly from barrel graduation markings.

#### Tubing

Two distinct sizes — fill lines are wider (fast fill from manifold), drip lines are narrower (controlled 1–3 minute delivery to plant).

| Qty | Item | Notes |
|---|---|---|
| 1 spool (10 ft) | Silicone tubing, food-grade, **1/2" ID × 3/4" OD** | Two runs: bulkhead → valve inlet, and valve outlet → manifold inlet. 10 ft covers both segments with margin regardless of layout. |
| 1 spool (25 ft) | Silicone tubing, food-grade, **1/4" ID × 1/2" OD** | Fill lines: manifold 1/4" barb outlets → intermediate reservoir tops. Direct fit on barbs — no reducers. Fills even a 500ml reservoir in <10 seconds at 1m head. |
| 1 spool (50 ft) | Silicone tubing, food-grade, **1/8" ID × 1/4" OD** | Drip lines: intermediate reservoir outlet → plant. At ~0.2m head from a syringe/bottle, 200ml drains in ~1 minute. 8 runs × ≤3 ft = 24 ft max; 50 ft spool gives margin. |

**Drip line timing:** At 0.2m head (typical syringe or bottle height), 1/8" ID drains ~200ml in ~40–60 seconds. Smaller reservoirs (50ml) deliver in ~10–15 seconds. Adjust reservoir size to control volume; tube length can fine-tune rate within ~2× range if needed.

#### Fitting Chain

```
Gravity reservoir (HDPE bucket)
  → bulkhead fitting (1/2" NPT)  [copper mesh basket inside bucket over inlet]
  → [1/2" NPT-F × 1/2" barb adapter] → 1/2" ID supply tube → [1/2" NPT-M × 1/2" barb adapter]
  → solenoid valve inlet (1/2" NPT-F)
  → valve outlet (1/2" NPT-F) → [1/2" NPT-M × 1/2" barb adapter]
  → 1/2" ID main feed line
  → [1/2" NPT-M × 1/2" barb adapter] → 8-port distribution manifold (1/2" FPT inlet, Orbit 67000)
  → [1/4" barb outlets] × 8 → 1/4" ID silicone fill lines (direct fit, no reducers)
  → drop into tops of intermediate reservoirs
  → [syringe Luer nozzle] × 8
  → 8× 1/8" ID silicone drip lines
  → plants
```

**Sealing notes:** PTFE tape on all NPT threads (2–3 wraps). No hose clamps on silicone at gravity pressure. GHT connections seal with EPDM washer only.

### Tubing Research Summary

See PROCESS_LOG.md for full comparison. Four options evaluated: silicone, vinyl/PVC, polyurethane, PTFE.

**Selected: silicone, 1/8" ID × 1/4" OD.**

- Fully plant-safe and inert; no plasticizer concerns
- Stays flexible at room temperature; easy to route around pots
- Fits standard 1/8" barbed fittings without clamps under gravity pressure
- Does not harden or crack over multi-year service life
- Clear grades allow visual flow confirmation
- Moderate kink resistance — use gentle curves, not tight bends
- Cost is moderate–high; small premium justified for permanent indoor installation

**Alternative not selected:** Aquarium-grade vinyl airline tubing (3/16" ID) — proven, cheap, adequate for short-term use; hardens over 2–3 years.

**PTFE note:** Not suitable for flexible runs, but fine-bore PTFE capillary (1/16" ID, 5–15cm segment) can serve as an in-line flow restrictor at the plant outlet if drip rate needs to be reduced.

### Enclosure

| Qty | Item | Notes |
|---|---|---|
| — | 3D-printed enclosure | Custom design; no purchased box |

---

## Phase 1 — Hardware Design & Procurement

### 1.1 Solenoid Valve
- **Selected:** U.S. Solid 1/2" Brass Solenoid Valve, 12V DC, Normally Closed, VITON seal
- VITON seal is chemically resistant — compatible with H₂O₂ water treatment
- 1/2" NPT fittings — matches upstream bulkhead and strainer; no restriction relative to 1/4" ID fill lines
- Normally closed = safe default (valve stays closed if power is lost)
- **Direct-acting** (confirmed) — opens at zero pressure, fully compatible with gravity-fed reservoir at any height
- **Why 1/2" over 1/4":** A 1/4" NPT direct-acting valve has a ~3–4mm internal orifice, which is the system bottleneck and restricts flow below what the 1/4" ID tubing can carry. A 1/2" NPT valve has a ~10–12mm orifice; the tubing becomes the limiting factor, not the valve.
- **Option A:** purchase 1 valve; **Option B:** purchase 1 per plant (5–8)

### 1.2 Per-Plant Water Delivery

**Option A — Intermediate Reservoirs**
- Determine required water volume per plant
- Select or fabricate small containers (e.g. syringes, small bottles, 3D-printed cups)
- Size each reservoir to hold exactly the desired per-plant volume
- Drill/fit outlet for drip tube at the bottom
- Choose drip tube diameters so reservoirs empty slowly but fully before the next cycle

**Option B — Direct Drip Lines**
- Run one tube directly from manifold to each plant
- No intermediate reservoirs needed
- Per-plant volume set by valve open duration in firmware

### 1.3 Main Reservoir — Water Quality
- Use an **opaque container** to block light and prevent algae growth
- **Seal or cover the top** to keep out dust, insects, and airborne spores
- Place reservoir in a **cool, shaded spot** — warm water accelerates bacterial growth
- Place a **copper mesh basket** (100–200 mesh) inside the bucket over the bulkhead inlet opening; serves dual purpose: filters sediment before it enters the line, and provides slow Cu²⁺ release which is naturally antimicrobial and safe for plants at trace levels. No separate inline strainer needed.
- Periodically add a small dose of **food-grade hydrogen peroxide** (~1–3ml of 3% H₂O₂ per liter) to kill bacteria and algae; breaks down into water and oxygen, safe for plants
- **Flush and refill** every 1–2 weeks to prevent stagnation; plan a refill reminder into the schedule

### 1.4 Manifold & Plumbing
- Main gravity reservoir → valve inlet (Option A: single valve; Option B: valve → manifold → per-plant valves)
- Valve outlet → manifold (T-fittings or drilled bar)
- **Option A:** manifold → intermediate reservoirs → thin drip tubes to plants
- **Option B:** manifold → one direct drip line per plant

### 1.5 Electronics BOM

**Current build (Option A, Seeed Tiny BLE):**
- Seeed Tiny BLE (nRF51822) — already owned
- ZTX650 or ZTX651 NPN transistor (single valve driver)
- 470Ω resistor (base resistor)
- 10kΩ resistor (base pull-down to GND)
- 1N4007 flyback diode
- Meanwell GST60A12 12V/5A PSU
- LM2596 buck converter module (12V→5V, screw terminals)
- IEC C8 panel-mount socket
- Perfboard, jumper wires, screw terminals

**If expanding to Option B (future, requires ESP32):**
- 1× ZTX650/651 NPN transistor per plant (8 total), or 2× ULN2803A Darlington ICs
- See Section 1.6 for full 8-channel driver comparison

> **Valve driver selection research:** See Section 1.6 below for a full comparison of all reasonable options for driving 8× 12V/500mA solenoid valves from 3.3V GPIO (relevant if expanding to Option B).

---

### 1.6 Valve Driver Options — ESP32 (3.3V GPIO) to 8× 12V/500mA Solenoid Valves

System parameters: ESP32 dev board (3.3V GPIO, ~12mA source max per pin), 8× 12V DC solenoid valves at 500mA each, all 8 potentially open simultaneously (4A total at 12V), inductive loads requiring flyback protection.

---

#### Option 1 — ULN2803A Darlington Array IC

**What it is:** 18-pin DIP IC containing 8 NPN Darlington transistor pairs, one per channel. Texas Instruments, STMicroelectronics, ON Semi all make compatible parts; ~$0.50–$1.00 each.

| Parameter | Value |
|---|---|
| Channels | 8 |
| Output voltage max | 50V |
| Output current max | 500mA continuous per channel (600mA peak) |
| Input series resistor (internal) | 2.7kΩ on each input |
| Input threshold to turn on | V_I(on) = 2.4V max (for 200mA output) |
| Logic family (designed for) | TTL and 5V CMOS |
| Built-in flyback diodes | Yes — one per channel; COM pin must be tied to the 12V load supply rail |
| Package | 18-DIP (through-hole); also SOIC |

**3.3V logic compatibility:** The ULN2803A will turn on with a 3.3V input. The V_I(on) threshold is 2.4V maximum, so a 3.3V ESP32 GPIO (which drives ~2.8V–3.3V high) is above threshold. Confirmed in practice by TI forum engineers and community use. Performance is somewhat reduced compared to 5V drive — expect slightly higher V_CE(sat), but at 500mA loads this is acceptable.

**Critical derating — all 8 channels simultaneous at 500mA:**
The absolute maximum substrate/GND pin current is 2.5A. With 8 channels each at 500mA = 4A, this **exceeds the GND pin limit by 60%** and would destroy the IC. The total package thermal dissipation is also ~2.25W, which at 8 × 500mA is far exceeded. This is the fundamental limitation for this application.

**Workaround:** Use **two ULN2803A ICs** (4 channels each), keeping each IC's total current at 2A. With all 8 valves open: IC1 handles valves 1–4, IC2 handles valves 5–8. Each IC carries 2A peak, within its 2.5A GND pin limit with margin. Total cost: ~$1–2 for both ICs.

**Flyback diodes:** Built-in, no external diodes needed. Connect the COM pin of each IC to the 12V solenoid supply. This is the most convenient aspect of this IC.

**Wiring:**
```
ESP32 GPIO → ULN2803A input pin (IN1–IN8)
ULN2803A output pin (OUT1–OUT8) → solenoid coil (–)
Solenoid coil (+) → 12V supply
ULN2803A COM pin → 12V supply  (activates built-in flyback diodes)
ESP32 GND and 12V supply GND → common GND
```

**Breakout boards:** The ULN2803A is available as a bare DIP IC only — no common breakout boards exist. It's designed to be wired directly on a PCB or perfboard.

**Pros:**
- Extremely simple — 8 channels in one IC (or split across two)
- Built-in flyback diodes, no external parts needed per channel
- Very low cost (~$0.50–$1.00 per IC)
- Well-documented, decades of use
- Works with 3.3V ESP32 GPIO

**Cons:**
- Cannot run all 8 channels at full 500mA from a single IC — requires 2 ICs for this application
- Darlington topology adds ~1–1.4V V_CE(sat) drop at the output (minor, dissipates ~0.5–0.7W per channel; acceptable)
- No optoisolation — ESP32 GND shares with load circuit
- Bare IC only; requires perfboard or PCB, no plug-in module

**Verdict:** Best choice for a custom PCB or neat perfboard build. Two ICs covering 4 channels each is the correct configuration for this application. Lowest cost and parts count of all DIY options.

---

#### Option 2 — ULN2003A Darlington Array IC

**What it is:** Functionally nearly identical to the ULN2803A but with 7 channels instead of 8 and a different internal input resistor suited to higher-voltage logic families.

| Parameter | ULN2003A | ULN2803A |
|---|---|---|
| Channels | 7 | 8 |
| Output voltage max | 50V | 50V |
| Output current max | 500mA per channel | 500mA per channel |
| Input series resistor | 2.7kΩ | 2.7kΩ |
| Logic family targeted | TTL / 5V CMOS | TTL / 5V CMOS |
| Built-in flyback diodes | Yes | Yes |
| Package | 16-DIP | 18-DIP |

**3.3V logic compatibility:** Essentially the same as ULN2803A — V_I(on) is 2.4V max, so 3.3V GPIO will drive it. No practical difference from ULN2803A on this point.

**Thermal/current derating:** Identical to ULN2803A — same GND pin 2.5A max, same package dissipation constraints apply.

**Key difference:** Only 7 channels. For 8 valves, you need two ULN2003A ICs regardless (one handling 4, one handling 4 — wasting 3 channels on the second IC). This makes the ULN2803A strictly better for this application: 8 channels per IC means two ICs cover exactly 8 channels with zero waste.

**Note on input resistor:** Some sources cite the ULN2003A as being optimized for TTL (5V) logic while ULN2803A is rated for "all logic families including CMOS." In practice both ICs behave the same with 3.3V inputs. The real-world difference is negligible here.

**Verdict:** Do not choose ULN2003A over ULN2803A for this application. The channel count mismatch makes it slightly more wasteful with no compensating benefit.

---

#### Option 3 — Pre-made 8-Channel Relay Modules (Optocoupler-Isolated)

**What it is:** A PCB containing 8 electromechanical relays with optocoupler input isolation, IN1–IN8 signal pins, Vcc/GND power, and screw terminal or header outputs (COM/NO/NC per relay). Widely available on Amazon and AliExpress from multiple brands (SainSmart, FlyNoval, generic Chinese OEM).

**3.3V trigger compatibility — the critical split:**

Most inexpensive 8-channel relay modules sold as "Arduino relay modules" are designed for 5V trigger logic. The optocoupler LED on the input side requires ~5–10mA to activate, and the input circuit resistor is sized for 5V. At 3.3V, the LED current may fall below the turn-on threshold and the relay will not reliably activate. This is the most common ESP32 gotcha with relay modules.

Two variants exist:

| Variant | Trigger voltage | Works with ESP32 3.3V? |
|---|---|---|
| Standard "Arduino" 5V relay module | 5V | No — unreliable or fails entirely |
| Modules explicitly marked "3.3V compatible" or "3V optocoupler" | 3.3V | Yes — designed for this |

**Identifying 3.3V-compatible modules:** Look for product listings that state "3.3V relay," "3V optocoupler," or "ESP8266/ESP32 compatible." Example: FlyNoval 8 Channel 3.3V Relay Module (Amazon, ~$10–12) uses 3V optocouplers and is explicitly rated for 3.3V trigger input. Avoid any module that only lists "5V trigger" or says "for Arduino Uno."

**Active-high vs active-low:** Many relay modules are active-low — the relay turns ON when GPIO is pulled LOW (0V), and turns OFF when GPIO is HIGH. This is counter-intuitive and must be accounted for in firmware. Some modules offer a jumper to select polarity. Verify your specific module before writing code.

**Current and voltage ratings:** Relay contacts on common modules are rated 10A at 250V AC / 10A at 30V DC. Well above the 12V/500mA needed for solenoid valves. The relay coil itself draws ~70–90mA at the coil supply voltage — this is drawn from Vcc (not from the ESP32 GPIO).

**Power supply note:** These modules require a separate coil supply (typically 5V from the module's Vcc pin). The signal input goes through the optocoupler. On dual-supply modules, the optocoupler input side is powered by the ESP32's 3.3V, and the coil side by a 5V source. This eliminates ground loop issues. Confirm the module you select has this dual-supply architecture if you want full isolation.

**Flyback diodes:** Relay coil flyback is typically handled internally on relay modules. However, the relay contact output side provides no flyback protection for the solenoid — the solenoid's own flyback spike is suppressed by the relay contact arc quenching and any external diode you add. Best practice: add a 1N4007 across each solenoid coil (at the valve terminals), independent of the relay module.

**Wiring:**
```
ESP32 GPIO (3.3V) → optocoupler input (IN1–IN8)
Module Vcc → 5V supply (for relay coils)
Module GND → common GND
Relay COM → 12V supply
Relay NO → solenoid coil (+)
Solenoid coil (–) → GND
1N4007 diode across each solenoid coil (external, at valve)
```

**Cost:** ~$8–12 for an 8-channel module. Plus 8× 1N4007 diodes (~$1).

**Pros:**
- Full galvanic isolation between ESP32 and high-side circuit (via optocoupler)
- Very high contact current rating (10A) — no derating concerns
- Works with 12V, 24V, or any voltage the relay contacts can handle
- Plug-in module, minimal soldering
- Widely available, many tutorials

**Cons:**
- Most common modules require 5V logic — must specifically source 3.3V-compatible variant
- Active-low logic is confusing and must be handled in firmware
- Electromechanical relays produce audible clicks during valve switching (minor for plant watering)
- Relay mechanical lifetime: typically 100,000 operations; fine for daily watering but worth noting
- External flyback diodes needed on solenoid side (relay contacts do not suppress load-side inductive spikes; the arc quenches but does not clamp)
- Module typically needs 5V coil supply — requires a 5V rail (from USB adapter or buck converter)

**Verdict:** Good choice if isolation between ESP32 and valve circuit is important, or if you prefer plug-and-play assembly. Must buy a module explicitly rated for 3.3V trigger. Add external flyback diodes at the solenoid terminals.

---

#### Option 4 — Pre-made 8-Channel MOSFET Driver Boards

**What it is:** A PCB containing multiple N-channel MOSFETs wired as low-side switches, one per channel, with (ideally) gate drive resistors and flyback diodes. Far fewer standardized pre-built 8-channel MOSFET modules exist compared to relay modules.

**The 3.3V gate drive problem:**

This is the most critical issue for ESP32 use. Standard power MOSFETs (IRF520, IRF540, IRFZ44N) have gate threshold voltages (V_GS(th)) of 2–4V to just begin conducting, and require 8–10V on the gate to reach full R_DS(on) and maximum current capacity. At 3.3V gate drive, these devices are partially on — they conduct but with much higher on-resistance, generating significant heat. They should not be used with 3.3V logic at 500mA loads.

Logic-level MOSFETs required for 3.3V drive (V_GS(th) 1–2.5V, fully on at 3.3V):

| Part | V_DS max | I_D max | V_GS(th) | R_DS(on) at 3.3V | Notes |
|---|---|---|---|---|---|
| IRLB8721PbF | 30V | 62A | 1.35–2.35V | ~8.7mΩ | TO-220; excellent at 3.3V |
| IRLZ44N | 55V | 47A | 1–2V | ~17.5mΩ at 3.3V | TO-220; well proven |
| AOD4184 / D4184 | 40V | 50A | ~2–4V (uncertain) | moderate | Common on pre-made modules; may not fully switch at 3.3V — some sources report marginal behavior |
| IRLB3034PbF | 40V | 195A | 1–2V | ~1.4mΩ | Overkill but works perfectly |
| STN4NF03L | 30V | 4A | ~1V | very low | SOT-223; good for ≤4A |

**IRF520 warning:** The IRF520 is widely used on cheap "MOSFET driver modules" sold for Arduino. Its V_GS(th) is 2–4V and it is rated for 10V gate drive. At 3.3V it is unreliable. Do not use IRF520-based modules with ESP32 directly.

**Available pre-made modules:**

Single-channel MOSFET modules (IRF520, AOD4184, or generic D4184 boards) are widely sold, ~$1–2 each. Buying 8 gives you 8 independent channels but requires 8 separate modules — messy wiring.

4-channel modules: The YYNMOS-4 (4-channel MOSFET module, uses 60N03 MOSFET, 30V/45A, 5A per channel, includes 1N4148 flyback diodes) is available for ~$5–8. You need two for 8 channels. Rated for 3–20V input; however, at least one source reports that the YYNMOS-4 may not trigger reliably at 3.3V due to optocoupler input resistor values sized for higher voltages — verify with your specific module version before relying on it.

True purpose-built 8-channel logic-level MOSFET boards for 3.3V are not a widely standardized product category. What you will find on AliExpress/Amazon tends to be:
- 8× individual IRF520 modules sold as a set (IRF520 is the wrong MOSFET, as noted)
- Generic D4184/AOD4184 single-channel modules (buy 8, ~$8–12 total)
- 4-channel YYNMOS-4 boards × 2

**Flyback diodes:** This varies heavily by module. Some modules include a small diode (often 1N4148 — fast but low current). The D4184 module documentation explicitly warns that it has **no flyback diode**. Always verify and add 1N4007 diodes externally at the solenoid terminals for certainty.

**Wiring (low-side switch, per channel):**
```
ESP32 GPIO (3.3V) → gate (via 100–470Ω resistor)
MOSFET drain → solenoid coil (–)
Solenoid coil (+) → 12V supply
MOSFET source → GND
1N4007 flyback diode across solenoid coil (cathode to 12V, anode to drain)
```

**Cost:** 8× individual logic-level MOSFET modules: ~$8–16 depending on source and MOSFET used. Or 2× YYNMOS-4: ~$10–16.

**Pros:**
- MOSFETs are solid-state — silent operation, no mechanical wear
- Very low on-resistance (logic-level parts) — minimal heat at 500mA
- Fast switching — not relevant for solenoid valve timing but a nice property
- Lower voltage drop than Darlington arrays
- Can support PWM for variable control if needed in future

**Cons:**
- No standardized high-quality 8-channel 3.3V MOSFET board exists — you assemble from single or 4-channel modules
- Must select logic-level MOSFET parts specifically; easy to accidentally buy the wrong module (IRF520-based)
- Flyback diode inclusion is inconsistent across modules — must verify each product
- More wiring than a single-IC solution
- Pre-made modules often lack optoisolation (ESP32 shares GND with load circuit)

**Verdict:** Good performance, but the pre-made landscape is fragmented. Best option is either (a) build a simple 8-channel board with IRLB8721 or IRLZ44N MOSFETs and 1N4007 diodes directly, or (b) buy 2× YYNMOS-4 boards and verify 3.3V triggering before committing.

---

#### Option 5 — Ready-Made ESP32 + 8-Channel Relay/MOSFET Controller Boards

**What it is:** Integrated boards combining ESP32 and 8-channel relay output on one PCB, ready to program via Arduino IDE or ESPHome.

Two products are relevant:

---

**KinCony KC868-A8 / KC868-A8v3**

- ESP32-WROOM-32 (original) or ESP32-S3 (v3 variant)
- 8× SPDT relays; contact rating 10A at 250V AC (documentation says 10A/220V)
- Relays driven via ULN2003A Darlington array controlled by PCF8574P I2C GPIO expander (not direct ESP32 GPIO)
- 8× optoisolated digital inputs (EL357 optocouplers)
- Also includes RJ45 Ethernet, RS485, I2C headers, analog inputs
- Relay coils driven from internal 12V-to-5V regulation (board requires 12V DC input)
- Logic levels internally: 3.3V ESP32 → I2C expander → ULN2003A → relay coil; no need to worry about 3.3V GPIO limitation
- Price: ~$55–67 depending on configuration (PCB only to PCB + enclosure + accessories)
- ESPHome support confirmed; Tasmota community use documented

Suitability for this application:
- 12V DC contact load well within relay rating
- 500mA per valve well within 10A contact rating
- Board requires 12V DC input supply — already in BOM (Meanwell GST60A12)
- Relay contacts: add external 1N4007 diodes at solenoid terminals for best practice
- Gotcha: relay actuation goes through I2C GPIO expander, which adds firmware complexity vs. direct GPIO; also Ethernet chip occupies 9 GPIOs, limiting direct pin count
- Includes features (Ethernet, RS485, analog inputs) that are unnecessary for this application — you are paying for a more complex board

---

**Waveshare ESP32-S3-ETH-8DI-8RO / 8DI-8RO-C**

- ESP32-S3-WROOM-1U-N16R8 (16MB flash, 8MB PSRAM, dual-core 240MHz)
- 8× SPDT relays; contact rating 10A at 250V AC / 10A at 30V DC
- 8× optoisolated digital inputs (5V–36V input range, NPN or PNP)
- Onboard TVS surge protection
- Power input: 7–36V DC (or 5V USB-C)
- Isolated RS485 interface (standard variant) or isolated CAN bus (C variant)
- Full power supply isolation
- Price: $29.99–$32.99 (RS485 variant) / $44.99–$49.99 (CAN variant) — available on Amazon
- ESPHome support confirmed (devices.esphome.io)

Suitability for this application:
- Relay contact rating (10A at 30V DC) comfortably covers 12V/500mA valves
- 7–36V input means the Meanwell 12V supply powers it directly
- Onboard TVS helps protect against solenoid switching transients
- Full isolation is a quality feature
- Again: add external 1N4007 across each solenoid coil for belt-and-suspenders protection
- Gotcha: relay actuation is controlled through internal logic (PCF8574 or similar expander — confirm in wiki before firmware); not direct GPIO per relay
- RS485/CAN interface, Ethernet, and digital inputs are unnecessary for this project

---

**Summary comparison — ready-made ESP32+relay boards:**

| | KC868-A8v3 | Waveshare ESP32-S3-ETH-8DI-8RO |
|---|---|---|
| ESP32 | ESP32-S3 | ESP32-S3-WROOM |
| Relay channels | 8 | 8 |
| DC contact rating | 10A (verify DC spec) | 10A / 30V DC (confirmed) |
| Relay drive | I2C expander → ULN2003A | Internal (verify) |
| Isolation | Optocoupler inputs | Full power isolation + TVS |
| Power input | 12V DC | 7–36V DC or USB-C |
| Price | $55–67 | $30–50 |
| ESPHome | Yes | Yes |
| Extras | Ethernet, RS485, analog in | Ethernet, RS485 or CAN |

---

### 1.6 Summary and Recommendation

| Option | 3.3V Compatible | Flyback Diodes | Wiring Complexity | Cost (8 ch) | Form Factor |
|---|---|---|---|---|---|
| 2× ULN2803A IC | Yes (verified) | Built-in (via COM pin) | Low — perfboard/PCB | ~$1–2 | Bare ICs |
| ULN2003A (×3) | Yes | Built-in | Low | ~$1.50 | Bare ICs |
| 8-ch relay module (3.3V type) | Yes — if explicitly rated | Partial (add external at solenoid) | Very low | ~$10–12 | Plug-in module |
| 8-ch relay module (5V type) | No | Partial | Very low | ~$8–10 | Plug-in module — DO NOT USE |
| MOSFET modules (logic-level) | Yes — if correct MOSFET | Varies — verify/add external | Medium | ~$8–16 | Multiple modules |
| KC868-A8v3 (integrated) | Yes (internal) | Partial — add external at solenoid | None (all-in-one) | ~$55–67 | Dev board w/ ESP32 |
| Waveshare ESP32-S3-ETH-8DI-8RO | Yes (internal) | TVS onboard + add external at solenoid | None (all-in-one) | ~$30–50 | Dev board w/ ESP32 |

**Recommended path:**

- **Lowest cost / most elegant (custom build):** Two ULN2803A ICs, 4 channels each, COM pins to 12V rail. No external flyback diodes needed for the valve drive circuit. ~$1–2 total. Requires a PCB or tidy perfboard layout. Confirmed 3.3V compatible.

- **Easiest assembly / plug-in (separate ESP32):** A 3.3V-compatible 8-channel relay module (e.g. FlyNoval 8-channel 3.3V with optocouplers, ~$10–12). Add 8× 1N4007 diodes at the solenoid terminals. Handle active-low logic in firmware. Requires a 5V coil supply (USB adapter or buck converter).

- **All-in-one integrated board:** Waveshare ESP32-S3-ETH-8DI-8RO (~$30–50) if you want a single board combining ESP32 + 8-relay output with industrial-grade isolation, ready to run ESPHome. Overkill on features for plant watering, but well-supported and robustly designed. Eliminates separate ESP32 purchase.

**What to avoid:**
- Any relay module listed only as "5V trigger" — will not work reliably with ESP32 3.3V GPIO
- IRF520-based MOSFET modules — not a logic-level device; marginal at 3.3V
- A single ULN2803A driving all 8 channels at full 500mA — exceeds the 2.5A GND pin maximum; must split across two ICs

---

## Phase 2 — Electronics Assembly

### 2.1 Transistor Switch Circuit
- Wire Tiny BLE P4 → 470Ω resistor → ZTX650/651 base
- 10kΩ pull-down from base to GND (installed alongside 470Ω; holds base at 0V when GPIO is inactive)
- Collector → solenoid coil (–), solenoid coil (+) → 12V
- Flyback diode across solenoid (cathode to 12V)
- Common GND between Tiny BLE and 12V supply
- Firmware must set H0H1 high-drive mode on P4 after pinMode() — nRF51822 standard drive (~0.5mA) is insufficient to saturate the transistor at 500mA collector current

### 2.2 Bench Test
- Verify transistor saturates and valve clicks open/closed
- Confirm no voltage spikes on Tiny BLE GPIO (check flyback diode is working)

---

## Phase 3 — Firmware

**Language: Arduino C++.** Target: Seeed Tiny BLE (nRF51822) via sandeepmistry/arduino-nRF5 board package. Entry point is `firmware_ble/watering_system/watering_system.ino`. Compile in Arduino IDE; upload by copying the `.bin` to the MBED drive via `cp` (macOS Finder drag-and-drop is unreliable).

A MicroPython ESP32 implementation exists in `firmware/` — deferred until an ESP32 is acquired.

### 3.1 Core Valve Control

- P4 HIGH → transistor on → valve open; P4 LOW → transistor off → valve closed
- Valve open duration set by `FILL_DURATION_S` (default 2s; calibrate upward)
- Hard safety cap: `VALVE_MAX_ON_S` (default 60s) — valve never stays open longer regardless of config
- Waters immediately on first boot, then every 24 hours

### 3.2 Scheduling

- `millis()`-based interval timing — no RTC, no NTP, no WiFi
- `WATER_INTERVAL_MS = 24UL * 60UL * 60UL * 1000UL` — UL suffixes required to prevent 32-bit overflow before widening
- Unsigned subtraction is rollover-safe: correct for up to ~49 days between checks

### 3.3 Safety and Status

- H0H1 high-drive mode set on valve pin after `pinMode()` — required for nRF51822 to source sufficient base current through 470Ω
- Valve pin written LOW before `pinMode(OUTPUT)` — ensures valve stays closed from first clock cycle
- RGB LED status (active LOW): 3 green blinks on boot, solid green while running, blue during watering cycle

### 3.4 Debug Output
- Serial at 115200 baud — log valve open/close events (visible via serial monitor or `mpremote`)

---

## Phase 4 — Calibration & Testing

### 4.1 Option A — Reservoir Fill & Drip Test
- Run valve and measure time to fill all intermediate reservoirs to capacity
- Confirm manifold distributes water to all reservoirs before any overflow
- Measure time for each reservoir to fully empty; adjust tube diameter/length as needed
- Target: fully empty well before the next scheduled watering cycle

### 4.2 Option B — Duration Calibration
- Run each valve individually; measure volume delivered per second of open time
- Set per-plant durations in firmware to deliver target volumes
- Verify simultaneous operation doesn't starve any line (gravity flow may drop under load)

### 4.3 Full Cycle Test (both options)
- Run several complete cycles on schedule
- Check for leaks, overflow, or under-delivery

---

## Phase 5 — Enclosure & Final Setup

### 5.1 Enclosure
- House electronics in a small project box
- Ensure 12V and Arduino power connections are strain-relieved

### 5.2 Mounting
- Mount gravity reservoir above plants (higher = more head pressure = faster fill)
- Route tubing neatly; secure any intermediate reservoirs (Option A) near each plant

### 5.3 Long-term Validation
- Run for 1–2 weeks unattended
- Check plant health and main reservoir water levels

---

## Future Additions (Out of Scope Now)
- Soil moisture sensors per plant to trigger watering conditionally
- Low-water alert for main reservoir
- Web/app interface for schedule changes (ESP32 + WiFi already in build — interface implementation deferred)
