// Forager enclosure — 2-part (front bezel + rear tray), screwed together.
//
// PORTRAIT LAYOUT: the Waveshare display PCB is physically landscape
// (93.5 x 78.5mm) but the UI renders rotated 90 deg into a portrait
// 300x400 canvas (epd.setRotation(3), see CLAUDE.md's "Display" section).
// This enclosure mounts the board rotated 90 deg to match: the board's
// long 93.5mm edge runs along the case's Y (height) axis, short 78.5mm
// edge along X (width).
//
// Hardware fit (measured / vendor-spec):
//   - Display: Waveshare 4.2" e-paper (GDEY042T81), PCB 93.5x78.5mm, active
//     area 84.8x63.6mm. Board + FPC bump thickness measured at 12.0mm.
//   - MCU: ESP32-S3 Super Mini, 22.52 x 18mm.
//   - Battery: 2000mAh LiPo pouch, 60 x 36 x 7mm (measured).
//   - Buttons: 3x 6x6mm tactile switches, in the front bezel.
//   - Slide switch: generic small SPST (ASSUMPTION, no spec given).
//
// ASSUMPTIONS flagged inline — verify against real parts before printing.
// Several dimensions below were corrected against real-hardware photos
// after a first print (marked "real-hardware photo" / "ASSUMPTION").
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
// Rotated 90 deg from native landscape PCB layout; disp_w/disp_h/win_w/
// win_h are CASE-axis (X=width, Y=height), not the vendor's native labels:
//   native disp_w (93.5, long edge)  -> case-Y (disp_h)
//   native disp_h (78.5, short edge) -> case-X (disp_w)
//   native win_w  (84.8, active long axis)  -> case-Y (win_h)
//   native win_h  (63.6, active short axis) -> case-X (win_w)
disp_w = 78.5;
disp_h = 93.5;
disp_t = 12.0;  // measured: board + FPC connector bump
disp_wire_clearance = 3.0; // padding behind the board for the FPC cable to
                            // bend/route without pinching

win_h  = 90.0;  // Was 84.8 (vendor spec) -- real print showed content
                 // clipped at the top edge; bumped, re-check after reprint.
win_margin_top = (disp_h - win_h) / 2;
win_margin_bottom = win_margin_top;

// Real measurement from the board (not vendor spec -- couldn't confirm
// that online): ~8mm margin on one side, ~0mm on the other. The bezel's
// case-X axis runs opposite the physical left/right the measurement was
// taken against, so the wide margin goes on case-X's low side here even
// though it's the physically-right-hand margin -- swapped from the first
// attempt, which had it backwards. Small 0.3mm safety sliver instead of a
// true 0, just enough to keep the window cut from running flush into the
// pocket cut.
win_margin_left  = 0.3;
win_margin_right = 8.0;
win_w = disp_w - win_margin_left - win_margin_right;

// ---- MCU: ESP32-S3 Super Mini ----
mcu_w = 22.52;
mcu_l = 18.0;
mcu_component_h = 4.0; // clearance for USB-C connector / header stubs
mcu_standoff_h = 0.8; // Was 1.5 -- board still sat too high, lowered

// ---- Battery: 2000mAh LiPo, 60 x 36 x 7mm ----
batt_w = 60.0;
batt_h = 36.0;
batt_t = 7.0;   // measured
batt_puff_clearance = 2.0; // LiPo pouches puff up over time; extra depth
                            // margin beyond batt_t

// ---- Buttons: 3x 6x6mm tactile, in a row below the display window ----
btn_body      = 6.0;
btn_hole      = 6.3;  // counterbore the switch body presses into, from behind
btn_counterbore_depth = 1.4;
btn_cap_hole  = 3.6;  // through-hole the actuator cap pokes through
btn_pitch     = 16.0;
btn_row_gap   = 8.0;  // gap from window bottom edge to button hole centers
btn_center_offset = (win_margin_left - win_margin_right) / 2; // shifts the
                          // button row so it's centered under the WINDOW
                          // (which isn't centered under outer_w/2, given
                          // the asymmetric win_margin_left/right) rather
                          // than under the whole case

// ---- Display module's onboard buttons (now the case's TOP edge) ----
// KEY0/KEY1 sit on the board's native RIGHT edge, FPC on the native LEFT
// edge. After rotation KEY0/KEY1 land on the case's TOP edge, FPC on the
// BOTTOM. Only KEY1 (wired as PIN_BTN_SETTINGS) gets a cutout.
disp_btn_w = 6.0;  // KEY1's real actuator is a rectangle, oriented the
disp_btn_h = 3.5;  // other way from the first guess, and a bit smaller --
                    // rotated 90deg (w/h swapped) and shrunk, real-hardware
                    // photo/fit-check.
