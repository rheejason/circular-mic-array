# Fusion build: base and top

Companion to `enclosure-plan.md`. Decided 2026-11-03: **solid printed top with 8 pinholes and foam gaskets, held by
3 screws from below** (replaces the 2026-10-26 fabric cap; reasons in `enclosure-plan.md` §5).
All sizes are Fusion user parameters, so the cavity depth can change once the USB plug is measured.

## 1. How the pieces meet (section through the wall)

Heights (z = 0 at the underside of the floor, before feet):

| z (mm) | Level |
|---|---|
| 31.6 | Top surface (pinholes open here) |
| 28.1 | Top plate underside = stub top |
| 27.3 | Mic boss faces (gaskets stuck here, squeezed to 1.2 mm against the board) |
| 26.3 | Top skirt bottom edge |
| 26.1 | Board top = **shoulder** = visible seam. The top's screw bosses land here |
| 24.5 | Board bottom = post tops; LEDs hang just below |
| 19.5 | Translucent band bottom (band runs 19.5 → 26.1) |
| 4.8 | Screw-head seat at the top of each counterbore (with M3 × 25 screws) |
| 2.5 | Floor top |

Radii:

| r (mm) | Edge |
|---|---|
| 70.0 | Board edge |
| 70.4 | Wall / stub inside face |
| 72.4 | Band inside face (recessed, band 1.6 thick) |
| 72.6 | Stub outside face |
| 72.8 | Skirt inside face |
| 74.0 | Skirt outside face = base outside face |

- The board sits on 3 hollow posts. Its top face is the **shoulder** where the base wall steps in to a thin **stub**.
- The top's 3 screw bosses press on the board, and 3 screws from below pull the top down. Post, board and top are
  clamped together, and the boss heights fix the 2 mm gap and the gasket squeeze.
- The top's skirt hangs over the stub with 0.2 mm clearance (alignment and looks only; the screws hold it).
- From outside you see: flat black top with 8 pinholes, a thin seam, the light band, then the opaque base.

## 2. Parameters

*Modify → Change Parameters → +*. Enter them in this order (later ones use earlier ones).

