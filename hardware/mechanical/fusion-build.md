# Fusion build: base and cap

Companion to `enclosure-plan.md`. Decided 2026-10-26: **grille cloth wrapped over a printed perforated plate.**
All sizes are Fusion user parameters, so the cavity depth can change once the USB plug is measured.

## 1. How the pieces meet (section through the wall)

Heights (z = 0 at the underside of the floor, before feet):

| z (mm) | Level |
|---|---|
| 30.7 | Cap plate top (fabric on top of this) |
| 29.1 | Cap plate underside = stub top |
| 26.7 | Cap skirt bottom edge (fabric folded under it) |
| 26.1 | Board top = **shoulder** = visible seam |
| 24.5 | Board bottom = post tops; LEDs hang just below |
| 19.5 | Translucent band bottom (band runs 19.5 → 26.1) |
| 2.5 | Floor top |

Radii:

| r (mm) | Edge |
|---|---|
| 70.0 | Board edge |
| 70.4 | Wall / stub inside face |
| 71.9 | Stub outside face |
| 72.4 | Band inside face (recessed, band 1.6 thick) = skirt inside face |
| 73.6 | Skirt outside face (+ 0.4 fabric = 74.0) |
| 74.0 | Base outside face |

- The board sits on 3 posts. Its top face is the **shoulder** where the base wall steps in to a thin **stub**.
- The cap's plate rests on the top of the stub. Its skirt slides down over the stub to just above the shoulder.
  The fabric folded inside the skirt makes the friction fit.
- From outside you see: fabric top and skirt, a thin seam, the light band, then the opaque base.

## 2. Parameters

*Modify → Change Parameters → +*. Enter them in this order (later ones use earlier ones).

