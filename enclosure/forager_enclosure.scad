// Forager enclosure — 2-part (front bezel + rear tray), screwed together.
//
// PORTRAIT LAYOUT: the display PCB is physically landscape (93.5 x 78.5mm)
// but the UI renders rotated 90deg into a portrait 300x400 canvas
// (epd.setRotation(3), see CLAUDE.md). The board mounts rotated 90deg to
// match: its long 93.5mm edge runs along the case's Y axis.
//
// Hardware (measured / vendor-spec):
//   - Display: Waveshare 4.2" e-paper (GDEY042T81), PCB 93.5x78.5mm, active
//     area 84.8x63.6mm, board+FPC thickness 12.0mm.
//   - MCU: ESP32-S3 Super Mini, 22.52 x 18mm.
//   - Battery: 2000mAh LiPo pouch, 60 x 36 x 7mm.
//   - Buttons: 3x 6x6mm tactile, in the front bezel.
//   - Slide switch: 4 x 8mm (ASSUMPTION on exact part).
//
// CASE-X AXIS IS MIRRORED FROM PHYSICAL LEFT/RIGHT: confirmed against a
// real print -- physical RIGHT is case-X's low side (X near 0), physical
// LEFT is case-X's high side (X near outer_w). Keep this in mind before
// changing any *_x_lo/*_x_hi value.
//
// Render one part at a time:
//   openscad -D part=\"bezel\" -o stl/front_bezel.stl forager_enclosure.scad
//   openscad -D part=\"tray\"  -o stl/rear_tray.stl   forager_enclosure.scad
//   openscad -D part=\"both\"  -o preview/assembled.png forager_enclosure.scad
part = "both"; // "bezel" | "tray" | "both" (both = preview only, side by side)

$fn = 48;

// ---- General ----
wall      = 1.5;   // side-wall thickness (Y / top+bottom, and the X base)
clearance = 0.3;   // general fit clearance around boards
corner_r  = 3.0;   // rounded exterior corners

// Side-entry screw dimensions -- defined early since top_rim/bottom_rim
// below need side_boss_od/side_clear_d to size their bands correctly.
side_boss_od      = 3.5;
side_pilot_d      = 2.0;
side_clear_d      = 2.7;

// wall_x_lo/wall_x_hi: the TRAY's own side-wall thickness. Just a plain
// wall with a small pass-through clearance hole for the screw shaft -- no
// need to be any thicker than the standard `wall`.
wall_x_lo = wall;
wall_x_hi = wall;

// The screw FLANGE (the tab on the bezel side, see front_bezel()) is a
// separate feature from the tray's wall above, and floats independently in
// the tray's open cavity. flange_w is its own thickness -- kept at a
// sturdier 2.2mm (not the thinner general `wall`) since it's the
// self-tapping screw's actual thread engagement depth, not just a cosmetic
// enclosure wall.
flange_w = 2.2;
// flange_inset is a fixed 5mm from the case edge -- independent of
// corner_r now (not corner_r+1), so shrinking the corner radius doesn't
// silently resize the bezel margins that depend on this.
flange_inset = 5.0;

// Gap from the flange's far face to where the display pocket starts, so
// the flange has clearance beside the board instead of colliding with it.
flange_side_clearance = 2.0;
// Case edge to pocket edge, both sides (symmetric: flange position and
// size don't differ side to side -- the bezel-width asymmetry comes
// entirely from win_margin_left/right below, the real hardware offset).
pocket_x_inset = flange_inset + flange_w + flange_side_clearance;

