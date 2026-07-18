// Forager enclosure — 2-part (front bezel + rear tray), screwed together.
//
// PORTRAIT LAYOUT: the Waveshare display PCB is physically landscape
// (93.5 x 78.5mm, wider than tall) but the software renders its UI rotated
// 90 deg into a portrait 300x400 logical canvas (epd.setRotation(3), see
// CLAUDE.md's "Display" section). This enclosure mounts the board rotated
// 90 deg so the case itself is genuinely portrait (tall/narrow) to match
// what's actually on screen — the board's long 93.5mm edge runs along the
// case's Y (height) axis, its short 78.5mm edge runs along the case's X
// (width) axis. See the "Display module" and "onboard buttons" sections
// below for the full derivation, including exactly which board edge
// (KEY0/KEY1 vs. the FPC connector) ends up on which new case edge.
//
// Hardware fit (measured / vendor-spec):
//   - Display: Waveshare 4.2" e-paper module (GDEY042T81), PCB 93.5 x 78.5mm,
//     active area 84.8 x 63.6mm, mounted rotated 90 deg (see above). Board +
//     FPC bump thickness measured at 12.0mm (was a 4.5mm placeholder before).
//   - MCU: ESP32-S3 Super Mini, 22.52 x 18mm.
//   - Battery: 2000mAh LiPo pouch, 60 x 36 x 7mm (7mm thickness measured/
//     confirmed).
//   - Buttons: 3x standard 6x6mm tactile switches, mounted in the front bezel.
//   - Slide switch: generic small SPST slide switch (assumed footprint below —
//     confirm/adjust SWITCH_* before printing).
//
// ASSUMPTIONS flagged inline with "ASSUMPTION:" — verify against real parts
// before printing, especially the display's native top/bottom margin split
// (6.0 / 8.9mm, never confirmed against a real board photo — now resolved
// conservatively into a single symmetric left/right margin, see below) and
// the slide switch footprint (no spec was given for that one).
//
// Render one part at a time:
//   openscad -D part=\"bezel\" -o stl/front_bezel.stl forager_enclosure.scad
//   openscad -D part=\"tray\"  -o stl/rear_tray.stl   forager_enclosure.scad
//   openscad -D part=\"both\"  -o preview/assembled.png forager_enclosure.scad
part = "both"; // "bezel" | "tray" | "both" (both = preview only, side by side)

$fn = 48;

// ---- General ----
wall      = 2.2;   // side-wall thickness
clearance = 0.3;   // general fit clearance around boards

// ---- Display module (Waveshare 4.2", GDEY042T81) ----
// ROTATED 90 deg from the panel's native landscape PCB layout so the case
// reads portrait (see top-of-file note). disp_w/disp_h/win_w/win_h below are
// CASE-axis dimensions (X = width, Y = height), not the vendor's native
// width/height labels — every dimension here is the native spec re-derived
// onto the new axes, not just relabeled:
//   native disp_w (93.5, long PCB edge)  -> now case-Y (disp_h below)
//   native disp_h (78.5, short PCB edge) -> now case-X (disp_w below)
//   native win_w  (84.8, active long axis)  -> now case-Y (win_h below)
//   native win_h  (63.6, active short axis) -> now case-X (win_w below)
disp_w = 78.5;  // = native disp_h (short PCB edge), now the case width
disp_h = 93.5;  // = native disp_w (long PCB edge), now the case height
disp_t = 12.0;  // CONFIRMED (measured): board + FPC connector bump thickness
disp_wire_clearance = 3.0; // JUDGMENT CALL, not a spec: extra depth behind
                            // the board (beyond disp_t + the general fit
                            // `clearance`) so the FPC flex cable and its
                            // connector have room to bend/route without
                            // being pinched against the pocket floor. Revisit
                            // if the real cable geometry needs more/less —
                            // this is padding, not a measurement.
win_h  = 84.8;  // = native win_w (active long axis), now vertical

// Native top/bottom margins on the short axis (6.0 / 8.9mm — ASSUMPTION,
// ~never confirmed against a real board photo) become this case's LEFT/
// RIGHT margins after the rotation. The user wants minimal bezels but a
// MATCHING left/right, and the two true margins are unequal, so: use one
// symmetric margin equal to the LARGER of the two assumed values (8.9mm).
// The larger value is the safe direction — applied to both sides it can
// only make the cut window slightly *smaller* than the true active area
// (a sliver of inactive board border masked at the edges), never bigger,
// so it can never expose past the true active-area edge on the side where
// the real margin is actually only 6.0mm. Still flagged ASSUMPTION since
// 6.0/8.9 themselves were never confirmed — just resolved conservatively.
win_margin_lr = 8.9; // ASSUMPTION: max(6.0, 8.9) native top/bottom margins,
                      // reused symmetrically as the case's left/right margin
