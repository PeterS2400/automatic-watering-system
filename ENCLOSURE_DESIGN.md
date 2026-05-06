# Enclosure Design Notes

3D-printed enclosure, two-part: main body + lid. Lid secured with screws. Single parting line shared by lid joint and cable glands.

---

## External Interfaces

All penetrations through the enclosure wall.

### 1. Power Entry — 12V DC from Meanwell PSU

| Property | Detail |
|---|---|
| Signal | 12V DC, up to 5A |
| External connector | IEC 60320 C8 panel-mount socket (already decided) |
| Cable from Meanwell | Barrel jack (5.5×2.5mm, center+) → pigtail → C8 plug |
| Strain relief | Split cable gland, 3D-printed into enclosure body |
| Water resistance | Split gland geometry seals around cable jacket |
| Inside termination | Pigtail wires to perfboard screw terminals (12V+, GND) |

### 2. Solenoid Cable Entry — 12V switched + GND

| Property | Detail |
|---|---|
| Signal | 12V DC, ~500mA switched; 2-wire |
| Strain relief | Split cable gland, 3D-printed into enclosure body |
| Water resistance | Split gland geometry seals around cable jacket |
| Detachable connector | TBD — options discussed: DIN 43650A, GX12/GX16, XT30, pluggable screw terminal |
| Connector location | Inside the enclosure wall — cable gland on outside, connector socket on inside |
| Outside cable | Solenoid power cable, already attached to valve, terminates in mating connector plug |

### 3. USB Access — Tiny BLE programming

No panel interface. Remove lid to access USB port.

---

## Internal Components

| Component | Orientation | Mounting method |
|---|---|---|
| Seeed Tiny BLE (nRF51822) | Sideways (standing on edge) | 4 L-shaped corner extrusions from wall; lid presses top edge down to lock |
| LM2596 buck converter module | Sideways (standing on edge) | 4 L-shaped corner extrusions from wall; lid presses top edge down to lock |
| Perfboard | Flat | Standoffs from floor + M3 screws; drill holes in perfboard to match |

**Corner extrusion detail:**
- **Bottom 2 corners:** Full L-shape — captures the side and bottom edge of the board. Supports the board's weight and prevents forward/downward movement.
- **Top 2 corners:** Side tab only — grips the sides of the board but has no lip over the top edge. Prevents lateral movement without obstructing insertion from above. Avoids overhangs in the print.
- Lid closes over the top edge to provide the only top retention.
- Board drops straight in from above and lifts straight out — no angling or squeezing required.

Board dimensions (measured):
- Perfboard: 40mm × 60mm
- Seeed Tiny BLE: 38.5mm × 42mm
- LM2596 buck converter: TBD — measure before modeling

**Wire tension retention:** The Tiny BLE and LM2596 are positioned slightly lower than their natural resting point as determined by the connected wiring. The slight downward offset puts the wires in gentle tension/bend, which presses the boards down into their corner supports and prevents rattling without any additional fasteners.

Orient each board so tallest components face into available clearance.

---

## Dimensions

- Body interior height (floor to parting line): 45mm
- Body interior floor: 65mm × 85mm

## Lid

- Secured with screws (count and size TBD)
- Consider heat-set brass inserts in the body for cleaner screw attachment
- Lid removal provides USB access for firmware updates

---

## Split Cable Gland Design Notes

Both cable penetrations are fully integrated into the 3D-printed enclosure — no purchased glands, no separate clamp parts.

**Split geometry:**
- Single parting line: the body/lid split IS the gland split
- Lower half of each gland is part of the enclosure body; upper half is part of the lid
- Screwing the lid down clamps the cable — no additional hardware

**Bore and fit:**
- Power cable OD: 1/4" (6.35mm) → model bore at ~5.9mm
- Solenoid cable OD: 1/8" (3.175mm) → model bore at ~2.8mm
- Slight interference fit (~0.3–0.5mm under cable OD) so the lid clamps the jacket when tightened
- Interior shoulder or step inside the enclosure wall prevents the cable being pulled inward under tension

**Water resistance:**
- Exterior chamfer sheds water away from the opening
- Optionally apply a thin bead of silicone sealant inside the bore at final assembly for a positive seal

---

## Open Items

- Finalize solenoid-side detachable connector (DIN 43650A is the natural fit — same family as the connector on the valve coil itself)
- Decide USB access method (panel port vs. lid removal)
- ~~Measure actual cable ODs before modeling gland bores~~ **Resolved:** power 6.35mm, solenoid 3.175mm
- Determine screw size and count for lid
- Determine internal mounting strategy for perfboard and LM2596 module (standoffs, snap clips, or adhesive)
