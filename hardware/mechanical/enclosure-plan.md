# Enclosure plan (8-mic board)

Status 2026-11-02: planning only, no parts in hand. Cap decided (fabric over a printed plate); filament decided
(all PETG, §6); Fusion parameters and build steps in `fusion-build.md`. Board facts come from
`kicad/beamformer` at commit `f6136db`; earlier decisions come from `../layout-checklist.md`.

## 1. What the board forces on the enclosure

| Fact | Value | Consequence |
|---|---|---|
| Board | Ø 140 mm circle, 1.6 mm, 4 layers | Inner wall Ø ≈ 140.8 mm (0.4 mm side gap) |
| Orientation | **Components down.** All parts are on F.Cu; mic sound holes go through the board, so the bare B.Cu face points up and the mics "hear" upward like an Echo | Every part hangs into the base cavity |
| Mic holes | 8 × Ø 0.5 mm at r = 55 mm, 45° apart | Nothing over them except the grille, 2.5–3 mm above. No gaskets |
| Mounting holes | 3 × M3 NPTH (Ø 3.2) at r = 41.5 mm, 54° / 174° / 294° | 3 posts on an 83 mm circle |
| Tallest parts below the board | J1/J3 headers ≈ 8.5 mm, USB-C J2 ≈ 7.5 mm + the plug | Cavity depth set by the USB plug (see §4) |
| J2 (vertical USB-C) | r = 37.7 mm, 24° | Plug goes in from below; right-angle plug turns the cable sideways |
| J1 (SWD) | r = 44.4 mm, 312°, 3.45 mm from H3 | Post at H3 limited to Ø 6.5 mm |
| J3 (debug UART/TDM) | r ≈ 8 mm, near centre | Reached only by lifting the board out |
| LEDs D1–D16 | Side-emitting, r = 65 mm, 22.5° apart, facing outward | Light leaves just under the board edge, 5–6 mm from the wall: the translucent band goes there |
| Through-hole leads on the top face | J1/J3 pins, J2 shell stakes | Trim to < 1.5 mm so they stay under the grille gap |

Angles above are in the KiCad top view (counter-clockwise from +x). `firmware/final/README.md` assumes the board
is flipped left-to-right (MK1 stays at the top), so seen from above a KiCad angle θ sits at 180° − θ: holes at
126° / 6° / 246°, J2 at 156°, and mic 0 (MK7) at 0°, which is the front. Full table in `fusion-build.md` §3.
In Fusion, import the board STEP and flip it there instead of retyping positions.

## 2. Layout (top to bottom)

```
   ┌───────────── grille cap (fabric over printed or perforated plate) ─────────────┐  ← removable
   │  2.5–3 mm air gap  (M3 button heads, 1.65 mm, sit here)                       │
   ╞══════════════ PCB Ø140, mic holes facing up ══════════════════════════════════╡
   ▓▓ translucent band (4–6 mm tall), LEDs shine straight into it                 ▓▓
   │                                                                               │
   │  cavity: headers, USB plug, coiled cable           post   post   post        │  ← opaque base
   │                                                                     [USB-C   │
   └──── floor 2.5 mm ── feet ───────────────────────────────────────── panel] ───┘
```

| Layer | Height (mm) |
|---|---|
| Grille cap top plate (printed, grid infill) + fabric | 2.0 |
| Air gap over the board (decided: 2.5–3 mm, ≥ 30 % open grille) | 3.0 |
| PCB | 1.6 |
| Cavity under the board (provisional until the USB cable is measured, see §4) | 20–22 |
| Floor | 2.5 |
| Feet (stick-on bumpers) | 2–3 |
| **Total** | **≈ 31–33** |

Outer diameter = 140.8 + 2 × 3.6 mm wall = **148 mm** (the wall is 3.6 so a 1.5 mm stub is left for the cap
to slide over, see `fusion-build.md`). Fits the X1C's 256 mm bed easily, so each part prints in one
piece.

Acoustics: the walls only rise ~4.5 mm above the mic plane. At speech frequencies (wavelength 86 mm at 4 kHz)
that small rim does nothing measurable. Keep the cap edge rounded or chamfered rather than a tall lip.

## 3. Parts and how they go together

1. **Base** (opaque, one print with the translucent band, see §6): floor, wall, 3 posts, USB panel cutout, foot recesses.
2. **Board**: drops onto the posts, 3 × M3 button-head screws from the top into heat-set inserts in the posts.
3. **Grille cap**: its plate rests on a thin stub at the top of the base wall, and its fabric-wrapped skirt slides
   over the stub. The fabric folded inside the skirt is the friction fit. A fingernail notch at the back lifts it off.

Service: pull the cap, undo 3 screws, lift the board out on its USB cable (30 cm, coiled in the base). That gives
access to J1 (SWD) and J3. Normal firmware updates still go over SWD on the bench.

### Posts

- H1/H2: Ø 7 mm. H3: Ø 6.5 mm max (J1 is 3.45 mm away). 6.5 mm is the usual minimum boss for an M3 insert
  (4.0 mm hole, 4.6 mm insert), so it's tight: print a test coupon.
- Fallback if the H3 post splits: make all three posts plain Ø 3.4 mm through-holes and use M3 × 25 screws into nuts
  or inserts pocketed in the underside of the floor.