win_w = disp_w - 2 * win_margin_lr; // 60.7 — derived, intentionally a bit
                                     // less than the native 63.6mm active
                                     // width, because the safe symmetric
                                     // margin above eats slightly more than
                                     // the true (asymmetric) native total of
                                     // 14.9mm. Minimal-but-safe, not padded
                                     // further than the larger margin requires.
win_margin_top = (disp_h - win_h) / 2; // 4.35, exact — this pairing (native
win_margin_bottom = win_margin_top;    // win_margin_lr) was already symmetric
                                        // before rotation, so no assumption here

// ---- MCU: ESP32-S3 Super Mini ----
mcu_w = 22.52;
mcu_l = 18.0;
mcu_component_h = 4.0; // clearance for USB-C connector / header stubs

// ---- Battery: 2000mAh LiPo, 60 x 36 x 7mm ----
batt_w = 60.0;
batt_h = 36.0;
batt_t = 7.0;   // CONFIRMED (measured, 0.7cm) — matches the prior placeholder
batt_puff_clearance = 2.0; // JUDGMENT CALL, not a spec: LiPo pouches can run
                            // a bit thicker than nominal (puffiness over the
                            // battery's life) and there's usually a layer of
                            // tape/padding around it, so pad the tray's
                            // interior depth a couple mm beyond batt_t on top
                            // of the general fit `clearance` — otherwise a
                            // slightly-fat pack could bind against the front
                            // bezel when the case is closed.

// ---- Buttons: 3x 6x6mm tactile, in a row below the display window ----
btn_body      = 6.0;
btn_hole      = 6.3;  // counterbore the switch body presses into, from behind
btn_counterbore_depth = 1.4;
btn_cap_hole  = 3.6;  // through-hole the actuator cap pokes through
btn_pitch     = 16.0;
btn_row_gap   = 8.0;  // gap from window bottom edge to button hole centers

// ---- Display module's own 2 onboard buttons (now the case's TOP edge) ----
// Confirmed against vendor dimensioned photo, in the board's NATIVE
// (pre-rotation) orientation: KEY0/KEY1 sit on the board's native RIGHT
// edge, in the narrow native win_margin_lr strip (4.35mm) between that edge
// and the active window. The FPC connector is on the board's native LEFT
// edge (opposite side), per README/CLAUDE.md.
//
// Rotation direction: KEY1 must land in the case's TOP-LEFT corner (fixed
// constraint). Tracking the native right-edge corners through each of the
// two "rotate in place" options shows neither gets there on its own — a
// plain 90 deg CW rotation lands KEY1 bottom-left, a plain CCW rotation
// lands it top-right. Landing exactly top-left additionally requires
// picking this case's own left/right handedness (nothing external pins
// "case left" to a physical side — this shell is being authored from
// scratch), i.e. mirroring the whole case layout left-right to match. That
// mirrors the CASE we're drawing, not the board itself: the board still
// sits face-up, un-flipped, in its pocket, same as before. Net transform,
// applied consistently to every point on the board (window, pocket, both
// buttons): new_x = native_y, new_y = native_x. Under that transform the
// native RIGHT edge (KEY0/KEY1) becomes the case's TOP edge, and the native
// LEFT edge (FPC) becomes the case's BOTTOM edge.
//
// KEY1 (native 16.03mm from the board's bottom corner) lands at new_x =
// 16.03 — near the case's top-left corner, as required. KEY0 (native
// 33.70mm, 17.67mm further along the same edge) lands at new_x = 33.70,
// further right along that same top edge but still left-of-center (case
// width is 78.5mm). Both sit within the old 4.35mm margin strip, which
// after rotation is exactly win_margin_top (see above) — holes are
// centered in that strip, at case-Y = pocket top minus half of
// win_margin_top. disp_btn_d (hole diameter) is still a PLACEHOLDER — no
// switch body/cap size was given, confirm before printing. The holes are
// cut from the front face straight onto the board's surface, same as the
// main buttons.
//
// The FPC connector (native left edge -> now the pocket's BOTTOM edge) is
// not modeled as a cutout (no exact position was given), but keep it in
// mind: it needs open clearance for the flex cable to bend, not a wall
// flush against that edge. Conveniently, the button-strip area directly
// below the pocket already has no back layer over it (see the front face
// comment in front_bezel() below), so there's an open cavity right there
// for the FPC to route into.
disp_btn_d  = 3.0;   // PLACEHOLDER: switch body/cap diameter not measured
disp_btn_x1 = 16.03; // KEY1 — nearer the case's top-left corner
disp_btn_x2 = 33.70; // KEY0 — 17.67mm further right along the same top edge