disp_btn_x1 = 16.03; // KEY1 -- has a cutout
disp_btn_x2 = 33.70; // KEY0 -- reference only, no cutout

top_rim = 4.5;  // Was 6.0 -- shrunk further for a flush top edge. Floor is
                 // the side-entry screw flange (see below), which needs
                 // this whole ~4.5mm band above the pocket to fit its boss --
                 // can't go much thinner without shrinking that boss too.

// ---- USB-C access (in tray side wall, aligned to MCU edge) ----
usbc_slot_w = 10.0;
usbc_slot_h = 5.0; // extra vertical tolerance since the slot is now
                    // anchored to the MCU's mounting height (see
                    // usbc_slot_z below), not a floating guess

// ---- Slide switch (ASSUMPTION — no spec given, adjust before printing) ----
sw_slot_w = 12.0;
sw_slot_h = 5.0;  // Was 3.0 -- real switch didn't fit through, widened
sw_body_w  = 9.0;  // Was 7.0 -- widened to match the wider slot + margin
sw_body_l  = 13.0;
sw_pocket_h = 6.5;  // Was 4.5 -- still too shallow to contain the switch,
                     // deepened

// ---- Screw bosses (M2.5 self-tapping, bottom corners) ----
boss_od      = 6.0;
boss_pilot_d = 2.0;   // pilot hole for self-tap thread
boss_clear_d = 2.7;   // clearance hole through the bezel for the screw shaft
boss_inset   = 7.0;   // inset from outer corner, both axes (clear of corner_r)
countersink_depth = 1.2; // recess for a flat/pan screw head at the bezel front

corner_r = 4.0; // rounded exterior corners

// ---- Derived footprint ----
pocket_w = disp_w + 2 * clearance;
pocket_h = disp_h + 2 * clearance;

outer_w = pocket_w + 2 * wall;                    // 83.5
button_area_h = 19.0; // Was 16.0 -- too tight, given more room back
outer_h = pocket_h + wall + button_area_h + top_rim;

bezel_front_t = 2.4;   // front face thickness (button cap holes live here)
bezel_pocket_depth = disp_t + clearance + disp_wire_clearance;
bezel_total_t = bezel_front_t + bezel_pocket_depth;

disp_tray_protrusion = 13.0; // ASSUMPTION, from a real-hardware photo: the
                              // display's pin-header/FPC assembly sticks
                              // well into the TRAY cavity, not just the
                              // bezel-side pocket. Re-check after reprint.
tray_interior_depth = max(batt_t + batt_puff_clearance, mcu_component_h, disp_tray_protrusion) + clearance;
tray_floor_t = 2.2;
tray_wall_h  = tray_interior_depth + tray_floor_t;

// ---- Top side-entry screws (replace the snap-fit, which didn't hold) ----
// The snap-fit post/hook didn't work in practice. Switched to 2 horizontal
// screws through the tray's LEFT/RIGHT walls, biting into small flanges
// added to the bezel's side edges (front_bezel()). Confined to the X range
// already inside `wall` -> can't collide with the window/pocket cuts
// regardless of Y, so these can sit right at the top corners without
// touching the display pocket at all (unlike the old top-center screw).
side_boss_od      = 3.5;
side_pilot_d      = 2.0;
side_clear_d      = 2.7;
side_flange_depth = 8.0; // how far the bezel's flange reaches into the tray cavity
side_y = outer_h - top_rim / 2; // vertical center, in the top rim band
side_boss_z_bezel = bezel_front_t + side_flange_depth / 2; // bezel-local Z
side_boss_z_tray  = tray_wall_h - side_flange_depth / 2;   // tray-local Z
                                                             // (see front_bezel()/
                                                             // rear_tray() for
                                                             // why these differ)

// Rounded-rectangle outline (2D) for the case's exterior footprint.
module rounded_rect(w, h, r) {
    hull() {
        translate([r, r]) circle(r = r);
        translate([w - r, r]) circle(r = r);
        translate([r, h - r]) circle(r = r);
        translate([w - r, h - r]) circle(r = r);
    }
}

// Shared by screw_bosses() (tray) and front_bezel()'s clearance holes, so
// the two parts' screws can't drift out of alignment. Bottom corners only
// -- the top boss was replaced by the snap-fit fastener above.
function boss_positions() = [
    [boss_inset, boss_inset],
    [outer_w - boss_inset, boss_inset],
];

module screw_bosses(hole_d, extra_h = 0) {
    for (p = boss_positions())
        translate([p[0], p[1], 0]) {
            difference() {
                cylinder(d = boss_od, h = extra_h);
                translate([0, 0, -0.1]) cylinder(d = hole_d, h = extra_h + 0.2);
            }
        }
}

module front_bezel() {
    win_x = wall + win_margin_left;
    win_y = wall + button_area_h + win_margin_bottom; // measured from bottom
    pocket_x = wall;
    pocket_y = wall + button_area_h;