// ---- Display module (Waveshare 4.2", GDEY042T81) ----
// Rotated 90deg from native landscape PCB layout, so disp_w/disp_h/win_w/
// win_h below are CASE-axis (X=width, Y=height), not vendor labels:
//   native disp_w (93.5, long edge)  -> case-Y (disp_h)
//   native disp_h (78.5, short edge) -> case-X (disp_w)
//   native win_w  (84.8, active long axis)  -> case-Y (win_h)
//   native win_h  (63.6, active short axis) -> case-X (win_w)
disp_w = 70.5;  // real board measured smaller than modeled, same story as disp_h below
disp_h = 90.5;  // real board measures 3mm shorter than the vendor spec
disp_t = 12.0;  // board + FPC connector bump
disp_wire_clearance = 3.0; // room behind the board for the FPC cable to bend

win_h  = 86.5;  // real measured visible screen height (was modeled too tall)
win_margin_top = (disp_h - win_h) / 2;
win_margin_bottom = win_margin_top;

// Measured from the board: a small margin on one side, a much wider one on
// the other. Case-X is mirrored from physical (see header note), so the
// wide margin lands on win_margin_right here. 0.3mm floor instead of a true
// 0 so the window cut doesn't run flush into the pocket cut.
win_margin_left  = 0.3;
win_margin_right = 5.2;
win_w = disp_w - win_margin_left - win_margin_right; // = 65.0, real measured visible screen width

// ---- MCU: ESP32-S3 Super Mini ----
mcu_w = 22.52;
mcu_l = 18.0;
mcu_component_h = 4.0; // clearance for USB-C connector / header stubs
mcu_standoff_h = 3.5;  // tall enough to actually contact the board

// ---- Battery: 2000mAh LiPo, 60 x 36 x 7mm ----
batt_w = 60.0;
batt_h = 36.0;
batt_t = 7.0;
batt_puff_clearance = 2.0; // LiPo pouches puff up over time

// ---- Buttons: 3x 6x6mm tactile, in a row below the display window ----
btn_body      = 6.0;
btn_hole      = 6.3;  // counterbore the switch body presses into, from behind
btn_counterbore_depth = 1.4;
btn_cap_hole  = 3.6;  // through-hole the actuator cap pokes through
btn_pitch     = 13.0;
btn_row_gap   = 5.65;  // gap from window bottom edge to button hole centers -- centered
                       // in button_area_h (button_area_h/2)
btn_center_offset = (win_margin_left - win_margin_right) / 2; // centers the
                          // row under the WINDOW, not outer_w/2, since the
                          // window itself isn't centered (asymmetric margins)

// ---- Display module's onboard buttons (now the case's TOP edge) ----
// KEY0/KEY1 sit on the board's native RIGHT edge, FPC on the native LEFT
// edge; after rotation that's the case's TOP/BOTTOM. Only KEY1 (wired as
// PIN_BTN_SETTINGS) gets a cutout.
disp_btn_w = 6.0;
disp_btn_h = 3.5;
disp_btn_x1 = 16.03; // KEY1 -- has a cutout
disp_btn_x2 = 33.70; // KEY0 -- reference only, no cutout

// top_rim/bottom_rim: bands above/below the display pocket.
//
// Only the BOTTOM pair of screws still lives up against a corner (see
// side_y_bottom/side_ys below) -- the top pair moved down to 10mm off the
// top edge (see side_y_top below), floating in the side margin next to the
// pocket rather than needing its own dedicated band, since a corner-hugging
// screw up there was costing ~4mm of bezel it didn't need to. A corner
// screw needs real clearance for two reasons:
//   - The bezel's flange sits at X=flange_inset (>= corner_r), so it's
//     safe at ANY Y as long as it doesn't touch the flat edge exactly.
//   - The TRAY's clearance hole enters right at the true exterior surface
//     (X=0), where corner_r actually matters: the case boundary near a
//     corner curves away from X=0 for any Y < corner_r, so a hole drilled
//     from X=0 at Y < corner_r isn't drilled through real wall material at
//     its outer opening -- it's carved into the curve itself.
// So side_screw_edge_clearance needs >= corner_r + the hole's own radius
// (side_clear_d/2) + a small buffer, not just side_boss_od/2. bottom_rim
// still needs to contain that. top_rim no longer hosts a screw at all, so
// it only needs to be a plain closing wall.
side_screw_edge_clearance_bottom = corner_r + side_clear_d / 2 + 0.3;
top_rim    = wall + 1.0;
bottom_rim = side_screw_edge_clearance_bottom + side_boss_od / 2 + 0.3;

