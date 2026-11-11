# Enclosure plan (8-mic board)

Status 2026-11-03: planning only, no parts in hand. Top decided (solid printed top with 8 pinholes and foam
gaskets, screws from below, §5); filament decided (all PETG, §6); Fusion parameters and build steps in
`fusion-build.md`. Board facts come from `kicad/beamformer` at commit `f6136db`; earlier decisions come from
`../layout-checklist.md`.

## 1. What the board forces on the enclosure

| Fact | Value | Consequence |
|---|---|---|
| Board | Ø 140 mm circle, 1.6 mm, 4 layers | Inner wall Ø ≈ 140.8 mm (0.4 mm side gap) |
| Orientation | **Components down.** All parts are on F.Cu; mic sound holes go through the board, so the bare B.Cu face points up and the mics "hear" upward like an Echo | Every part hangs into the base cavity |
| Mic holes | 8 × Ø 0.5 mm at r = 55 mm, 45° apart | One Ø 1 mm pinhole in the top over each, sealed to the board by a foam gasket (§5) |
| Top face around each mic hole | Solder mask only. Nearest B.Cu track or via 0.8–1.9 mm from the hole centre, vias tented both sides; nearest exposed pad ≥ 10 mm away | Flat enough for a soft foam gasket to seal |
| Mounting holes | 3 × M3 NPTH (Ø 3.2) at r = 41.5 mm, 54° / 174° / 294° | 3 hollow posts below, 3 screw bosses in the top above; screws come up from the floor |
| Tallest parts below the board | J1/J3 headers ≈ 8.5 mm, USB-C J2 ≈ 7.5 mm + the plug | Cavity depth set by the USB plug (see §4) |
| J2 (vertical USB-C) | r = 37.7 mm, 24° | Plug goes in from below; right-angle plug turns the cable sideways |
| J1 (SWD) | r = 44.4 mm, 312°, header body 3.45 mm from H3 | Post at H3 limited to Ø 6.5 mm. On the top face J1's pads are 4.9 mm from H3, so a Ø 7 top boss clears them |
| J3 (debug UART/TDM) | r ≈ 8 mm, near centre | Reached only by lifting the board out |
| LEDs D1–D16 | Side-emitting, r = 65 mm, 22.5° apart, facing outward | Light leaves just under the board edge, 5–6 mm from the wall: the translucent band goes there |
| Through-hole leads on the top face | J1/J3 pins, J2 shell stakes | Trim to < 1.5 mm so they clear the 2 mm gap under the top |

Angles above are in the KiCad top view (counter-clockwise from +x). `firmware/final/README.md` assumes the board
is flipped left-to-right (MK1 stays at the top), so seen from above a KiCad angle θ sits at 180° − θ: holes at
126° / 6° / 246°, J2 at 156°, and mic 0 (MK7) at 0°, which is the front. Full table in `fusion-build.md` §3.
In Fusion, import the board STEP and flip it there instead of retyping positions.

## 2. Layout (top to bottom)

```
   ┌──────── solid top, 3.5 mm, 8 pinholes Ø1 at r = 55 ─────────────────────────────┐
   │  2 mm gap: 3 screw bosses (inserts) + 8 mic bosses with foam gaskets          │
   ╞══════════════ PCB Ø140, mic holes facing up ══════════════════════════════════╡
   ▓▓ translucent band (4–6 mm tall), LEDs shine straight into it                 ▓▓
   │                                                                               │
   │  cavity: headers, USB plug, coiled cable     hollow posts (screws up through)  │  ← opaque base
   │                                                                     [USB-C   │
   └──── floor 2.5 mm ── feet ── screw heads in counterbores ────────── panel] ───┘
```

| Layer | Height (mm) |
|---|---|
| Top plate (printed, solid) | 3.5 |
| Gap over the board (bosses and gaskets live here) | 2.0 |
| PCB | 1.6 |
| Cavity under the board (provisional until the USB cable is measured, see §4) | 20–22 |
| Floor | 2.5 |
| Feet (stick-on bumpers) | 2–3 |
| **Total** | **≈ 32–35** |

Outer diameter = 140.8 + 2 × 3.6 mm wall = **148 mm**. The top's skirt covers a 2.2 mm stub at the top of the
wall and ends flush with the base (see `fusion-build.md`). Fits the X1C's 256 mm bed easily, so each part prints
in one piece.