// ---- USB-C access (in tray side wall, aligned to MCU edge) ----
usbc_slot_w = 10.0;
usbc_slot_h = 4.0;

// ---- Slide switch (ASSUMPTION — no spec given, adjust before printing) ----
sw_slot_w = 12.0;
sw_slot_h = 4.0;

// ---- Screw bosses (M2.5 self-tapping, 4 corners) ----
boss_od      = 6.0;
boss_pilot_d = 2.0;   // pilot hole for self-tap thread
boss_clear_d = 2.7;   // clearance hole through the bezel for the screw shaft
boss_inset   = 6.0;   // inset from outer corner, both axes
countersink_depth = 1.2; // recess for a flat/pan screw head at the bezel front

// ---- Derived footprint ----
pocket_w = disp_w + 2 * clearance;
pocket_h = disp_h + 2 * clearance;

// outer_w/outer_h: case is now portrait (tall/narrow) — outer_w is derived
// from the rotated (short-axis) disp_w=78.5, outer_h from the rotated
// (long-axis) disp_h=93.5 plus the button strip, so outer_h ends up well
// over outer_w, unlike the old landscape-ish footprint.
outer_w = pocket_w + 2 * wall;                 // 83.5
button_area_h = 20.0;                          // room below window for button row
outer_h = pocket_h + wall + button_area_h + wall; // 118.5

bezel_front_t = 2.4;   // front face thickness (button cap holes live here)
bezel_lip     = 1.5;   // inward lip that catches the display's front edge
bezel_pocket_depth = disp_t + clearance + disp_wire_clearance; // 15.3 — board
                                                                // + FPC bump
                                                                // + wire-route
                                                                // padding
bezel_total_t = bezel_front_t + bezel_pocket_depth; // 17.7

tray_interior_depth = max(batt_t + batt_puff_clearance, mcu_component_h) + clearance; // 9.3
tray_floor_t = 2.2;
tray_wall_h  = tray_interior_depth + tray_floor_t;

module screw_bosses(hole_d, extra_h = 0) {
    positions = [
        [boss_inset, boss_inset],
        [outer_w - boss_inset, boss_inset],
        [boss_inset, outer_h - boss_inset],
        [outer_w - boss_inset, outer_h - boss_inset],
    ];
    for (p = positions)
        translate([p[0], p[1], 0]) {
            difference() {
                cylinder(d = boss_od, h = extra_h);
                translate([0, 0, -0.1]) cylinder(d = hole_d, h = extra_h + 0.2);
            }
        }
}

module front_bezel() {
    win_x = wall + win_margin_lr;
    win_y = wall + button_area_h + win_margin_bottom; // measured from bottom
    pocket_x = wall;
    pocket_y = wall + button_area_h;

    btn_row_y = pocket_y - btn_row_gap; // button hole centers, below the pocket
    btn_start_x = outer_w / 2 - btn_pitch;