// ---- USB-C access (in tray side wall, aligned to MCU edge) ----
usbc_slot_w = 9.5;
usbc_slot_h = 4.0;

// ---- Slide switch ----
// Real switch measures 4 x 8mm. sw_body_w/sw_body_l follow from the slot
// plus a uniform 1mm containment-wall margin on every side.
sw_slot_w = 9.0;
sw_slot_h = 4.5;
sw_wall_margin = 1.0;
sw_body_w  = sw_slot_h + 2 * sw_wall_margin;
sw_body_l  = sw_slot_w + 2 * sw_wall_margin;
sw_pocket_h = 6.5;

// ---- Derived footprint ----
pocket_w = disp_w + 2 * clearance;
pocket_h = disp_h + 2 * clearance;

outer_w = pocket_w + 2 * pocket_x_inset; // pocket_x_inset already includes the wall
button_area_h = 11.3; // sized to hit the 20mm bottom-bezel target with the new bottom_rim
outer_h = pocket_h + bottom_rim + button_area_h + top_rim;

bezel_front_t = 2.4;   // front face thickness (button cap holes live here)
bezel_pocket_depth = disp_t + clearance + disp_wire_clearance;
bezel_total_t = bezel_front_t + bezel_pocket_depth;

disp_tray_protrusion = 13.0; // ASSUMPTION: how far the display's
                              // pin-header/FPC assembly sticks into the
                              // TRAY cavity, beyond the bezel-side pocket
tray_interior_depth = max(batt_t + batt_puff_clearance, mcu_component_h, disp_tray_protrusion) + clearance;
tray_floor_t = 2.2;
tray_wall_h  = tray_interior_depth + tray_floor_t;

// ---- Side-entry screws, mid-height AND bottom ----
// 4 horizontal screws total, through the tray's LEFT/RIGHT walls, each
// biting into a small flange on the bezel's side edges (front_bezel()).
// Confined to the flange_inset X range, so they can't collide with the
// window/pocket cuts regardless of Y.
side_flange_depth = 8.0; // how far the bezel's flange reaches into the tray cavity

// side_y_top: 10mm off the top edge (the low end of a "1-2cm" ask -- the
// battery bay's own height barely fits below it at 10mm; 15mm+ doesn't
// leave enough room for the battery no matter where it sits). Clear on
// both sides: case-X high (physical LEFT) has nothing else out there at
// all; case-X low (physical RIGHT) required moving the battery bay down
// (see batt_y in rear_tray()) to open up real clearance above it.
mcu_top_edge = wall + 4 + mcu_w; // mcu_y + mcu_fp_y, replicated here since
                                  // this needs to be a top-level value
side_y_top    = outer_h - 10;
side_y_bottom = side_screw_edge_clearance_bottom;
side_ys = [side_y_top, side_y_bottom];
side_boss_z_bezel = bezel_front_t + side_flange_depth / 2; // bezel-local Z
side_boss_z_tray  = tray_wall_h - side_flange_depth / 2;   // tray-local Z (see modules below)

// Rounded-rectangle outline (2D) for the case's exterior footprint.
module rounded_rect(w, h, r) {
    hull() {
        translate([r, r]) circle(r = r);
        translate([w - r, r]) circle(r = r);
        translate([r, h - r]) circle(r = r);
        translate([w - r, h - r]) circle(r = r);
    }
}

module front_bezel() {
    pocket_x = pocket_x_inset;
    pocket_y = bottom_rim + button_area_h;
    win_x = pocket_x + win_margin_left;
    win_y = bottom_rim + button_area_h + win_margin_bottom;