Acoustics: the pinholes are flush in a flat top, so nothing around them shadows the sound. Keep the top's outer
edge chamfered or rounded rather than a raised lip.

## 3. Parts and how they go together

1. **Base** (opaque, one print with the translucent band, see §6): floor, wall, stub, 3 hollow posts with
   counterbores underneath, USB panel cutout, foot recesses.
2. **Board**: sits on the posts. Nothing screws into the base.
3. **Top**: solid plate with 8 pinholes, 8 short mic bosses carrying the gaskets, 3 screw bosses with heat-set
   inserts, and a skirt over the stub.
4. **Screws**: 3 × M3 up from under the floor, through each post and the board's mounting hole, into the top's
   inserts. They clamp post, board and top together; the boss heights set the gap and the gasket squeeze.

Assembly order:
1. Heat-set the 3 inserts into the top's screw bosses.
2. Stick a gasket on each of the 8 mic bosses, centred on the pinhole.
3. Mount the USB panel jack in the base wall and coil the cable inside.
4. Plug the cable into J2 from below, then lower the board onto the posts (mic 0 to the front mark).
5. Lower the top on. Its skirt and the screw bosses locate it; the pinholes land over the mic holes.
6. Holding top, board and base together, turn it over and drive the 3 screws from below until snug.
   The bosses bottom out on the board, so don't over-tighten.
7. Leak test (§5).

Service: undo the 3 screws from below, lift the top off, lift the board out on its USB cable (30 cm, coiled in
the base). That gives access to J1 (SWD) and J3. Normal firmware updates still go over SWD on the bench. The
gaskets stay on the top and can be reused a few times; cut spares.

### Posts

