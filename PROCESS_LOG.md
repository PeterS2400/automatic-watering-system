# Process Log: Automatic Plant Watering System

---

## 2026-03-24 — Project initiation & BOM setup

### Decisions recorded

| Decision | Choice | Notes |
|---|---|---|
| Architecture | **Option A — single valve + intermediate reservoirs** | 1 solenoid valve; per-plant volume controlled by intermediate reservoir size |
| Plant count | **8 plants** | Upper end of planned range |
| Controller + driver | **ESP32 dev board + ZTX650 transistor circuit** | Single valve; same circuit as original CLAUDE.md design. 12V→5V buck converter for ESP32 power. |
| Intermediate reservoirs | **Hybrid: Luer-slip syringes (≤60ml) + PP lab bottles (>60ml)** | Syringes: plunger kept as adjustable fill ceiling; drill fill port through plunger disc; lock plunger with cable tie through barrel ears. Bottles: drill near base, rubber grommet + 1/8" barb; volume changed by swapping bottle size. |
| WiFi / remote control | **Yes — build it in now** | Schedule and status accessible remotely |
| Manifold | **Off-shelf 8-port drip manifold** | e.g. Orbit/Rain Bird; 1/4" barb outlets. Reducer couplers eliminated — fill lines are 1/4" ID silicone, direct fit on barb. |
| Enclosure | **3D-printed** | Custom design, no purchased project box |
| Power supply | **Meanwell GST60A12, 12V/5A desktop brick** | UL/CE certified, continuous duty; 5.5×2.5mm barrel jack |
| Fill line tubing | **Silicone, 1/4" ID × 1/2" OD, 25 ft spool** | Manifold → intermediate reservoir tops. Direct fit on 1/4" barb; fills 500ml in <10s at 1m head. |
| Drip line tubing | **Silicone, 1/8" ID × 1/4" OD, 50 ft spool** | Intermediate reservoir outlet → plant. Drains 200ml in ~1 min at 0.2m head. Target: 1–3 min delivery. |

---

### Tubing research — 2026-03-24

Compared four material options for the drip lines (gravity-fed, low-pressure, indoor).

**Materials evaluated:** silicone, vinyl/PVC, polyurethane (PU), PTFE

**Summary:**

| | Silicone | Vinyl (PVC) | PU | PTFE |
|---|---|---|---|---|
| Plant/food safe | Yes | Yes (food-grade only) | Yes (food-grade) | Yes |
| Flexibility | Excellent | Good | Good | Poor |
| Kink resistance | Moderate | Moderate | Good | Poor |
| Transparency | Clear grades available | Clear–hazy | Clear grades available | Milky/translucent |
| Barbed fitting fit | Excellent | Excellent | Good | Poor |
| Longevity | Excellent | Fair (hardens over years) | Good | Excellent |
| Cost | Moderate–high | Low | Moderate | Moderate–high |
| Indoor drip suitability | **Excellent** | Good | Good | Poor |

**Recommendation from research:** Silicone, 1/8" ID × 1/4" OD. Fully inert, stays flexible, fits barbed fittings without clamps, safe for plants long-term.