- Posts can widen below 10 mm (under J1's header height) for stiffness.
- Print the base upright so the posts' layers run across the screw load, not along it.

## 4. USB

BOM already has the usb-fever right-angle USB-C male → panel-mount female extension (30 cm, panel screws 17 mm apart).

- The plug goes **up** into J2 from inside the cavity. Its overmold sets the cavity depth: J2 is ~7.5 mm tall, then
  the right-angle head adds roughly 8–14 mm. **Measure the real plug when it arrives** before fixing the post height;
  until then design with 22 mm and keep it a Fusion parameter.
- Point the plug's cable exit toward the panel port and coil the rest along the wall.
- Put the panel port on the "back" of the device (opposite your 0° front mark).
- Panel cutout: model it from the cable's real flange; leave 0.3 mm clearance.

## 5. Top grille ("mesh")

There's no speaker: the grille is just a dust and finger cover over the mic holes that sound passes through.
Rule from the board review: **≥ 30 % open, 2.5–3 mm above the board, no gaskets.**

| Option | Look | How | Cost |
|---|---|---|---|
| **A. Fabric over a printed perforated plate (decided 2026-10-26)** | Echo / Nest look | Print a 1.6 mm plate whose middle the slicer turns into an open grid (~75 % open); wrap grille cloth over it and glue the edge inside the skirt (steps in `fusion-build.md` §6) | ~$10 |
| B. Fabric over perforated aluminium | Same look, stiffer, heavier, more premium | Cut a Ø ~145 mm disc from 0.032" perforated aluminium (e.g. ⅛" holes, 40 % open), deburr, wrap fabric the same way | ~$30–60 |
| C. Bare printed grille | Plain but clean | Hex or radial-slot pattern printed top-face-down on the textured plate | ~$1 |
| D. Bare machined/perforated metal | Industrial | As B without fabric | as B |

Fabric: open-weave polyester **speaker grille cloth**, e.g. Parts Express 260-335 (black) or 260-337 (gray),
about $10/yard at 70" wide, so one yard is enough for dozens of caps. Test: if you can breathe through it easily,
it's acoustically fine. Black or gray hides the board and screws; white grille cloth is see-through and very
bright. Avoid tightly woven fabric or felt.

Making it look good:
- Stretch the fabric evenly: wet it slightly or warm it, pull from opposite sides (12, 6, 3, 9 o'clock first).
- Round the plate's edge (≥ 2 mm fillet) so the fabric wraps without creasing.
- Put a black printed plate under the fabric so the board doesn't show through.
- A thin contrasting ring at the joint between cap and base (printed or machined aluminium) hides the seam.

## 6. Filament and the translucent band (X1C + AMS)

Decided 2026-11-02: **all PETG**. California Filament **black Matte PETG** for the base and cap, California Filament
**Cloudy White PETG** for the light band, black grille cloth on top.

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
1. Perforated aluminium plate under the fabric (option B): stiff, flat, premium feel.
2. Turned aluminium bottom plate or weight ring: makes the puck heavy so it doesn't slide and damps table knocks
   (also pair it with soft feet).
3. Thin aluminium accent ring at the cap/base seam.

Keep the light band and posts printed. Metal near the mics is fine acoustically as long as the open-area rule holds.

## 8. Order of work

| # | When | What |
|---|---|---|
| 1 | Now | Add GCT's USB4145 STEP to J2 (open checklist item), then export the board STEP from KiCad (*File → Export → STEP*) |
| 2 | Now | Order: grille cloth, M3 heat-set inserts (+ insert tip for the iron), M3 × 6/8 button-head screws (ISO 7380), stick-on silicone feet, California Filament Matte PETG + Cloudy White PETG |
| 3 | Now | Fusion: import the board STEP, flip it, build base and cap around it with parameters (cavity depth, gap, wall, band Z) |
| 4 | Before the final print | Test coupons (each < 30 min): (a) one post with an insert at Ø 6.5 mm; (b) 30 mm wall strips with the band in 2–3 materials/thicknesses, lit by any blue LED at 5.5 mm; (c) cap-to-rim fit; (d) a grille corner with fabric wrapped |
| 5 | USB cable arrives | Measure the plug, set cavity depth, model the panel cutout |
| 6 | Board assembled | Full print, fit check, then a clap/DOA test with the cap on vs off to confirm the grille is transparent |

## 9. Rough shopping list

| Item | Source | Approx. |
|---|---|---|
| Speaker grille cloth, black or gray, 1 yd | Parts Express 260-335 / 260-337 | $10 |
| M3 brass heat-set inserts (≈ 4.6 mm OD, 4.0 mm hole) | Ruthex / CNC Kitchen / McMaster | $10 |
| M3 × 6 and × 8 button-head screws | McMaster / Amazon | $8 |
| Silicone bumper feet | Amazon / 3M Bumpon | $5 |
| Black Matte PETG + Cloudy White PETG, 1 kg each | California Filament | ~$40 |
| 0.032" perforated aluminium, ⅛" holes 40 % open (option B only) | McMaster | $30–60 |
| USB-C right-angle → panel extension | already in the BOM | — |