    btn_row_y = pocket_y - btn_row_gap; // button hole centers, below the pocket
    btn_start_x = outer_w / 2 + btn_center_offset - btn_pitch;

    difference() {
        union() {
            // Front face slab -- no back layer over the button strip, so
            // switch bodies have clearance behind it in the tray cavity.
            // The slab alone retains the display (win_w/win_h <
            // pocket_w/pocket_h), no separate pocket lip needed.
            linear_extrude(height = bezel_front_t)
                rounded_rect(outer_w, outer_h, corner_r);

            // Side-entry screw flanges (left + right). Sit just INSIDE the
            // `wall` margin (X: wall to 2*wall / outer_w-2*wall to
            // outer_w-wall) -- not at X:0-wall, which is where the tray's
            // own solid side wall physically is; placing the flange there
            // too would have both parts trying to occupy the same space.
            // Reaching into the (otherwise empty, at this Y) tray cavity in
            // Z gives the horizontal screw real material to bite into.
            translate([wall, side_y - side_boss_od / 2, bezel_front_t - 0.01])
                cube([wall, side_boss_od, side_flange_depth]);
            translate([outer_w - 2 * wall, side_y - side_boss_od / 2, bezel_front_t - 0.01])
                cube([wall, side_boss_od, side_flange_depth]);
        }

        // Display window (full depth)
        translate([win_x, win_y, -0.1])
            cube([win_w, win_h, bezel_front_t + bezel_pocket_depth + 0.2]);

        // Display pocket recess (board sits here)
        translate([pocket_x, pocket_y, bezel_front_t - 0.01])
            cube([pocket_w, pocket_h, bezel_pocket_depth + 0.2]);

        // Button holes: straight bore, full front-face depth. Switches
        // press in from behind; cap pokes out the front.
        for (i = [0:2]) {
            bx = btn_start_x + i * btn_pitch;
            translate([bx, btn_row_y, -0.1])
                cylinder(d = btn_hole, h = bezel_front_t + 0.2);
        }

        // KEY0/KEY1 are side-actuated, so their access hole lives in
        // rear_tray()'s top wall instead.

        // Corner screw clearance holes, with a countersink at the BACK
        // (tray-facing) side, not the front -- was on the front face,
        // which was backwards.
        for (p = boss_positions()) {
            translate([p[0], p[1], -0.1])
                cylinder(d = boss_clear_d, h = bezel_total_t + 0.2);
            translate([p[0], p[1], bezel_total_t - countersink_depth])
                cylinder(d1 = boss_clear_d, d2 = boss_od - 1, h = countersink_depth + 0.1);
        }

        // Side-entry screw pilot holes, through the flanges (self-tapping)
        translate([wall - 0.1, side_y, side_boss_z_bezel])
            rotate([0, 90, 0])
                cylinder(d = side_pilot_d, h = wall + 0.2);
        translate([outer_w - 2 * wall - 0.1, side_y, side_boss_z_bezel])
            rotate([0, 90, 0])
                cylinder(d = side_pilot_d, h = wall + 0.2);
    }
}

module rear_tray() {
    // MCU near the BOTTOM wall (USB-C port lines up with the access slot
    // there, opposite KEY1 on the top wall). Battery bay near the top.
    // Board mounted ROTATED 90deg from its native mcu_w x mcu_l labels --
    // real-hardware photo showed the header pin rows run along the tray's
    // Y axis, not X -- footprint here is mcu_l wide x mcu_w tall, centered.
    mcu_fp_x = mcu_l;
    mcu_fp_y = mcu_w;
    // MCU/USB-C shared X-center, left of case-center -- keeps the MCU out
    // from under the front buttons (which shifted right, see
    // btn_center_offset) and lines the USB-C cutout up with the board.
    mcu_usbc_x_center = outer_w * 0.35;
    mcu_x = mcu_usbc_x_center - mcu_fp_x / 2;
    mcu_y = wall + 4;

    // Battery mounted ROTATED 90deg from its native batt_w x batt_h labels
    // too -- real photo showed the long 60mm edge runs along the tray's Y
    // axis. Shifted flush-left to dodge display-PCB components.
    batt_x = wall + 4; // Was wall+1 -- too close to the left wall, offset more
    batt_y = outer_h - wall - batt_w - 4; // Was -7, before that -11 -- freed
                                           // more room for the switch below
                                           // to move up a real amount, not
                                           // just a couple mm (still clears
                                           // the top rim/flange area above)