| Name | Expression | Value | Meaning |
|---|---|---|---|
| `board_d` | `140 mm` | 140 | PCB diameter |
| `board_t` | `1.6 mm` | 1.6 | PCB thickness |
| `side_clr` | `0.4 mm` | 0.4 | Radial gap, board edge to wall |
| `wall` | `3.6 mm` | 3.6 | Base wall (thick enough to leave a 1.5 mm stub, see `stub_od`) |
| `inner_d` | `board_d + 2 * side_clr` | 140.8 | Wall inside diameter |
| `outer_d` | `inner_d + 2 * wall` | 148.0 | Outside diameter |
| `floor_t` | `2.5 mm` | 2.5 | Floor thickness |
| `cavity` | `22 mm` | 22 | Floor top to board bottom. **Set from the measured USB plug** |
| `gap` | `3 mm` | 3.0 | Board top to cap plate underside (decided 2.5–3) |
| `z_board` | `floor_t + cavity` | 24.5 | Board bottom = post tops |
| `z_shoulder` | `z_board + board_t` | 26.1 | Board top = shoulder = seam |
| `z_plate` | `z_shoulder + gap` | 29.1 | Cap plate underside = stub top |
| `plate_t` | `1.6 mm` | 1.6 | Cap plate thickness (8 layers at 0.2) |
| `fabric_t` | `0.4 mm` | 0.4 | Grille cloth thickness. **Measure with calipers, lightly squeezed** |
| `fit_clr` | `0.1 mm` | 0.1 | Extra radial clearance, skirt to stub. Tune on the test ring |
| `skirt_t` | `1.2 mm` | 1.2 | Cap skirt thickness |
| `cap_od` | `outer_d - 2 * fabric_t` | 147.2 | Cap OD, so fabric ends flush with the base |
| `skirt_id` | `cap_od - 2 * skirt_t` | 144.8 | |
| `stub_od` | `skirt_id - 2 * (fabric_t + fit_clr)` | 143.8 | Stub is (143.8 − 140.8)/2 = 1.5 mm thick |
| `skirt_h` | `gap - fabric_t - 0.2 mm` | 2.4 | Skirt length below the plate; leaves room for fabric folded under its edge |
| `rim_w` | `3.5 mm` | 3.5 | Solid rim of the cap plate; must cover the stub top (> 3.2) |
| `mesh_d` | `cap_od - 2 * rim_w` | 140.2 | Diameter of the open mesh area |
| `band_h` | `5 mm` | 5.0 | Band height below the board bottom |
| `band_t` | `1.6 mm` | 1.6 | Band thickness (4 perimeters) |
| `z_band` | `z_board - band_h` | 19.5 | Band bottom |
| `post_r` | `41.5 mm` | 41.5 | Mounting-hole circle radius |
| `post_d` | `7 mm` | 7.0 | Posts at H1, H2 |
| `post_d_h3` | `6.5 mm` | 6.5 | Post at H3 (J1 is 3.45 mm away) |
| `post_wide_d` | `10 mm` | 10 | Lower, wider part of each post |
| `post_narrow_h` | `10 mm` | 10 | Narrow length at the post top (clears J1's 8.5 mm header) |
| `insert_hole_d` | `4.0 mm` | 4.0 | Heat-set insert hole (check the insert maker's drawing) |
| `insert_depth` | `6.5 mm` | 6.5 | Insert length + 0.8 mm |
| `usb_w` | `12 mm` | 12 | Panel cutout width. **Placeholder until the cable arrives** |
| `usb_h` | `7 mm` | 7 | Panel cutout height. Placeholder |
| `usb_pitch` | `17 mm` | 17 | Panel screw spacing (from the listing) |
| `usb_screw_d` | `3.2 mm` | 3.2 | Panel screw hole. Placeholder |
| `foot_d` | `8.5 mm` | 8.5 | Recess for stick-on feet |
| `foot_r` | `60 mm` | 60 | Feet circle radius |

## 3. Angles after the flip

`firmware/final/README.md` assumes the board is flipped **left-to-right (MK1 stays at the top)**. Seen from above,
a KiCad angle θ then sits at 180° − θ:

| | KiCad view | Seen from above (enclosure) |
|---|---|---|
| H1 / H2 / H3 | 54° / 174° / 294° | **126° / 6° / 246°** |
| J2 USB-C | 24° | 156° (near the back, good for the cable) |
| J1 SWD | 312° | 228° |
| Mic 0 = **front** | MK7 at 180° | **0° (+x)** |

So: front mark at 0°, USB panel port at 180°.

## 4. Build steps

Fusion with Z up (*Preferences → General → Default modeling orientation: Z up*). Make two components, **Base** and
**Cap**, plus the imported **Board**. Activate the right component before each step.

### 4.1 Import the board (for checking only)

1. In KiCad, add the USB4145 STEP to J2 first, then *File → Export → STEP* (board only, include 3D models).
2. Fusion: *Insert → Insert Mesh/Upload*, or drag the .step in and insert it into the design as a component named Board.
3. Rotate it 180° about the axis through MK1 and MK5 (the left-to-right flip). Check: MK7 lands on +x, H2 at ~6°.
4. Move it so the board centre is on the Z axis and its **bottom face** (the component side, now facing down) is
   at `z_board`. Right-click → *Ground*.

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
5. **Insert holes.** *Create → Hole* on each post top: simple, diameter `insert_hole_d`, depth `insert_depth`,
   flat bottom.
6. **Band recess (inside face).** Sketch on the XZ plane. Draw the profile to remove:
   from r = `inner_d / 2` to r = `outer_d / 2 - band_t`, from z = `z_band` up to `z_shoulder - 2 mm`, with a
   45° slope at the top meeting the wall at `z_shoulder`. The slope stops the stub hanging over a flat ledge
   when printing. *Revolve*, axis Z, 360°, *Cut*.
7. **USB panel cutout.** *Construct → Tangent plane* on the outer wall at 180°. Sketch: rectangle `usb_w` × `usb_h`
   centred at z = `floor_t + cavity / 2`, and two `usb_screw_d` holes `usb_pitch` apart. Extrude *Cut*, distance
   `wall + 1 mm` into the wall.
8. **Fingernail notch.** On the same tangent plane: rectangle 10 × 1.5 mm with its top edge at `z_shoulder`.
   Cut 1 mm deep. This lets you lift the cap off.
9. **Front mark.** Tangent plane at 0°: small triangle (3 mm) pointing up, at z ≈ 10. Cut 0.4 mm.
10. **Feet.** Sketch on the bottom face: 3 circles `foot_d` on `foot_r`, 120° apart (e.g. 60°/180°/300°, avoiding the USB
    cutout). Cut 0.5 mm.
11. **Fillets.** Outside bottom edge R1.5. Floor-to-wall inside edge R1. Post bases R1. No fillet on the shoulder or
    the stub (the cap has to seat on them).

### 4.3 Cap

1. **Plate.** Sketch on an offset plane at `z_plate`: circle `cap_od`. Extrude up `plate_t`. New body.
2. **Skirt.** Same plane: circles `skirt_id` and `cap_od`. Extrude **down** `skirt_h`, *Join*.
3. **Edges.** Top outside edge: chamfer 1.5 mm (prints cleanly face-down, and fabric wraps it well). Skirt bottom
   outside edge: fillet R0.5 so it can't cut the cloth.
4. **Mesh.** Leave the plate solid in Fusion. The slicer opens it up (§5). If you'd rather model it: sketch a
   hexagon 4 mm across flats, rectangular-pattern it at 5.2 mm pitch (≈ 59 % open), keep only the ones inside
   `mesh_d`, and cut through `plate_t`. It's slow in Fusion, which is why the slicer route is the default.

### 4.4 Check

1. *Inspect → Interference* with Base, Cap, Board selected. Expect none. Look closely at H3 vs J1, and J2 vs the floor.
2. *Inspect → Section Analysis* on the XZ plane: check the 3 mm gap, the cap sitting on the stub, and the skirt
   clearing the shoulder.
3. Change `cavity` to 18 and to 26 and confirm the model rebuilds without errors.

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
- No supports needed: the USB cutout top bridges 12 mm, and the band recess has the 45° slope.

**Cap** (black Matte PETG): upside down, plate top on the plate.
- *Add modifier → Cylinder*: diameter `mesh_d` (140.2), height `plate_t` (1.6), centred, sitting on the bed.
- In the modifier: *Wall loops* 0, *Top shell layers* 0, *Bottom shell layers* 0, *Sparse infill* Grid 25 %.
  That turns the middle of the plate into an open grid (about 3 × 3 mm holes, ~75 % open), anchored by the solid rim.
- Check the preview: the grid should run edge to edge with no solid top layer.

## 6. Wrapping the fabric

1. Cut a fabric disc ~175 mm across (cap + skirt + 10 mm to tuck).
2. Lay it face-down, put the cap on it top-down, centred.
3. Mask the mesh area with paper. Spray adhesive (e.g. 3M Super 77) only on the skirt's outside and inside faces.
   Glue on the mesh would block the holes.
4. Pull the fabric up over the skirt edge and press it to the inside face: 12 and 6 o'clock, then 3 and 9, then
   the gaps between, ~16 points. Smooth out wrinkles as you go.
5. Trim to ~3 mm inside the skirt and lock the edge with a thin line of hot glue or CA.
6. Push the cap onto the base. Too tight: sand the stub lightly or raise `fit_clr` by 0.05. Too loose: lower it.

## 7. Test prints before the full base (each < 30 min)

| Coupon | How | What to learn |
|---|---|---|
| Post | One H3 post (`post_d_h3`) on a 20 mm square | Insert goes in straight without splitting |
| Fit ring | Top 8 mm of the base (shoulder + stub) and the cap rim with a 10 mm skirt, as rings, plus a fabric strip | `fit_clr` and `fabric_t` |
| Band | 60° wall arc from `z_band - 3` to `z_shoulder`, made 2–3 ways in Cloudy White PETG (1.2 mm single skin, 0.8 + gap + 1.2 mm double skin, inner flutes, fuzzy skin) | Blending between 3 blue LEDs 27 mm apart, held 5.5 / 9 / 12 mm behind it |
| Grille | 40 mm square of the cap plate with the grid modifier, fabric wrapped | Stiffness and look |