- H1/H2: Ø 7 mm. H3: Ø 6.5 mm max (J1's header body is 3.45 mm away).
- The posts are now plain Ø 3.4 mm through-holes with a Ø 6.5 mm counterbore from below for the screw head, so the
  old insert-splitting risk at H3 is gone: the posts only carry compression.
- The counterbore depth sets the screw's reach. It's a Fusion parameter (`z_seat`) derived from the screw length;
  re-check it once the cavity depth is final (§4).
- Posts can widen below 10 mm (under J1's header height) for stiffness; the counterbore sits in that wide part.
- Print the base upright.
- The screw heads show on the bottom. Leave them, or hide them under one large adhesive rubber or cork disc
  instead of 3 small feet, as Echo Dots do.

## 4. USB

BOM already has the usb-fever right-angle USB-C male → panel-mount female extension (30 cm, panel screws 17 mm apart).

- The plug goes **up** into J2 from inside the cavity. Its overmold sets the cavity depth: J2 is ~7.5 mm tall, then
  the right-angle head adds roughly 8–14 mm. **Measure the real plug when it arrives** before fixing the post height;
  until then design with 22 mm and keep it a Fusion parameter.
- Point the plug's cable exit toward the panel port and coil the rest along the wall.
- Put the panel port on the "back" of the device (opposite your 0° front mark).
- Panel cutout: model it from the cable's real flange; leave 0.3 mm clearance.

## 5. Top: solid plate, pinholes and gaskets (decided 2026-11-03)

Echo-style: a flat matte black top with a ring of 8 pinholes, one over each mic. Each pinhole is joined to its
mic hole by a small closed-cell foam ring squeezed between a boss on the top and the bare top face of the board,
so every mic gets its own sealed path to the room.

**Why the gaskets are required.** The air under any top acts as a Helmholtz resonator (a bottle). Rough estimates
with the formula in InvenSense AN-1003:

| Top | Resonance | Verdict |
|---|---|---|
| Solid top, 8 pinholes, **no gaskets** (all mics share one 140 mm × 2–3 mm air layer) | ≈ 300–500 Hz | Inside the 300–4000 Hz DOA band and the speech band, plus sound leaks from one mic's hole to the others. Not usable |
| Solid top with gaskets (vent Ø 1 × 4.3 mm, gasket bore Ø 1.5 × 1.2 mm) | ≈ 15 kHz | Above the speech band |
| Open printed grille, ≥ 30 % open over a 3 mm gap | ≈ 9 kHz | Above the speech band |
| Grille cloth over a ~75 % open printed frame | ≈ 13 kHz | Above the speech band |

These are order-of-magnitude numbers (AN-1003 says the formula only locates the resonance roughly). The point is
that a solid top only works if every gasket seals. A leaking gasket reconnects that mic to the shared gap.

**Gasket**

| Item | Value | Why |
|---|---|---|
| Material | Closed-cell foam, adhesive on one side: silicone foam or Rogers PORON (best), or closed-cell neoprene foam tape | AN-1003: rubber, silicone, neoprene or closed-cell foam; stiffness barely matters. **Never open-cell**: sound passes straight through it |
| Free thickness | 1/16" (1.6 mm) | Squeezed to 1.2 mm (25 %) by the bosses |
| Shape | Ring, 5 mm OD, 1.5 mm ID | ID is 3 × the 0.5 mm board hole, so ±0.5 mm misalignment doesn't block it. AN-1003: keep the gasket cavity as small as possible |
| Cutting | Hollow punch set: punch the 1.5 mm hole first, then the 5 mm outline around it | Cleaner than a knife; cut 16+ so there are spares |
| Mounting | Adhesive side on the top's mic boss, foam side onto the board | The gaskets come off with the top for service |

Use the softest foam you can get, so 8 gaskets don't bow the plate between the screws.

**Pinholes and bosses** (parameters in `fusion-build.md`)
- Pinholes Ø 1.0 mm, positions taken from the board STEP's mic holes. Print them at 0.8 mm and drill to 1.0 mm with
  a pin vise: small printed holes come out undersized and out of round, drilling makes all 8 identical.
- Mic bosses Ø 5.5 mm, 0.8 mm tall. Screw bosses Ø 7 mm, 2.0 mm tall: they bottom out on the board, which fixes
  the 2.0 mm gap and the gasket squeeze.
- Top plate 3.5 mm, so the short inserts (≈ 4 mm long, in a 4.5 mm deep hole) leave a 1 mm skin on the top face.
  Use a low iron temperature and stop flush; check the top face for a bump on the coupon first.

**Leak test** (after assembly): stream audio, play steady noise from a phone above the top, then cover one pinhole
with tape. That mic's level should drop a lot (aim for ≥ 20 dB) while the others barely move. If it doesn't drop,
that gasket leaks: re-seat or replace it. Repeat for all 8.

**Fallback** if the gaskets can't be made to seal: a grille-cloth top over a printed open frame (≈ 13 kHz, no
gaskets) fits the same base, screwing from below the same way. Other rejected options: a bare printed grille
(looks unfinished) and perforated metal (needs machining, same acoustics as the cloth).

Sources: InvenSense AN-1003 (mounting MEMS mics, gasket rules and the resonance formula), ST AN4427 (gasket design
for MEMS mics), InvenSense note on sealing bottom-port mics, and an element14 Echo Dot 2nd gen teardown, which
shows the mic holes passing through a shaped foam pad on the mic board.

## 6. Filament and the translucent band (X1C + AMS)

Decided 2026-11-02: **all PETG**. California Filament **black Matte PETG** for the base and top, California Filament
**Cloudy White PETG** for the light band.

- Everything is PETG, so the band bonds to the base in one print. **Don't mix PLA with PETG in one part**: they
  barely stick to each other (people use one as support interface for the other). If any PLA is used, keep it to
  separate parts (e.g. a band ring trapped between a lip and the board seat).
- Cloudy White is milky, between solid white and clear (sold as a lithophane material), so it diffuses better than
  clear-ish PETG Translucent and doesn't tint the blue LEDs. It's fairly opaque when thick: keep the band walls thin
  (start at 1.2 mm).
- Print settings: no official numbers on the product page; one user ran it at 255 °C nozzle / 80 °C bed on an X1C.
  Start around 240–255 / 70–80, low part-cooling fan. On smooth PEI use glue stick as a release layer (PETG can tear
  the surface). Textured PEI is fine bare.
- Black body gives the best contrast, but the inside of the light chamber (ledge under the band, wall next to the
  LEDs) must be white or cloudy, not black, or it absorbs the light that spreads between LEDs. Start the band's
  height-range modifier 1–2 mm below `z_band` so the ledge's top face prints cloudy too.
- Matte PETG usually gets its finish from a filler, which can make it a little more brittle than plain PETG. Check
  the insert coupon for cracking.

**Printing translucent + opaque in one part:** in Bambu Studio, right-click the base → *Height range modifier* →
set the band's Z range (e.g. 4–6 mm under the board's seat) to the translucent filament. Since the whole slice
is translucent, the AMS swaps filament only twice (at the band's bottom and top), so there's almost no purge
waste. The posts are also translucent in that range, which nobody sees.

If you want **opaque baffles** between LEDs (crisp 16 segments instead of a glow), they share the band's Z range,
so the AMS swaps every layer there (~20–30 swaps). Use *Flush into object's infill* to cut waste. Or print the
band as a separate ring and glue it in.

Diffusion is the main unknown: the LEDs are 27 mm apart but only ~5.5 mm from the wall, so expect hot spots. Each
LED lights a patch about 1.3 × its distance wide, and patches only blend when the distance is ~0.7–1 × the spacing
(18–25 mm, which would make the enclosure Ø 175–190 mm). Plan instead: grow the gap moderately (9–12 mm, Ø ≈ 155–160)
and let the firmware draw the fade (Gaussian over 3–4 LEDs). Things that help, most useful first:
- **Vertical flutes** on the band's inner face (half-round, ~1.5–2 mm pitch): they spread each LED sideways around
  the ring. Horizontal layer lines only spread light up and down.
- **Two thin skins with an air gap** (e.g. 0.8 mm + 1.5–2 mm gap + 1.2 mm) instead of one thick wall, or a 4–5 mm
  band with 2 walls and ~20 % gyroid infill.
- **White chamber** above and below the LEDs (see the black-body note above).
- Outer surface frosted: fuzzy skin or light sanding.
- Opaque baffles (below) only if you want 16 crisp segments instead of a glow.

The band coupon must have **3 LEDs at the real 27 mm spacing**: blending between neighbours is what matters, not one
LED's spot. Make the LED-to-wall distance a Fusion parameter and pick it from the coupon.

## 7. Using the machining equipment

Metal is optional. Ideas in order of payoff:
1. Turned aluminium bottom plate or weight ring: makes the puck heavy so it doesn't slide and damps table knocks
   (also pair it with soft feet).
2. Thin aluminium accent ring at the top/base seam.

Keep the top, light band and posts printed: the pinholes and bosses must line up with the board, which is easiest
in one printed part.

## 8. Order of work

| # | When | What |
|---|---|---|
| 1 | Now | Add GCT's USB4145 STEP to J2 (open checklist item), then export the board STEP from KiCad (*File → Export → STEP*) |
| 2 | Now | Order: gasket foam, hollow punch set, short M3 heat-set inserts (+ insert tip for the iron), M3 × 25 button-head screws (ISO 7380), pin vise + 1.0 mm drills, stick-on silicone feet, California Filament Matte PETG + Cloudy White PETG |
| 3 | Now | Fusion: import the board STEP, flip it, build base and top around it with parameters (cavity depth, gap, wall, band Z, screw length) |
| 4 | Before the final print | Test coupons (`fusion-build.md` §7, each < 30 min): (a) top screw boss with an insert; (b) mic boss + pinhole + gasket; (c) top-to-stub fit ring; (d) 60° band arc with 3 LEDs |
| 5 | USB cable arrives | Measure the plug, set cavity depth, re-check `z_seat` and the screw length, model the panel cutout |
| 6 | Board assembled | Full print, fit check, tape leak test on all 8 pinholes (§5), then a clap/DOA test with the top on vs off |

## 9. Rough shopping list

| Item | Source | Approx. |
|---|---|---|
| Closed-cell foam sheet, 1/16" (1.6 mm), adhesive one side: silicone foam or PORON (or closed-cell neoprene tape) | McMaster-Carr / Rogers PORON distributors / hardware store | $10–20 |
| Hollow punch set (includes ~1.5 mm and 5 mm) | Amazon / Harbor Freight | $15 |
| Short M3 brass heat-set inserts (≈ 4 mm long; check the hole size on the maker's drawing) | Ruthex / CNC Kitchen / McMaster | $10 |
| M3 × 25 button-head screws (buy a few lengths, 20–30, until the cavity is final) | McMaster / Amazon | $8 |
| Pin vise + 1.0 mm drill bits | Amazon / hobby shop | $10 |
| Silicone bumper feet, or one large adhesive rubber/cork disc | Amazon / 3M Bumpon | $5 |
| Black Matte PETG + Cloudy White PETG, 1 kg each | California Filament | ~$40 |
| USB-C right-angle → panel extension | already in the BOM | — |
| Fallback only: speaker grille cloth, black, 1 yd | Parts Express 260-335 | $10 |
