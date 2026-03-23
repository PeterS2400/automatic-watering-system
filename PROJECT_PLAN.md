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

## Phase 1 — Hardware Design & Procurement

### 1.1 Solenoid Valve
- **Selected:** U.S. Solid 1/4" Brass Solenoid Valve, 12V DC, Normally Closed, VITON seal
- VITON seal is chemically resistant — compatible with H₂O₂ water treatment
- 1/4" NPT fittings — standard, easy to plumb
- Normally closed = safe default (valve stays closed if power is lost)
- **Direct-acting** (confirmed) — opens at zero pressure, fully compatible with gravity-fed reservoir at any height
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
- Add a **copper mesh insert or copper piece** at the bottom; slow copper ion release is naturally antimicrobial and safe for plants at trace levels
- Fit a **fine mesh filter** at the reservoir outlet to catch sediment and biofilm before it enters tubing and clogs drip lines
- Periodically add a small dose of **food-grade hydrogen peroxide** (~1–3ml of 3% H₂O₂ per liter) to kill bacteria and algae; breaks down into water and oxygen, safe for plants
- **Flush and refill** every 1–2 weeks to prevent stagnation; plan a refill reminder into the schedule

### 1.4 Manifold & Plumbing
- Main gravity reservoir → valve inlet (Option A: single valve; Option B: valve → manifold → per-plant valves)
- Valve outlet → manifold (T-fittings or drilled bar)
- **Option A:** manifold → intermediate reservoirs → thin drip tubes to plants
- **Option B:** manifold → one direct drip line per plant

### 1.5 Electronics BOM

**Common to both options:**
- Arduino Uno or Nano
- 470Ω resistor per valve (base resistor)
- 1N4007 or similar flyback diode per valve
- DS3231 RTC module + CR2032 coin cell
- 12V DC power adapter (≥1A for Option A; ≥1A × number of simultaneously open valves for Option B)
- 5V supply for Arduino (USB or 7805 from 12V rail)
- Breadboard/PCB, jumper wires, connectors

**Option A additions:**
- 1× ZTX650 or ZTX651 NPN transistor

**Option B additions:**
- 1× ZTX650/651 NPN transistor per plant (5–8 total)
- Or: pre-built relay module or MOSFET driver board to simplify multi-channel switching

---

## Phase 2 — Electronics Assembly

### 2.1 Transistor Switch Circuit (per valve)
- Wire Arduino GPIO → 470Ω resistor → ZTX650 base
- Collector → solenoid coil (–), solenoid coil (+) → 12V
- Flyback diode across solenoid (cathode to 12V)
- Common GND between Arduino and 12V supply
- Option B: repeat circuit for each valve; assign one Arduino digital pin per valve

### 2.2 RTC Module
- Connect DS3231 to Arduino via I2C (SDA/SCL)

### 2.3 Bench Test
- Verify each transistor saturates and valve clicks open/closed
- Confirm no voltage spikes on Arduino GPIO (check flyback diode is working)

---

## Phase 3 — Firmware

### 3.1 Core Valve Control

**Option A:**
- Single GPIO HIGH/LOW to open/close the one valve
- Valve open duration = time to fill all intermediate reservoirs to capacity

**Option B:**
- One GPIO pin per plant valve
- Each valve opened for a configured duration (e.g. `int waterDuration[8] = {5, 10, 8, ...}` in seconds)
- Open all valves simultaneously, close each independently when its duration elapses

### 3.2 RTC Scheduling
- Read current time from DS3231
- Trigger watering at a configured time-of-day
- Configurable schedule (hardcoded constant or serial-configurable)

### 3.3 Safety Logic
- Maximum valve open duration cap per valve (prevents flooding if something jams)
- Optional: blink onboard LED during active watering cycle

### 3.4 Serial Debug Output
- Log watering events with timestamp over Serial for initial testing

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
- Web/app interface for schedule changes (would require ESP32 swap)