    btn_row_y = pocket_y - btn_row_gap; // button hole centers, below the pocket
    btn_start_x = outer_w / 2 + btn_center_offset - btn_pitch;

    difference() {
        union() {
            // Front face slab. No back layer over the button strip, so
            // switch bodies clear it in the tray cavity. The slab alone
            // retains the display (win_w/win_h < pocket_w/pocket_h).
            linear_extrude(height = bezel_front_t)
                rounded_rect(outer_w, outer_h, corner_r);

            // Side-entry screw flanges (left+right, top+bottom -- 4
            // total), floating in the tray's open cavity at flange_inset
            // from the case edge (independent of the tray's own thin
            // wall), reaching into the tray cavity in Z to give the
            // horizontal screw material to bite into.
            for (sy = side_ys) {
                translate([flange_inset, sy - side_boss_od / 2, bezel_front_t - 0.01])
                    cube([flange_w, side_boss_od, side_flange_depth]);
                translate([outer_w - flange_inset - flange_w, sy - side_boss_od / 2, bezel_front_t - 0.01])
                    cube([flange_w, side_boss_od, side_flange_depth]);
            }
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

        // Side-entry screw pilot holes, through the flanges (self-tapping)
        for (sy = side_ys) {
            translate([flange_inset - 0.1, sy, side_boss_z_bezel])
                rotate([0, 90, 0])
                    cylinder(d = side_pilot_d, h = flange_w + 0.2);
            translate([outer_w - flange_inset - flange_w - 0.1, sy, side_boss_z_bezel])
                rotate([0, 90, 0])
                    cylinder(d = side_pilot_d, h = flange_w + 0.2);
        }
    }
}

module rear_tray() {
    // MCU near the BOTTOM wall (USB-C lines up with the access slot
    // there, opposite KEY1 on the top wall). Battery bay near the top.
    // Board mounted rotated 90deg from its native mcu_w x mcu_l labels.
    mcu_fp_x = mcu_l;
    mcu_fp_y = mcu_w;
    // MCU/USB-C shared X-center, left of case-center -- clears the front
    // buttons (shifted right, see btn_center_offset) and lines the
    // USB-C cutout up with the board.
    mcu_usbc_x_center = outer_w * 0.25; // battery side (case-X low, physical RIGHT), clear of its footprint
    mcu_x = mcu_usbc_x_center - mcu_fp_x / 2;
    mcu_y = wall + 4;

    // Battery mounted rotated 90deg from its native batt_w x batt_h
    // labels; long edge runs along the tray's Y axis, flush-left. Shifted
    // down from the top edge (was flush against it) to leave real
    // clearance for the top screw at side_y_top -- 4mm above the MCU,
    // then the same wall_margin+fit_clearance/2 (0.5+1.25=1.75) the
    // battery bay's own geometry below uses, so its cube starts exactly
    // 4mm above the MCU rather than an inch-perfect coincidence.
    batt_x = wall_x_lo + 4;
    batt_y = mcu_y + mcu_fp_y + 4.0 + 1.75;

    // Slide switch hole, well to the right of the battery bay in X (not
    // stacked -- switch Y is unconstrained by the battery), pushed up
    // near the top rim, clear of the display's ribbon cable near the MCU.
    // ASSUMPTION: confirm before printing.
    sw_x = outer_w * 0.7 - 1.0 - sw_slot_h / 2;
    sw_top_clearance = 8.0; // gap below the pocket's top edge
    sw_center_y = bottom_rim + button_area_h + pocket_h - sw_top_clearance - sw_body_l / 2;
    sw_y = sw_center_y - sw_slot_w / 2;
    sw_center_x = sw_x + sw_slot_h / 2;

    // USB-C slot Z, anchored to the MCU's standoff height.
    usbc_slot_z = tray_floor_t + 1.0;

