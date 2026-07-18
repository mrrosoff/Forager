// Forager enclosure — 2-part (front bezel + rear tray), screwed together.
//
// Hardware fit (measured / vendor-spec):
//   - Display: Waveshare 4.2" e-paper module (GDEY042T81), PCB 93.5 x 78.5mm,
//     active area 84.8 x 63.6mm.
//   - MCU: ESP32-S3 Super Mini, 22.52 x 18mm.
//   - Battery: 2000mAh LiPo pouch, 60 x 36 x 7mm.
//   - Buttons: 3x standard 6x6mm tactile switches, mounted in the front bezel.
//   - Slide switch: generic small SPST slide switch (assumed footprint below —
//     confirm/adjust SWITCH_* before printing).
//
// ASSUMPTIONS flagged inline with "ASSUMPTION:" — verify against real parts
// before printing, especially display board thickness/connector margin and
// the slide switch footprint (no spec was given for those).
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
disp_w = 93.5;
disp_h = 78.5;
disp_t = 4.5;   // ASSUMPTION: board + FPC bump thickness, not vendor-specced
win_w  = 84.8;  // active area
win_h  = 63.6;
win_margin_lr = (disp_w - win_w) / 2;      // 4.35, symmetric
win_margin_top = 6.0;                       // ASSUMPTION: verify against board photo
win_margin_bottom = disp_h - win_h - win_margin_top; // 8.9

// ---- MCU: ESP32-S3 Super Mini ----
mcu_w = 22.52;
mcu_l = 18.0;
mcu_component_h = 4.0; // clearance for USB-C connector / header stubs

// ---- Battery: 2000mAh LiPo, 60 x 36 x 7mm ----
batt_w = 60.0;
batt_h = 36.0;
batt_t = 7.0;

// ---- Buttons: 3x 6x6mm tactile, in a row below the display window ----
btn_body      = 6.0;
btn_hole      = 6.3;  // counterbore the switch body presses into, from behind
btn_counterbore_depth = 1.4;
btn_cap_hole  = 3.6;  // through-hole the actuator cap pokes through
btn_pitch     = 16.0;
btn_row_gap   = 8.0;  // gap from window bottom edge to button hole centers

// ---- Display module's own 2 onboard buttons (left edge of the board) ----
// PLACEHOLDER: exact position/size not yet given — these sit in the narrow
// win_margin_lr strip (4.35mm) between the board edge and the active window,
// so diameter must stay small. Update disp_btn_* once measured; the holes
// are cut from the front face straight onto the board's surface, same as
// the main buttons.
disp_btn_d  = 3.0;
disp_btn_y1 = 0.25; // fraction of pocket_h from the bottom of the pocket
disp_btn_y2 = 0.70;

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

outer_w = pocket_w + 2 * wall;                 // 98.5
button_area_h = 20.0;                          // room below window for button row
outer_h = pocket_h + wall + button_area_h + wall; // 103.5

bezel_front_t = 2.4;   // front face thickness (button cap holes live here)
bezel_lip     = 1.5;   // inward lip that catches the display's front edge
bezel_pocket_depth = disp_t + clearance; // depth behind the lip for the board
bezel_total_t = bezel_front_t + bezel_pocket_depth;

tray_interior_depth = max(batt_t, mcu_component_h) + clearance; // 7.3
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

        // Display's onboard buttons (left edge of the board, in the
        // win_margin_lr strip) — PLACEHOLDER position, see disp_btn_* above
        disp_btn_x = pocket_x + win_margin_lr / 2;
        for (f = [disp_btn_y1, disp_btn_y2])
            translate([disp_btn_x, pocket_y + f * pocket_h, -0.1])
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