**Budget alternative (not selected):** Aquarium-grade vinyl airline tubing (3/16" ID) — adequate for indoor use, replace every 2–3 years.

**PTFE note:** Not suitable for flexible drip routing due to stiffness, but fine-bore PTFE capillary (~1/16" ID) could be used as a short in-line flow restrictor at the plant outlet if precise slow drip rates are needed.

**User selection:** Silicone (recommended option).

---

---

### Power supply research — 2026-03-24

Compared generic wall warts, reputable brand wall-mount, generic desktop bricks, and Meanwell branded bricks.

**Key findings:**
- System draws 8 × 500mA = 4A peak; ≥5A supply recommended for headroom
- Generic adapters: certification authenticity questionable; thermal performance at sustained 4A varies; not recommended for unattended 24/7 operation
- Meanwell: verified UL/CE, published MTBF (>100k hours), designed for continuous embedded deployment
- Barrel jack: generic adapters use 5.5×2.1mm; Meanwell typically uses 5.5×2.5mm — a $1 adapter resolves any mismatch
- Solenoids are inductive loads; switching PSUs handle them fine; flyback diodes already in BOM are the correct mitigation

**User selection:** Meanwell GST60A12 (12V/5A desktop brick).

---

### Reservoir research — 2026-03-24

Compared HDPE buckets, collapsible bladders, beverage dispensers, garden reservoirs, and jerrycans.

**Key findings:**
- Collapsible bladders: head pressure drops as they collapse — incompatible with gravity-fed flow
- Most beverage dispensers are clear/translucent — algae risk; opaque options hard to source reliably
- Jerrycans: excellent in all respects except no viable bottom outlet (shape prevents bulkhead fitting)
- HDPE bucket: opaque (black), food-grade, H₂O₂-compatible, easy bulkhead fitting, cheap, standard Gamma-seal lid available
- 2.5-gallon (9.5L) keeps full weight under 10kg on shelf; covers 2–3 watering cycles before refill

**User selection:** Black food-grade HDPE 2.5-gal bucket + Gamma-seal lid + 1/2" NPT bulkhead fitting + EPDM gasket.

---

### Fittings & manifold research — 2026-03-24

**Critical barb sizing clarification:** Barb OD matches tubing ID. 1/8" barb → 1/8" ID tubing. Standard drip manifolds use 1/4" barb outlets (sized for 1/4" ID poly tubing, not 1/8" ID silicone) — requires reducers.

**Manifold options evaluated:**
- Off-shelf (Orbit/Rain Bird 8-port): ~$10–20; outlets are 1/4" barb — needs 8× 1/4"→1/8" reducer couplers (~$4–16). Total ~$15–35.
- 3D-printed PETG with native 1/8" barbs: ~$7–12 in materials, no reducers, but requires design/print time. FDM is not inherently waterproof; PETG + CA glue coat seals adequately at gravity pressure.

**User selection:** Off-shelf manifold + 8× 1/4"→1/8" reducer couplers.

**Complete fitting chain:**
```
Reservoir bulkhead (1/2" NPT) → strainer → manifold (3/4" FHT or 1/2" NPT inlet)
→ manifold 1/4" barb outlets → 1/4"→1/8" reducer couplers × 8
→ 1/4" NPT close nipples × 8 → valve inlets (1/4" NPT-F)
→ valve outlets (1/4" NPT-F) → 1/4" NPT-M × 1/8" barb adapters × 8
→ 1/8" ID silicone drip lines × 8 → plants
```

**Sealing:** PTFE tape on all NPT threads; no hose clamps on silicone at gravity pressure.

---

### Valve driver research — 2026-03-24 (superseded by Option A switch)

Originally researched for Option B (8 valves). Superseded when architecture switched to Option A (1 valve). Full comparison retained in PROJECT_PLAN.md §1.6 for reference.

**Summary of what was found (for record):** ULN2803A (2 ICs, 4 channels each) was the lowest-cost DIY option; Waveshare ESP32-S3-ETH-8DI-8RO was the best all-in-one. Key ESP32 gotcha: 3.3V GPIO — most cheap relay modules require 5V and won't trigger reliably.

**Outcome:** Architecture switched to Option A (1 valve). Driver circuit reduced to single ZTX650 NPN transistor + 470Ω base resistor + 1N4007 flyback diode — the original CLAUDE.md design.

---

### Tubing flow rate research — 2026-03-24

Researched fill line and drip line sizing separately after deciding on two-tube approach.

**Fill lines (manifold → intermediate reservoirs):**
- Turbulent flow regime at relevant head heights — Darcy-Weisbach used
- 1/4" ID at 1m head fills 500ml in ~8s, 200ml in ~3s — essentially instantaneous
- 1/4" ID silicone fits directly on standard 1/4" barb manifold outlets → no reducer couplers needed
- **Selected: 1/4" ID × 1/2" OD silicone, 25 ft spool**

**Drip lines (intermediate reservoir → plant):**
- Laminar flow regime at ~0.2m head from small reservoirs
- 1/8" ID at 0.2m head, 1m run: ~200ml drains in ~40–60s; 50ml in ~10–15s
- PTFE capillary (0.8mm ID) evaluated — provides hours-long drip but rejected by user (target: 1–3 minute delivery)
- Length tuning viable for fine adjustment within ~2× range
- **Selected: 1/8" ID × 1/4" OD silicone, 50 ft spool**

---

### Intermediate reservoir research — 2026-03-24

Researched commercial products (syringes, IV bags, drip stakes) and DIY containers (bottles, Falcon tubes, 3D-printed, glass).

**Key findings:**
- Drip stakes and IV bags: not suitable (pressurized design / sealed fill problem)
- Falcon/centrifuge tubes: elegant conical drain, rack mounting included, but 50ml hard limit
- 3D-printed PETG: cleanest result with integrated barbs; needs design work and CA glue coat for waterproofing
- Oral syringes (Luer-slip, plunger removed): zero modification, 1/8" silicone push-fits directly onto Luer nozzle, 10–60ml range
- PP lab reagent bottles (wide-mouth): drill near base, rubber grommet + 1/8" barb, 125–500ml range. Do NOT use hot glue (fails on PP). Silicone sealant alone won't bond PP without primer — rubber grommet is primary seal.

**User selection:** Hybrid — syringes for small plants (≤60ml), PP lab bottles for large plants (>60ml).

- [x] Architecture switched to **Option A** — 1 valve, intermediate reservoirs. Electronics simplified to ESP32 + single ZTX650 circuit.
- [x] Intermediate reservoir type selected: **syringes (≤60ml) + PP lab bottles (>60ml)**
- [x] Syringe volume control: **plunger-as-limiter** — drill fill port through plunger disc, lock plunger position with cable tie through barrel ears. Continuously adjustable, readable from barrel graduation markings.
- [x] Tubing: two sizes — 1/4" ID fill lines (fast), 1/8" ID drip lines (1–3 min delivery)
- [x] Reducer couplers eliminated — 1/4" ID fill lines fit directly on manifold 1/4" barb outlets
- [ ] Determine per-plant water volumes → confirm syringe/bottle sizes and quantities needed

---

### Open items as of 2026-03-24

- [x] Silicone tubing: **1/4" ID × 1/2" OD, 25 ft** (fill lines) + **1/8" ID × 1/4" OD, 50 ft** (drip lines)
- [x] Reducer couplers eliminated — fill lines are 1/4" ID, direct fit on manifold barbs
- [ ] Determine per-plant water volumes → finalize syringe/bottle sizes and quantities
- [ ] Barrel jack adapter (5.5×2.5mm → 5.5×2.1mm) — confirm during electronics design
- [ ] Manifold specific product — confirm outlet type (1/4" NPT-F vs. 1/4" barb) before ordering fittings

---

## 2026-03-24 — Manifold selection, BOM refinements, electrical interface documentation

### Intermediate reservoir quantities confirmed

Minimum quantities set: **at least 4 syringes** (≤60ml plants) + **at least 4 bottles** (>60ml plants). Final sizes still TBD per plant. BOM updated from TBD to 4+.

---

### Manifold research — 2026-03-24

Researched 6 manifold types: off-shelf drip irrigation, aquarium/hydroponics, DIY barbed tee chain, DIY 1/2" header with punch-ins, drilled PVC pipe, 3D-printed PETG. Full detail in `manifold_options.md`.

**Key findings:**

| Finding | Result |
|---|---|
| Off-shelf 8-port manifold outlet type | **1/4" barb universally** (Orbit, Raindrip, Rain Bird, DIG) — not NPT-F. Direct fit for 1/4" ID silicone fill lines. |
| Off-shelf manifold inlet type | **1/2" FPT** on all models — needs one adapter to connect to 1/4" ID supply line |
| Gravity-feed compatibility | Non-pressure-compensating manifolds pass gravity flow freely with knobs open. Pressure-compensating emitter inserts block flow at gravity pressure — do not install. |
| Aquarium manifolds | Outlet barbs are 3/16" or ~5.5mm — too small for 1/4" ID fill lines. Not compatible. |

**User selection:** **Orbit 67000** (~$8) + **Rain Bird XT025** inlet adapter (1/4" barb × 1/2" FPT female, ~$1–2).

**Resolved open item:** Manifold outlet type confirmed as 1/4" barb. No NPT-F adapters needed at outlet side.

---

### LM2596 datasheet review — 2026-03-24

Reviewed Addicore AD281 LM2596 datasheet. Document is the National Semiconductor LM2596 IC datasheet — covers the 5-pin TO-220/TO-263 chip only. No barrel jack is present on the IC itself.

**Finding:** The LM2596 module board (Addicore AD281 and equivalent cheap modules) uses **screw terminals** for 12V input and 5V output — no barrel jack socket. The Meanwell PSU's 5.5×2.5mm barrel jack therefore does not plug into a module socket; instead the 12V is distributed via a pigtail wire from the barrel jack to screw terminals on the perfboard.

**Resolved open item:** Barrel jack adapter (5.5×2.5mm → 5.5×2.1mm) — **not applicable**. No adapter needed.

---

### Inline strainer replaced with copper mesh basket — 2026-04-22

**Decision:** Remove inline sediment strainer; use copper mesh basket inside the bucket instead.

**Rationale:** A copper mesh basket (100–200 mesh) formed over the bulkhead inlet opening inside the bucket does the same filtration job as an inline strainer while also providing the antimicrobial Cu²⁺ release already planned for water quality. Consolidates two separate items into one, removes a plumbing fitting from the line, and simplifies the supply chain.

**Changes made:**
- Inline strainer row removed from BOM
- Copper mesh sheet/roll added to BOM (replaces both strainer and previously separate copper insert)
- Fitting chain updated: bulkhead → valve directly (strainer hop removed)
- Section 1.3 water quality note updated to describe dual-purpose copper mesh basket

---

### Bucket-to-valve tube segment added — 2026-04-22

**Issue:** Removing the inline strainer left the fitting chain with no tubing or adapters specified between the bulkhead and the valve inlet. That segment was silently missing from the BOM.

**Changes made:**
- Added 1/2" NPT-F × 1/2" barb adapter (×1) — female NPT threads onto bulkhead's external male NPT thread; barb accepts supply tube
- 1/2" NPT-M × 1/2" barb adapter qty increased from 2 to 3 (valve inlet added to existing valve outlet + manifold inlet uses)
- 1/2" ID silicone spool bumped from 5 ft to 10 ft to cover both runs (bulkhead→valve and valve→manifold)
- Fitting chain, BOM, system diagram, and CLAUDE.md all updated

---

### Main feed line upsized to 1/2" ID — 2026-04-22

**Decision:** Use 1/2" ID tubing for the entire main supply run — from reservoir bulkhead through the valve to the manifold inlet.

**Rationale:** The manifold inlet is 1/2" FPT. Dropping to 1/4" ID between the valve outlet and the manifold inlet was an unnecessary restriction. Keeping full 1/2" bore throughout the supply path eliminates all restrictions above the manifold; the 1/4" barb outlets on the manifold itself are where the flow splits and naturally reduces.

**Changes made:**
- Main feed line: 1/4" ID × 1/2" OD → **1/2" ID × 3/4" OD** silicone, short spool (5 ft added to BOM)
- Valve outlet adapter: 1/2" NPT-M × 1/4" barb → **1/2" NPT-M × 1/2" barb** (qty 2 — one at valve outlet, one at manifold inlet)
- Rain Bird XT025 inlet adapter (1/4" barb × 1/2" FPT) removed — no longer needed
- Fitting chain and CLAUDE.md updated throughout

---

### Solenoid valve upsized to 1/2" NPT — 2026-04-22

**Decision:** Switch solenoid valve from 1/4" NPT to 1/2" NPT.

**Rationale:** A 1/4" NPT direct-acting solenoid valve has an internal orifice of ~3–4mm, which is smaller than the 6.35mm bore of the 1/4" ID fill tubing and far smaller than the 1/2" NPT upstream plumbing (bulkhead, strainer). The valve orifice was the system's primary flow restriction. A 1/2" NPT valve has a ~10–12mm orifice — the 1/4" ID tubing becomes the bottleneck, which is the correct behavior for a gravity-fed system trying to fill reservoirs quickly.

**Changes made:**
- Valve spec: 1/4" NPT → 1/2" NPT throughout CLAUDE.md and PROJECT_PLAN.md
- Valve outlet adapter: 1/4" NPT-M × 1/4" barb → 1/2" NPT-M × 1/4" barb (connects valve outlet to 1/4" ID main feed line)
- Fitting chain updated (bulkhead → strainer → valve inlet/outlet all remain 1/2" NPT; reduction to 1/4" barb happens at valve outlet adapter)

---

### Electrical interfaces documented — 2026-03-24

Added a full electrical interface table to PROJECT_PLAN.md covering every connection in the system:

| Interface | Connection type |
|---|---|
| Wall → Meanwell | IEC C14 / country plug, 120V AC |
| Meanwell → 12V rail | 5.5×2.5mm barrel jack → pigtail wire → perfboard screw terminal |
| 12V rail → solenoid (+) | Wire → DIN 43650A connector |
| 12V rail → LM2596 IN+ | Wire → screw terminal |
| LM2596 OUT+ → ESP32 5V | Wire → ESP32 5V/VIN pin |
| ESP32 GPIO → transistor base | Wire on perfboard → 470Ω → ZTX650/651 base |
| Transistor collector → solenoid (−) | Wire → DIN 43650A connector |
| Flyback diode | 1N4007 across solenoid coil, cathode to 12V |
| Common GND | Single ground bus: Meanwell GND sleeve, LM2596 IN−/OUT−, ESP32 GND, transistor emitter |

---

## 2026-04-28 — Hardware procurement begun

### Items ordered

| Item | Qty | Notes |
|---|---|---|
| LM2596 buck converter module (5-pack) | 5 | 1 used; 4 spares |
| 150ml Luer-slip syringes (3-pack) | 3 | For higher-volume plants |
| Copper mesh woven filter, 3m roll | 1 | Basket filter inside main reservoir over bulkhead inlet |
| Nylon zip ties, assorted sizes (600pc) | 1 pack | Syringe plunger locking + general cable management |
| DIN 43650A solenoid connector plug w/ LED | 1 | LED provides visual confirmation when valve is energized |
| Orbit 67000 8-port drip manifold (2-pack) | 2 | 1 used; 1 spare |

### Design decisions confirmed — 2026-04-28

- **All intermediate reservoirs are syringes.** PP lab bottles removed from design. 150ml Luer-slip syringes replace the PP bottle approach for higher-volume plants. All 8 plants use the same plunger-as-volume-limiter construction — no grommet drilling or sealant needed. PP bottles, grommet fittings, and silicone sealant removed from BOM.
- **Second Orbit 67000 is a spare.** BOM qty remains 1.
- **150ml syringe quantity updated to 6** (second 3-pack ordered). BOM updated accordingly.
- **DIN 43650 connector removed from BOM** — valve ships with connector included.
- **Syringe fill-port construction method confirmed:** 1/4" ID fill tube passes directly through a drilled hole in the plunger disc (no barb fitting); silicone sealant seals around the tube. Luer nozzle tip accepts 1/8" ID drip line by push-fit. 1/4" barb fittings (×8) removed from BOM; aquarium-grade silicone sealant added back.
- **`intermediate_reservoir_options.md` removed.** File contained research on 5 container types (small plastic bottles, Falcon/culture tubes, 3D-printed PETG, glass jars, repurposed food containers) and 4 outlet sealing methods (rubber grommet, threaded bulkhead, aquarium silicone, hot glue). Design is now settled on Luer-slip syringes for all 8 reservoirs, making the container research moot. Full content available in git history if needed.

---

## 2026-05-06 — Microcontroller switch; firmware rewrite; electronics assembly begun

### Microcontroller switched to Seeed Tiny BLE (nRF51822)

**Decision:** Replace ESP32 with Seeed Tiny BLE for the active build.

**Rationale:** Board was already on hand. Using it avoids waiting on ESP32 procurement and allows hardware assembly to begin immediately.

**Implications:**
- No WiFi, no NTP. Timing is millis()-based 24h interval. Drift ~2–4s/day — acceptable for daily plant watering.
- Schedule changes require re-uploading firmware via USB/SWD drag-and-drop.
- nRF51822 GPIO standard drive (~0.5mA) is insufficient to saturate ZTX650/651 at 500mA collector current through a 470Ω base resistor. H0H1 high-drive mode must be set via direct NRF_GPIO register write after `pinMode()` — firmware handles this.
- Upload method: compile in Arduino IDE (sandeepmistry/arduino-nRF5 board package, Nordic nRF51X22 Development Kit), then `cp watering_system.ino.bin /Volumes/MBED/` via Terminal. macOS Finder drag-and-drop fails silently.

**ESP32 / MicroPython path:** Preserved in `firmware/`, deferred until ESP32 is acquired.

---

### Firmware rewritten in Arduino C++ for Tiny BLE

`firmware_ble/watering_system/watering_system.ino` is the active firmware.

Key implementation decisions:
- `VALVE_PIN 4` (P4 on Tiny BLE header)
- `FILL_DURATION_S 2` — start low, calibrate upward until reservoirs fill without overflow
- `VALVE_MAX_ON_S 60` — hard safety cap regardless of config
- Valve pin written LOW before `pinMode(OUTPUT)` to guarantee closed state from first clock cycle
- UL suffixes on interval constant (`24UL * 60UL * 60UL * 1000UL`) prevent 32-bit overflow before type widening
- Unsigned subtraction for millis() rollover safety
- Waters immediately on first boot for circuit verification; then every 24h
- LED status: 3× green blink on boot, solid green while running, blue during valve open

---

### Transistor failure on first power-up; pull-down resistor added

**Incident:** ZTX650 destroyed on first power-up. Microcontroller was unplugged (intentionally, to protect it). Valve energized briefly, then transistor failed.

**Root cause:** With the microcontroller disconnected, the base pin was floating through the 470Ω resistor to nothing. A floating base picks up noise and capacitive coupling from nearby conductors, drifting above the ~0.6V threshold and partially or fully enabling the transistor. The solenoid coil, being an inductor, generated inductive spikes each time the transistor switched erratically. Repeated spikes with the transistor in a partially-on (high-resistance) state exceeded the junction's power dissipation; transistor failed collector-to-emitter short.

**Fix:** Added 10kΩ pull-down resistor from base to GND. Holds base firmly at 0V when GPIO is not actively driving it. Prevents partial conduction, erratic switching, and resulting spike damage. BOM and assembly docs updated.

**Solenoid:** Not damaged. Coil survived — it released energy into the spike but did not absorb a damaging voltage itself.

**Flyback diode orientation confirmed correct:** cathode (silver stripe) to 12V rail.

**Pull-down resistor color code:** Brown-Black-Orange-Gold (10kΩ, 5% tolerance). A 5th yellow band indicates military-grade reliability rating — value is still 10kΩ.

---

### Enclosure design completed

3D-printed two-part enclosure (body + lid) designed. Full detail in `ENCLOSURE_DESIGN.md`.

**Key decisions:**
- Single parting line serves as both the lid joint and the cable gland split — no separate gland hardware
- Power entry: IEC C8 panel-mount socket; 12V DC via Meanwell barrel jack pigtail inside
- Solenoid cable entry: 2-wire split gland; DIN 43650A detachable connector inside (preferred — matches connector family already on valve coil)
- USB access: remove lid; no panel port
- Cable gland bores: power 5.9mm (cable OD 6.35mm), solenoid 2.8mm (cable OD 3.175mm)
- Interior dimensions: 65mm × 85mm floor, 45mm height to parting line
- Board mounting: L-shaped corner extrusions from walls; bottom 2 corners full L-shape (supports weight), top 2 corners side tab only (allows straight insertion, avoids overhangs in print)
- Wire tension retention: boards sit slightly lower than natural wire resting point so wires press boards into supports — no extra fasteners

**Dimensions confirmed:**
- Perfboard: 40mm × 60mm (measured)
- Seeed Tiny BLE: 38.5mm × 42mm (measured)
- LM2596 module: TBD — measure before modeling corner extrusions

---

## 2026-05-13 — Firmware running; total project cost tallied

### Firmware confirmed working

Firmware flashed to Tiny BLE via MBED drag-and-drop (sandeepmistry nRF5 package, Generic nRF51 board). Valve alternating open/closed on 20s cycle confirmed. LED corrected: GREEN = pin 21, RED = pin 22 (initial assignment was swapped).

**Power path note:** Barrel jack → LM2596 → board is unresolved. LM2596 outputs 5V but Tiny BLE VCC pin expects 3.7–4.2V. Board currently powered via USB during development.

**Small syringes:** Not needed — all 8 plants will use 150ml syringes.

---

### Total project cost

All prices confirmed against actual purchases. Ranges use midpoints.

| Item | Cost |
|---|---|
| 12V solenoid valve (U.S. Solid, 1/2" NPT) | $35 |
| LM2596 buck converter 5-pack | $9 |
| HDPE bucket + Gamma-seal lid | $18 |
| Bulkhead fitting, barb adapters ×4, PTFE tape | $25 |
| Copper mesh roll (3m) | $13 |
| Orbit 67000 manifold (2-pack, 1 used) | $18 |
| Silicone tubing — 1/2", 1/4", 1/8" spools | $35 |
| 150ml Luer-slip syringes ×6 | $23 |
| Zip ties (600pc) | $13 |
| Silicone sealant + pegboard | $35 |
| Enclosure filament | $2 |
| Meanwell PSU, Seeed Tiny BLE, all electronics | $0 (reused / from stash) |
| **Total** | **~$226** |

Items at $0: Meanwell GST60A12 (reused), Seeed Tiny BLE (already owned), TIP121G, resistors, 1N4007, IEC C8 socket, perfboard, jumper wire, screw terminals (all from existing parts stash).

---