    difference() {
        union() {
            // Floor, rounded exterior corners
            linear_extrude(height = tray_floor_t)
                rounded_rect(outer_w, outer_h, corner_r);
            // Perimeter wall -- 0.01mm overlap into the floor to avoid a
            // flush coincident face producing degenerate shells. A plain
            // uniform `wall` thickness all around: the screw flange floats
            // independently in this cavity (see flange_inset above), so
            // the wall itself no longer needs extra thickness anywhere.
            translate([0, 0, tray_floor_t - 0.01])
                difference() {
                    linear_extrude(height = tray_interior_depth + 0.01)
                        rounded_rect(outer_w, outer_h, corner_r);
                    translate([wall, wall, -0.1])
                        linear_extrude(height = tray_interior_depth + 0.2)
                            rounded_rect(outer_w - 2 * wall, outer_h - 2 * wall, max(corner_r - wall, 0.5));
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

        // USB-C access slot, shifted left with the MCU (see mcu_usbc_x_center)
        translate([mcu_usbc_x_center - usbc_slot_w / 2, -0.1, usbc_slot_z])
            cube([usbc_slot_w, wall + 0.2, usbc_slot_h]);

        // Side-entry screw clearance holes (left+right, top+bottom) --
        // screws pass through the tray's thin wall, then open cavity air,
        // into the bezel's flanges (front_bezel()), which float at
        // flange_inset rather than right at the tray's own wall.
        for (sy = side_ys) {
            translate([-0.1, sy, side_boss_z_tray])
                rotate([0, 90, 0])
                    cylinder(d = side_clear_d, h = flange_inset + flange_w + 0.2);
            translate([outer_w - flange_inset - flange_w - 0.1, sy, side_boss_z_tray])
                rotate([0, 90, 0])
                    cylinder(d = side_clear_d, h = flange_inset + flange_w + 0.2);
        }

        // KEY1 access hole -- side-actuated, cut through the top wall at
        // KEY1's real X position, centered in the wall's height.
        disp_btn_z = tray_wall_h / 2 + 1.75;
        translate([pocket_x_inset + disp_btn_x1 - disp_btn_w / 2, outer_h - wall - 0.1, disp_btn_z - disp_btn_h / 2])
            cube([disp_btn_w, wall + 0.2, disp_btn_h]);

        // Slide switch access hole
        translate([sw_x, sw_y, -0.1])
            cube([sw_slot_h, sw_slot_w, tray_floor_t + 0.2]);

        // "Forager by Max" -- indented text on the exterior bottom face,
        // bottom-right corner as seen from OUTSIDE. mirror() flips it to
        // read correctly from that side (model coords are the inside view).
        translate([outer_w - wall_x_hi - 20, wall + 6, -0.1])
            mirror([1, 0, 0])
                linear_extrude(height = 0.8)
                    text("Forager by Max", size = 3.6, font = "Noteworthy:style=Bold",
                         halign = "center", valign = "center");
    }

    // MCU standoff -- single post centered under the USB-C port.
    translate([mcu_usbc_x_center, mcu_y + mcu_fp_y - 5, tray_floor_t])
        cylinder(d = 3.5, h = mcu_standoff_h);

    // Battery bay walls (shallow retaining lip, held by friction/tape).
    batt_wall_margin = 0.5;
    batt_fit_clearance = 2.5; // real clearance beyond the battery's own footprint
    batt_wall_h = 4.0;
    translate([batt_x - batt_wall_margin - batt_fit_clearance / 2, batt_y - batt_wall_margin - batt_fit_clearance / 2, tray_floor_t]) {
        difference() {
            cube([batt_h + 2 * batt_wall_margin + batt_fit_clearance, batt_w + 2 * batt_wall_margin + batt_fit_clearance, batt_wall_h]);
            translate([batt_wall_margin, batt_wall_margin, -0.1])
                cube([batt_h + batt_fit_clearance, batt_w + batt_fit_clearance, batt_wall_h + 0.5]);
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
