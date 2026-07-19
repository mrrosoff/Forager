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

win_margin_lr = 5.0; // Was 8.9 -- real board showed the window needed to be
                      // wider to show the full 63.6mm active area.
win_w = disp_w - 2 * win_margin_lr;

// ---- MCU: ESP32-S3 Super Mini ----
mcu_w = 22.52;
mcu_l = 18.0;
mcu_component_h = 4.0; // clearance for USB-C connector / header stubs
mcu_wall_h = 2.5; // retaining wall around the MCU footprint (standoffs
                   // alone don't stop it sliding sideways)

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

// ---- Display module's onboard buttons (now the case's TOP edge) ----
// KEY0/KEY1 sit on the board's native RIGHT edge, FPC on the native LEFT
// edge. After rotation KEY0/KEY1 land on the case's TOP edge, FPC on the
// BOTTOM. Only KEY1 (wired as PIN_BTN_SETTINGS) gets a cutout.
disp_btn_w = 4.0;  // KEY1's real actuator is a rectangle, not round (real-
disp_btn_h = 7.0;  // hardware photo) -- same rough finger-press area as
                    // the old 6mm circle, shaped to match.
disp_btn_x1 = 16.03; // KEY1 -- has a cutout
disp_btn_x2 = 33.70; // KEY0 -- reference only, no cutout

top_rim = 6.0;  // Was 10.0 -- shrunk since KEY1 needs to sit near the top
                 // edge, not buried under a wide rim.

// ---- USB-C access (in tray side wall, aligned to MCU edge) ----
usbc_slot_w = 10.0;
usbc_slot_h = 5.0; // extra vertical tolerance since the slot is now
                    // anchored to the MCU's mounting height (see
                    // usbc_slot_z below), not a floating guess

// ---- Slide switch (ASSUMPTION — no spec given, adjust before printing) ----
sw_slot_w = 12.0;
sw_slot_h = 3.0;  // Was 4.0 -- real switch reads shorter (real-hardware photo)
sw_body_w  = 7.0;  // ASSUMPTION: switch body footprint (deeper than the
sw_body_l  = 13.0; // bare slot cut alone can seat)
sw_pocket_h = 4.5;  // ASSUMPTION: interior pocket depth to contain the
                     // switch body so it sits flush, not proud of the floor

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
button_area_h = 16.0; // Was 20.0 -- shrunk, buttons still fit with margin
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