    difference() {
        union() {
            // Front face slab (this is the only material over the button
            // strip — no back layer there, so switch bodies have clearance
            // to sit behind it in the tray cavity)
            cube([outer_w, outer_h, bezel_front_t]);
            // Raised lip + walls forming the pocket for the display board,
            // confined to the display area only (not the full footprint)
            translate([pocket_x, pocket_y, bezel_front_t])
                difference() {
                    cube([pocket_w, pocket_h, bezel_pocket_depth]);
                    translate([bezel_lip, bezel_lip, -0.1])
                        cube([
                            pocket_w - 2 * bezel_lip,
                            pocket_h - 2 * bezel_lip,
                            bezel_pocket_depth + 0.2
                        ]);
                }
            // No raised bosses here — the bezel just needs clean clearance
            // holes (cut below). Threaded bosses live on the tray side only,
            // otherwise the bottom two corners (outside the pocket-wall
            // footprint) would poke proud through the front face.
        }

        // Display window (full depth)
        translate([win_x, win_y, -0.1])
            cube([win_w, win_h, bezel_front_t + bezel_pocket_depth + 0.2]);

        // Display pocket recess behind the lip (board sits here)
        translate([pocket_x, pocket_y, bezel_front_t - 0.01])
            cube([pocket_w, pocket_h, bezel_pocket_depth + 0.2]);

        // Button holes: straight bore sized to the switch body, full front-face
        // depth. Switches press in from behind (tray side) until their flange
        // seats against the inside face; cap pokes out the front.
        for (i = [0:2]) {
            bx = btn_start_x + i * btn_pitch;
            translate([bx, btn_row_y, -0.1])
                cylinder(d = btn_hole, h = bezel_front_t + 0.2);
        }

        // Display's onboard buttons (now the pocket's TOP edge, in the
        // win_margin_top strip after rotation) — see disp_btn_* + the
        // rotation-direction comment above for the full derivation
        disp_btn_y = pocket_y + pocket_h - win_margin_top / 2;
        for (f = [disp_btn_x1, disp_btn_x2])
            translate([pocket_x + f, disp_btn_y, -0.1])
                cylinder(d = disp_btn_d, h = bezel_front_t + 0.2);

        // Corner screw clearance holes, full depth, with a countersink at the
        // front face so a flat/pan screw head sits recessed
        screw_positions = [
            [boss_inset, boss_inset],
            [outer_w - boss_inset, boss_inset],
            [boss_inset, outer_h - boss_inset],
            [outer_w - boss_inset, outer_h - boss_inset],
        ];
        for (p = screw_positions) {
            translate([p[0], p[1], -0.1])
                cylinder(d = boss_clear_d, h = bezel_total_t + 0.2);
            translate([p[0], p[1], -0.1])
                cylinder(d1 = boss_od - 1, d2 = boss_clear_d, h = countersink_depth + 0.1);
        }
    }
}

module rear_tray() {
    difference() {
        union() {
            // Floor
            cube([outer_w, outer_h, tray_floor_t]);
            // Perimeter wall
            translate([0, 0, tray_floor_t])
                difference() {
                    cube([outer_w, outer_h, tray_interior_depth]);
                    translate([wall, wall, -0.1])
                        cube([
                            outer_w - 2 * wall,
                            outer_h - 2 * wall,
                            tray_interior_depth + 0.2
                        ]);
                }
            screw_bosses(boss_pilot_d, tray_wall_h);
        }

        // USB-C access slot, centered on the top wall (MCU sits near here)
        translate([outer_w / 2 - usbc_slot_w / 2, outer_h - wall - 0.1, tray_floor_t + tray_interior_depth / 2 - usbc_slot_h / 2])
            cube([usbc_slot_w, wall + 0.2, usbc_slot_h]);

        // Slide switch access slot, centered on the left wall
        // ASSUMPTION: generic footprint, confirm before printing
        translate([-0.1, outer_h / 2 - sw_slot_w / 2, tray_floor_t + tray_interior_depth / 2 - sw_slot_h / 2])
            cube([wall + 0.2, sw_slot_w, sw_slot_h]);
    }

    // MCU standoffs (near the USB-C wall, board's short edge toward that wall)
    mcu_x = outer_w / 2 - mcu_w / 2;
    mcu_y = outer_h - wall - mcu_l - 4;
    for (p = [[0,0],[mcu_w,0],[0,mcu_l],[mcu_w,mcu_l]])
        translate([mcu_x + p[0], mcu_y + p[1], tray_floor_t])
            cylinder(d = 3.5, h = 1.5);

    // Battery bay walls (shallow retaining lip, battery held by friction/tape)
    batt_x = wall + 3;
    batt_y = wall + 3;
    translate([batt_x - 0.5, batt_y - 0.5, tray_floor_t]) {
        difference() {
            cube([batt_w + 1, batt_h + 1, 1.5]);
            translate([0.5, 0.5, -0.1]) cube([batt_w, batt_h, 2]);
        }
    }
}

if (part == "bezel") {
    front_bezel();
} else if (part == "tray") {
    rear_tray();
} else {
    front_bezel();
    translate([outer_w + 20, 0, 0]) rear_tray();
}