    // Slide switch hole. Sits well to the right of the battery bay in X
    // (battery's right edge is at batt_x-0.5+batt_h+1 =~43.7, switch center
    // is ~58) -- they're side-by-side, not stacked, so the switch was never
    // actually constrained by the battery's Y position at all. Freed from
    // that and pushed up near the top rim instead, clearing the display's
    // ribbon cable near the bottom/MCU by a wide margin.
    // ASSUMPTION: confirm before printing.
    sw_x = outer_w * 0.7 - sw_slot_h / 2;
    sw_top_clearance = 8.0; // gap below the pocket's top edge
    sw_center_y = wall + button_area_h + pocket_h - sw_top_clearance - sw_body_l / 2;
    sw_y = sw_center_y - sw_slot_w / 2;
    sw_center_x = sw_x + sw_slot_h / 2;

    // USB-C slot Z, anchored to the MCU's standoff height rather than a
    // floating centered guess (which a real print showed sitting too high).
    // Still too high after the first correction -- lowered further to
    // track mcu_standoff_h coming down too.
    usbc_slot_z = tray_floor_t + 1.0;

    difference() {
        union() {
            // Floor, rounded exterior corners
            linear_extrude(height = tray_floor_t)
                rounded_rect(outer_w, outer_h, corner_r);
            // Perimeter wall -- 0.01mm overlap into the floor to avoid a
            // flush coincident face producing degenerate shells.
            translate([0, 0, tray_floor_t - 0.01])
                difference() {
                    linear_extrude(height = tray_interior_depth + 0.01)
                        rounded_rect(outer_w, outer_h, corner_r);
                    translate([wall, wall, -0.1])
                        linear_extrude(height = tray_interior_depth + 0.2)
                            rounded_rect(outer_w - 2 * wall, outer_h - 2 * wall, max(corner_r - wall, 0.5));
                }
            screw_bosses(boss_pilot_d, tray_wall_h);

            // Slide switch containment pocket -- the switch's body nests
            // here so only its slide-tab pokes through the hole below.
            translate([sw_center_x - sw_body_w / 2, sw_center_y - sw_body_l / 2, tray_floor_t]) {
                difference() {
                    cube([sw_body_w, sw_body_l, sw_pocket_h]);
                    translate([(sw_body_w - sw_slot_h) / 2, (sw_body_l - sw_slot_w) / 2, -0.1])
                        cube([sw_slot_h, sw_slot_w, sw_pocket_h + 0.2]);
                }
            }
        }

        // USB-C access slot, shifted left with the MCU (see mcu_usbc_x_center)
        translate([mcu_usbc_x_center - usbc_slot_w / 2, -0.1, usbc_slot_z])
            cube([usbc_slot_w, wall + 0.2, usbc_slot_h]);

        // Side-entry screw clearance holes (left + right), near the top --
        // screws pass through here into the bezel's flanges (front_bezel()).
        translate([-0.1, side_y, side_boss_z_tray])
            rotate([0, 90, 0])
                cylinder(d = side_clear_d, h = wall + 0.2);
        translate([outer_w - wall - 0.1, side_y, side_boss_z_tray])
            rotate([0, 90, 0])
                cylinder(d = side_clear_d, h = wall + 0.2);

        // KEY1 access hole -- side-actuated, cut through the top wall at
        // KEY1's real X position, centered in the wall's height.
        disp_btn_z = tray_wall_h / 2 + 0.75;
        translate([wall + disp_btn_x1 - disp_btn_w / 2, outer_h - wall - 0.1, disp_btn_z - disp_btn_h / 2])
            cube([disp_btn_w, wall + 0.2, disp_btn_h]);

        // Slide switch access hole
        translate([sw_x, sw_y, -0.1])
            cube([sw_slot_h, sw_slot_w, tray_floor_t + 0.2]);

        // "Forager by Max" -- indented text on the exterior bottom face,
        // bottom-right corner. mirror() makes it read correctly from below
        // (outside the case); mirroring flips left/right, so "bottom-right"
        // as seen from outside means placing it at LOW X here.
        translate([outer_w - wall - 5, wall + 6, -0.1])
            mirror([1, 0, 0])
                linear_extrude(height = 0.8)
                    text("Forager by Max", size = 3.6, font = "Marker Felt",
                         halign = "center", valign = "center");
    }

    // MCU standoffs -- just 2, at the top of the footprint (the retaining
    // wall this used to pair with never fit the real board well and was
    // dropped; these posts alone are enough to clear the header pins
    // soldered on the board's underside).
    for (p = [[0, mcu_fp_y], [mcu_fp_x, mcu_fp_y]])
        translate([mcu_x + p[0], mcu_y + p[1], tray_floor_t])
            cylinder(d = 3.5, h = mcu_standoff_h);

    // Battery bay walls (shallow retaining lip, held by friction/tape)
    translate([batt_x - 0.5, batt_y - 0.5, tray_floor_t]) {
        difference() {
            cube([batt_h + 1, batt_w + 1, 1.5]);
            translate([0.5, 0.5, -0.1]) cube([batt_h, batt_w, 2]);
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