| Name | Expression | Value | Meaning |
|---|---|---|---|
| `board_d` | `140 mm` | 140 | PCB diameter |
| `board_t` | `1.6 mm` | 1.6 | PCB thickness |
| `side_clr` | `0.4 mm` | 0.4 | Radial gap, board edge to wall |
| `wall` | `3.6 mm` | 3.6 | Base wall |
| `inner_d` | `board_d + 2 * side_clr` | 140.8 | Wall inside diameter |
| `outer_d` | `inner_d + 2 * wall` | 148.0 | Outside diameter |
| `floor_t` | `2.5 mm` | 2.5 | Floor thickness |
| `cavity` | `22 mm` | 22 | Floor top to board bottom. **Set from the measured USB plug** |
| `gap` | `2 mm` | 2.0 | Board top to top-plate underside. Clears through-hole leads trimmed to < 1.5 mm |
| `z_board` | `floor_t + cavity` | 24.5 | Board bottom = post tops |
| `z_shoulder` | `z_board + board_t` | 26.1 | Board top = shoulder = seam |
| `z_plate` | `z_shoulder + gap` | 28.1 | Top-plate underside = stub top |
| `top_t` | `3.5 mm` | 3.5 | Top plate thickness (leaves a 1 mm skin over the inserts) |
| `fit_clr` | `0.2 mm` | 0.2 | Radial clearance, skirt to stub |
| `skirt_t` | `1.2 mm` | 1.2 | Top skirt thickness |
| `top_od` | `outer_d` | 148.0 | Top OD, flush with the base |
| `skirt_id` | `top_od - 2 * skirt_t` | 145.6 | |
| `stub_od` | `skirt_id - 2 * fit_clr` | 145.2 | Stub is (145.2 − 140.8)/2 = 2.2 mm thick |
| `skirt_h` | `gap - 0.2 mm` | 1.8 | Skirt length below the plate; stops 0.2 mm above the shoulder |
| `pin_d` | `1.0 mm` | 1.0 | Finished pinhole (drilled) |
| `pin_print_d` | `0.8 mm` | 0.8 | Pinhole as modelled and printed, drilled out to `pin_d` |
| `gasket_t` | `1.6 mm` | 1.6 | Gasket free thickness (1/16" foam). **Measure the real sheet** |
| `gasket_squeeze` | `0.4 mm` | 0.4 | Compression, 25 % of 1.6 |
| `gasket_od` | `5 mm` | 5.0 | Gasket ring outside (punch size) |
| `gasket_id` | `1.5 mm` | 1.5 | Gasket ring hole (punch size) |
| `mic_boss_d` | `gasket_od + 0.5 mm` | 5.5 | Mic boss under each pinhole |
| `mic_boss_h` | `gap - (gasket_t - gasket_squeeze)` | 0.8 | Leaves 1.2 mm for the squeezed gasket |
| `top_boss_d` | `7 mm` | 7.0 | Screw bosses under the top (J1's top-face pads are 4.9 mm from H3, so 7 clears them) |
| `top_insert_hole_d` | `4.0 mm` | 4.0 | Hole for a short M3 insert (check the insert maker's drawing) |
| `top_insert_depth` | `4.5 mm` | 4.5 | ≈ 4 mm insert + 0.5 mm; leaves `gap + top_t − 4.5` = 1.0 mm skin |
| `post_r` | `41.5 mm` | 41.5 | Mounting-hole circle radius |
| `post_d` | `7 mm` | 7.0 | Posts at H1, H2 |
| `post_d_h3` | `6.5 mm` | 6.5 | Post at H3 (J1's header body is 3.45 mm away) |
| `post_wide_d` | `10 mm` | 10 | Lower, wider part of each post (holds the counterbore) |
| `post_narrow_h` | `10 mm` | 10 | Narrow length at the post top (clears J1's 8.5 mm header) |
| `screw_clr_d` | `3.4 mm` | 3.4 | Screw clearance hole through each post |
| `cbore_d` | `6.5 mm` | 6.5 | Counterbore for the M3 button head (5.7 mm) |
| `screw_len` | `25 mm` | 25 | M3 screw length under the head |
| `insert_engage` | `3.7 mm` | 3.7 | How far the screw reaches into the top's insert |
| `z_seat` | `z_shoulder + insert_engage - screw_len` | 4.8 | Head seat = counterbore depth from the bottom face |
| `band_h` | `5 mm` | 5.0 | Band height below the board bottom |
| `band_t` | `1.6 mm` | 1.6 | Band thickness (4 perimeters) |
| `z_band` | `z_board - band_h` | 19.5 | Band bottom |
| `usb_w` | `12 mm` | 12 | Panel cutout width. **Placeholder until the cable arrives** |
| `usb_h` | `7 mm` | 7 | Panel cutout height. Placeholder |
| `usb_pitch` | `17 mm` | 17 | Panel screw spacing (from the listing) |
| `usb_screw_d` | `3.2 mm` | 3.2 | Panel screw hole. Placeholder |
| `foot_d` | `8.5 mm` | 8.5 | Recess for stick-on feet |
| `foot_r` | `60 mm` | 60 | Feet circle radius |

When `cavity` changes, `z_seat` moves with it. Keep it between ~1 mm and `floor_t + cavity - post_narrow_h`
(the counterbore must stay in the wide part of the post); otherwise change `screw_len` to a stock length.

## 3. Angles after the flip

`firmware/final/README.md` assumes the board is flipped **left-to-right (MK1 stays at the top)**. Seen from above,
a KiCad angle θ then sits at 180° − θ:

| | KiCad view | Seen from above (enclosure) |
|---|---|---|
| H1 / H2 / H3 | 54° / 174° / 294° | **126° / 6° / 246°** |
| J2 USB-C | 24° | 156° (near the back, good for the cable) |
| J1 SWD | 312° | 228° |
| Mic 0 = **front** | MK7 at 180° | **0° (+x)** |
| Mic holes | 8 × 45° at r = 55 | 8 × 45° at r = 55, MK7 at 0° |

So: front mark at 0°, USB panel port at 180°.

## 4. Build steps

Fusion with Z up (*Preferences → General → Default modeling orientation: Z up*). Make two components, **Base** and
**Top**, plus the imported **Board**. Activate the right component before each step.

### 4.1 Import the board (for checking and for hole positions)

1. In KiCad, add the USB4145 STEP to J2 first, then *File → Export → STEP* (board only, include 3D models).
2. Fusion: *Insert → Insert Mesh/Upload*, or drag the .step in and insert it into the design as a component named Board.
3. Rotate it 180° about the axis through MK1 and MK5 (the left-to-right flip). Check: MK7 lands on +x, H2 at ~6°.
4. Move it so the board centre is on the Z axis and its **bottom face** (the component side, now facing down) is
   at `z_board`. Right-click → *Ground*.
5. Check that the 8 Ø 0.5 mic holes and the 3 Ø 3.2 mounting holes show on the board's top face. Every hole
   position below is projected from them, never retyped.

### 4.2 Base

1. **Floor.** Sketch on XY: circle `outer_d`. Extrude `floor_t`. New body.
2. **Wall.** Sketch on XY: circles `inner_d` and `outer_d`. Extrude the ring, *Start: offset* `floor_t`,
   distance `z_shoulder - floor_t`, *Join*.
3. **Stub.** Sketch on XY: circles `inner_d` and `stub_od`. Extrude the ring, *Start: offset* `z_shoulder`,
   distance `gap`, *Join*. The stub top is now at `z_plate`.
4. **Posts.** Sketch on the floor's top face. *Project* (P) the 3 mounting-hole edges from the Board, and put a
   circle on each centre: `post_d` at H1 and H2, `post_d_h3` at H3. Extrude to `cavity`, *Join*.
   Then a second sketch on the floor top with `post_wide_d` circles at the same centres, extruded
   `cavity - post_narrow_h`, *Join*. Check that H3's wide part still clears J1 (step 4.4).
5. **Screw holes.** *Create → Hole* on the floor's **bottom** face at each post centre: *Counterbore*, diameter
   `screw_clr_d`, extent *All*, counterbore diameter `cbore_d`, counterbore depth `z_seat`.
6. **Band recess (inside face).** Sketch on the XZ plane. Draw the profile to remove:
   from r = `inner_d / 2` to r = `outer_d / 2 - band_t`, from z = `z_band` up to `z_shoulder - 2 mm`, with a
   45° slope at the top meeting the wall at `z_shoulder`. The slope stops the stub hanging over a flat ledge
   when printing. *Revolve*, axis Z, 360°, *Cut*.
7. **USB panel cutout.** *Construct → Tangent plane* on the outer wall at 180°. Sketch: rectangle `usb_w` × `usb_h`
   centred at z = `floor_t + cavity / 2`, and two `usb_screw_d` holes `usb_pitch` apart. Extrude *Cut*, distance
   `wall + 1 mm` into the wall.
8. **Front mark.** Tangent plane at 0°: small triangle (3 mm) pointing up, at z ≈ 10. Cut 0.4 mm.
9. **Feet.** Sketch on the bottom face: 3 circles `foot_d` on `foot_r`, 120° apart (e.g. 60°/180°/300°, avoiding the USB
   cutout). Cut 0.5 mm. Skip this if you'll cover the bottom with one large rubber or cork disc instead.
10. **Fillets.** Outside bottom edge R1.5. Floor-to-wall inside edge R1. Post bases R1. No fillet on the shoulder or
    the stub (the top has to seat over them).

### 4.3 Top

1. **Plate.** Sketch on an offset plane at `z_plate`: circle `top_od`. Extrude up `top_t`. New body.
2. **Skirt.** Same plane: circles `skirt_id` and `top_od`. Extrude **down** `skirt_h`, *Join*.
3. **Screw bosses.** Sketch on the `z_plate` plane. *Project* the Board's 3 mounting holes and put a `top_boss_d`
   circle on each centre. Extrude **down** `gap` (to `z_shoulder`), *Join*.
4. **Insert holes.** *Create → Hole* on each screw boss's bottom face: simple, diameter `top_insert_hole_d`, depth
   `top_insert_depth`, flat bottom.
5. **Mic bosses.** Sketch on the `z_plate` plane. *Project* the Board's 8 mic holes and put a `mic_boss_d` circle on
   each centre. Extrude **down** `mic_boss_h`, *Join*.
6. **Pinholes.** Sketch on the top surface, *Project* the 8 mic-hole centres again, circles `pin_print_d`. Extrude
   *Cut*, *All*, through the plate and the mic bosses.
7. **Edges.** Top outside edge: chamfer 1 mm or fillet R1.5. Skirt bottom outside edge: fillet R0.3. Optional:
   a 0.2 mm chamfer on each pinhole's top edge for a crisper look (the drill cleans it up anyway).

### 4.4 Check

1. *Inspect → Interference* with Base, Top, Board selected. Expect none except the 3 screw bosses touching the
   board face. Look closely at H3 vs J1 (post below and boss above), and J2 vs the floor.
2. *Inspect → Section Analysis* through a mic hole and through H1: check the 2.0 mm gap, the 1.2 mm space between
   each mic boss and the board, the pinhole coaxial with the mic hole, the skirt clearing the shoulder by 0.2 mm,
   and the screw tip (`z_shoulder + insert_engage`, 29.8) at least 1.5 mm below the top surface (31.6).
3. *Inspect → Measure* each pinhole centre to its mic-hole centre: < 0.2 mm.
4. Change `cavity` to 18 and to 26 and confirm the model rebuilds without errors and `z_seat` stays in range (§2).

### 4.5 Export

Right-click each body → *Save as Mesh* → 3MF (refinement High). Or *Utilities → 3D Print*, choosing Bambu
Studio as the target. Save the .f3d link and the exported 3MF/STEP into this folder.

## 5. Bambu Studio

**Base** (Matte PETG + Cloudy White PETG): upright, floor on the textured plate, 0.2 mm layers, 3 walls, 15 % gyroid.
- Right-click → *Height range modifier* → z from `z_band - 1.5` (18.0) to `z_shoulder` (26.1), starting below the
  band so the ledge under it prints white instead of black (black absorbs the light). Set its filament to
  Cloudy White PETG and *Wall loops* to 4, so the 1.6 mm band is solid perimeters (even light). Cloudy White is
  milky, so thin the band if the coupon is too dim (see `enclosure-plan.md` §6).
- Keep the prime tower. In *Flushing options*, turn on *Flush into objects' infill*.
- No supports needed: the USB cutout top bridges 12 mm, the band recess has the 45° slope, and the counterbores
  open downward onto the bed.

**Top** (black Matte PETG): upside down, top surface on the textured plate. That face gets the even matte texture
of the plate, and the bosses grow straight up, so no supports.
- 0.2 mm layers, 3 walls, 25 % gyroid, at least 5 bottom shell layers (the visible face and the 1 mm skin over the
  inserts must be solid).
- Check the preview: all 8 pinholes open through the plate and the mic bosses, and the insert holes stop 1 mm short
  of the face.

## 6. Gaskets and assembly

1. **Drill the pinholes.** Pin vise with a 1.0 mm bit, from the top face, straight down. Blow out the chips.
2. **Inserts.** Heat-set the 3 short inserts into the screw bosses from the underside. Low iron temperature, light
   pressure, stop flush with the boss face. Look at the top face for a bump; if there is one, use a lower
   temperature next time (the coupon in §7 is for this).
3. **Cut gaskets.** On the foam's backing, punch the 1.5 mm hole first, then centre the 5 mm punch on it. Cut 16+.
4. **Stick gaskets.** Peel, and press each one onto a mic boss with its hole centred on the pinhole (a 1 mm drill
   bit or a toothpick through the pinhole helps you centre it). No adhesive or foam may cover the hole.
5. **Assemble.** USB cable into J2 from below, board onto the posts (mic 0 to the front mark), top on, then turn the
   stack over holding it together and drive the 3 M3 × 25 screws from below. Snug, not tight: the bosses bottom out
   on the board.
6. **Leak test** (`enclosure-plan.md` §5): tape over one pinhole at a time; that mic's level should drop a lot
   while the others barely change.

## 7. Test prints before the full parts (each < 30 min)

| Coupon | How | What to learn |
|---|---|---|
| Top boss | 25 mm disc of the top (`top_t`) with one screw boss + insert hole | Insert goes in straight; no bump or crack on the top face |
| Gasket | 25 mm disc of the top with one mic boss and pinhole; drill it, stick a gasket, press it onto glass with 2.0 mm spacers | Gasket squeeze (should look ~25 %, sealed all round), drilled hole quality, gasket centring |
| Fit ring | Top 6 mm of the base (shoulder + stub) and the top's rim with its skirt, as rings | `fit_clr`: the skirt drops over the stub without binding |
| H3 post | One H3 post (`post_d_h3`) with the counterbored screw hole, on a 20 mm square | Screw passes freely; head seats in the counterbore |
| Band | 60° wall arc from `z_band - 3` to `z_shoulder`, made 2–3 ways in Cloudy White PETG (1.2 mm single skin, 0.8 + gap + 1.2 mm double skin, inner flutes, fuzzy skin) | Blending between 3 blue LEDs 27 mm apart, held 5.5 / 9 / 12 mm behind it |