// ---- Top-center snap-fit fastener (replaces a screw there) ----
// The old top-center screw's clearance hole punched into the display
// pocket once the window grew taller -- risked hitting the board. Swapped
// for a snap-fit: a flexible post on the tray (rear_tray()) pushes through
// a slot in the bezel (front_bezel()) above the pocket, barbs past it, and
// hooks on the outer face. ASSUMPTION: dimensions are a first estimate --
// printed vertical posts flex less readily than injection-molded ones, so
// the barb overhang is kept small (0.8mm). If it snaps off, reprint with
// the post reoriented, or glue just this one point.
snap_post_w = 4.0;
snap_post_t = 1.4;
snap_barb_h = 1.6;
snap_barb_overhang = 0.8;
snap_slot_clearance = 0.3; // slot sized post dims + this, per side
snap_y = outer_h - wall - snap_post_t / 2; // shared target Y for both parts

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
    win_x = wall + win_margin_lr;
    win_y = wall + button_area_h + win_margin_bottom; // measured from bottom
    pocket_x = wall;
    pocket_y = wall + button_area_h;

    btn_row_y = pocket_y - btn_row_gap; // button hole centers, below the pocket
    btn_start_x = outer_w / 2 - btn_pitch;

    difference() {
        // Front face slab -- no back layer over the button strip, so
        // switch bodies have clearance behind it in the tray cavity. The
        // slab alone retains the display (win_w/win_h < pocket_w/pocket_h),
        // no separate pocket lip needed.
        linear_extrude(height = bezel_front_t)
            rounded_rect(outer_w, outer_h, corner_r);

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

        // Corner screw clearance holes, with a countersink at the front
        // face so a flat/pan screw head sits recessed
        for (p = boss_positions()) {
            translate([p[0], p[1], -0.1])
                cylinder(d = boss_clear_d, h = bezel_total_t + 0.2);
            translate([p[0], p[1], -0.1])
                cylinder(d1 = boss_od - 1, d2 = boss_clear_d, h = countersink_depth + 0.1);
        }

        // Top-center snap-fit slot -- the tray's flexible post pushes
        // through here and barbs past the outer face.
        translate([outer_w / 2 - (snap_post_w + snap_slot_clearance) / 2,
                   snap_y - (snap_post_t + snap_slot_clearance) / 2, -0.1])
            cube([snap_post_w + snap_slot_clearance, snap_post_t + snap_slot_clearance, bezel_front_t + 0.2]);
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
    mcu_x = outer_w / 2 - mcu_fp_x / 2;
    mcu_y = wall + 4;

    // Battery mounted ROTATED 90deg from its native batt_w x batt_h labels
    // too -- real photo showed the long 60mm edge runs along the tray's Y
    // axis. Shifted flush-left to dodge display-PCB components.
    batt_x = wall + 1;
    batt_y = outer_h - wall - batt_w - 11; // moved down 5mm from the top edge

    // Slide switch hole, off-center in the gap between MCU and battery.
    // ASSUMPTION: confirm before printing.
    sw_x = outer_w * 0.7 - sw_slot_h / 2;
    sw_y = (mcu_y + mcu_fp_y + batt_y) / 2 - sw_slot_w / 2;
    sw_center_x = sw_x + sw_slot_h / 2;
    sw_center_y = sw_y + sw_slot_w / 2;

    // USB-C slot Z, anchored to the MCU's standoff height rather than a
    // floating centered guess (which a real print showed sitting too high).
    usbc_slot_z = tray_floor_t + 1.5;

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

            // Top-center snap-fit post -- flexes through the bezel's slot,
            // then barbs past it and hooks on the outer face.
            snap_post_h = tray_wall_h + bezel_front_t + 1.0; // pokes proud of the bezel
            translate([outer_w / 2 - snap_post_w / 2, snap_y - snap_post_t / 2, tray_floor_t]) {
                // +0.3 overlap into the top wall (back side only) to avoid
                // a flush coincident face.
                cube([snap_post_w, snap_post_t + 0.3, snap_post_h - snap_barb_h]);
                translate([0, 0, snap_post_h - snap_barb_h])
                    hull() {
                        cube([snap_post_w, snap_post_t + snap_barb_overhang, 0.1]);
                        translate([0, snap_barb_overhang * 0.3, snap_barb_h - 0.1])
                            cube([snap_post_w, snap_post_t * 0.6, 0.1]);
                    }
            }

            // MCU retaining wall -- cradles the board's edges (standoffs
            // alone don't stop it sliding).
            translate([mcu_x - 0.5, mcu_y - 0.5, tray_floor_t]) {
                difference() {
                    cube([mcu_fp_x + 1, mcu_fp_y + 1, mcu_wall_h]);
                    translate([0.5, 0.5, -0.1]) cube([mcu_fp_x, mcu_fp_y, mcu_wall_h + 0.2]);
                }
            }

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

        // USB-C access slot, centered on the bottom wall
        translate([outer_w / 2 - usbc_slot_w / 2, -0.1, usbc_slot_z])
            cube([usbc_slot_w, wall + 0.2, usbc_slot_h]);

        // KEY1 access hole -- side-actuated, cut through the top wall at
        // KEY1's real X position, centered in the wall's height.
        disp_btn_z = tray_wall_h / 2 + 0.75;
        translate([wall + disp_btn_x1 - disp_btn_w / 2, outer_h - wall - 0.1, disp_btn_z - disp_btn_h / 2])
            cube([disp_btn_w, wall + 0.2, disp_btn_h]);

        // Slide switch access hole
        translate([sw_x, sw_y, -0.1])
            cube([sw_slot_h, sw_slot_w, tray_floor_t + 0.2]);

        // "Forager by Max" -- small indented text on the exterior bottom
        // face, centered in the gap between MCU and battery. mirror()
        // makes it read correctly from below (outside the case).
        translate([outer_w / 2, (mcu_y + mcu_fp_y + batt_y) / 2, -0.1])
            mirror([1, 0, 0])
                linear_extrude(height = 0.8)
                    text("Forager by Max", size = 2.4, font = "Marker Felt",
                         halign = "center", valign = "center");
    }

    // MCU standoffs
    for (p = [[0,0],[mcu_fp_x,0],[0,mcu_fp_y],[mcu_fp_x,mcu_fp_y]])
        translate([mcu_x + p[0], mcu_y + p[1], tray_floor_t])
            cylinder(d = 3.5, h = 1.5);

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
