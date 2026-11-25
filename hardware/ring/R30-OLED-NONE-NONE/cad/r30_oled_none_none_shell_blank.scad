// SPDX-License-Identifier: CERN-OHL-S-2.0
//
// Serviceable first-board shell packet for the active optical ring lane.
// This is an honest mechanical packet, not a validated enclosure.

$fn = 96;

finger_circumference_mm = 60;
sensor_angle_deg = 30;
band_width_mm = 18;
shell_height_mm = 7.8;
wall_thickness_mm = 1.6;
radial_thickness_mm = 7.0;
// The installed 0.5 mm glide pad is exposed below the rim.  The rim is
// shortened so the actual pad contact plane, not the pocket floor, remains
// inside the 10 mm finger-to-surface envelope.
rim_height_mm = 1.7;
rim_radial_width_mm = 2.6;
glide_pad_thickness_mm = 0.5;
sensor_aperture_diameter_mm = 6.2;
sensor_platform_offset_mm = 8.5;
service_seam_height_mm = 5.2;
lid_top_skin_mm = 1.2;
lid_skirt_height_mm = 1.4;
seam_clearance_mm = 0.25;
service_pry_slot_mm = [4.8, 2.0, 1.4];

// First routed PCB facts, mapped from kicad/r30_oled_none_none.kicad_pcb.
// Board center is KiCad (120.500, 100.000); +X points toward the ESP32 antenna.
pcb_size_mm = [43.0, 18.0, 1.0];
pcb_clearance_mm = 0.35;
pcb_center_z_mm = 3.6;
pcb_top_z_mm = pcb_center_z_mm + pcb_size_mm[2] / 2;
// The electronics pod is lateral to the finger passage.  The board is not
// allowed to cross the central finger-hole volume merely because it is above
// the ring body.
pcb_pod_center_mm = [0, 23.4, 0];
pcb_tray_clearance_mm = [pcb_size_mm[0] + (2 * pcb_clearance_mm),
                         pcb_size_mm[1] + (2 * pcb_clearance_mm),
                         pcb_size_mm[2] + 0.6];
pcb_tray_center_mm = [pcb_pod_center_mm[0], pcb_pod_center_mm[1], pcb_center_z_mm];

// The PCB is 18 mm wide.  The wider y envelope is intentional: the two
// service screws sit outside the PCB rather than over components or pads.
// This is a lateral-pod tradeoff to be checked for accessibility, not hidden
// by pretending the original 21 mm pod could carry closure hardware.
pcb_pod_margin_mm = [1.8, 4.3, 0];
pcb_pod_size_mm = [pcb_size_mm[0] + (2 * pcb_pod_margin_mm[0]),
                   pcb_size_mm[1] + (2 * pcb_pod_margin_mm[1]),
                   shell_height_mm];
pcb_pod_corner_radius_mm = 2.2;

service_access_center_mm = [pcb_pod_center_mm[0] - 17.85,
                           pcb_pod_center_mm[1],
                           pcb_top_z_mm + 0.70];
service_access_opening_mm = [2.6, 8.2, 1.8];
service_fixture_keepout_mm = [4.2, 7.4, 1.8];

sensor_pcb_center_mm = [-4.00, pcb_pod_center_mm[1], 1.7];
dome_switch_center_mm = [1.50, pcb_pod_center_mm[1] - 2.45, pcb_top_z_mm + 0.45];
battery_connector_center_mm = [-11.90, pcb_pod_center_mm[1] - 5.75, pcb_top_z_mm + 0.35];
// Final KiCad radio anchor is (133.70, 100), board-local (+13.20, 0).
// The module body is 16.6 x 13.2 x 2.4 mm above the 1.0 mm PCB.
module_center_mm = [13.20, pcb_pod_center_mm[1], pcb_top_z_mm + 1.2];
module_keepout_mm = [16.6, 13.2, 2.4];
// Copper void is KiCad x=136.60..142.00, board-local +16.10..+21.50,
// across the full 18 mm board width.
antenna_keepout_center_mm = [18.80, pcb_pod_center_mm[1], pcb_top_z_mm + 1.2];
antenna_keepout_mm = [5.4, pcb_size_mm[1], 2.4];

board_rail_length_mm = 39.0;
board_rail_size_mm = [board_rail_length_mm, 0.9, 0.75];
// Rails support the board from below without intersecting its 1.0 mm outline.
board_rail_center_z_mm = pcb_center_z_mm - pcb_size_mm[2] / 2 - board_rail_size_mm[2] / 2;
board_end_stop_size_mm = [0.9, 1.0, 1.2];

lid_compression_pad_size_mm = [5.0, 1.2, 0.7];
lid_compression_pad_centers_mm = [
    [-13.5, 8.0, service_seam_height_mm - 0.35],
    [4.8, 8.0, service_seam_height_mm - 0.35]
];

dome_pocket_diameter_mm = 6.6;
dome_actuator_relief_diameter_mm = 3.2;
dome_pocket_depth_mm = 1.5;

battery_harness_channel_mm = [13.0, 3.2, 2.0];
battery_harness_channel_center_mm = [-6.7,
                                    battery_connector_center_mm[1],
                                    pcb_top_z_mm + 0.45];
battery_lift_window_mm = [22.0, 16.5, shell_height_mm - service_seam_height_mm - lid_top_skin_mm + 0.3];
battery_lift_window_center_z_mm = service_seam_height_mm + 0.85;
battery_service_loop_relief_mm = [12.0, 6.0, 1.6];
battery_service_loop_center_z_mm = pcb_top_z_mm + 0.20;

service_screw_centers_mm = [[-15.0, pcb_pod_center_mm[1] + 11.3],
                            [6.8, pcb_pod_center_mm[1] + 11.3]];

// Battery envelope remains a fit proxy until a real protected cell is chosen.
battery_keepout_mm = [20, 15, 4.2];
battery_pocket_clearance_mm = [0.35, 0.35, 0.15];
sensor_cavity_mm = [11.5, 13.5, 3.6];

// The cell is a separate lateral service pod.  It shares the service-side y
// datum but is separated from the PCB by an explicit x corridor.
battery_pod_center_mm = [-36.0, pcb_pod_center_mm[1], 0];
battery_service_screw_centers_mm = [[-42.0, battery_pod_center_mm[1] + 11.3],
                                    [-30.0, battery_pod_center_mm[1] + 11.3]];
all_service_screw_centers_mm = concat(service_screw_centers_mm,
                                      battery_service_screw_centers_mm);
battery_center_mm = [battery_pod_center_mm[0], battery_pod_center_mm[1], 4.7];
sensor_cavity_center_mm = [sensor_pcb_center_mm[0], sensor_pcb_center_mm[1], 2.6];
battery_top_z_mm = battery_center_mm[2] + battery_keepout_mm[2] / 2;
lid_component_clearance_floor_z_mm = pcb_top_z_mm + 1.7 -
                                     (shell_height_mm - pcb_top_z_mm - lid_top_skin_mm + 0.2) / 2;

// A removable gauge sets this datum. Confirm the matched optical assembly on a physical coupon.
pcb_bottom_z_mm = pcb_center_z_mm - pcb_size_mm[2] / 2;
glide_surface_z_mm = -rim_height_mm - glide_pad_thickness_mm;
sensor_optical_gap_mm = 2.9;
sensor_lens_datum_z_mm = glide_surface_z_mm + sensor_optical_gap_mm;
sensor_tunnel_length_mm = 5.0;

battery_pod_size_mm = [24.0, pcb_pod_size_mm[1], shell_height_mm];
battery_harness_bridge_center_mm = [
    (battery_connector_center_mm[0] + battery_pod_center_mm[0]) / 2,
    battery_connector_center_mm[1],
    pcb_top_z_mm + 0.45
];
battery_harness_bridge_mm = [
    abs(battery_connector_center_mm[0] - battery_pod_center_mm[0]),
    battery_harness_channel_mm[1],
    battery_harness_channel_mm[2]
];
battery_lift_window_top_z_mm = battery_lift_window_center_z_mm + battery_lift_window_mm[2] / 2;

screw_boss_outer_diameter_mm = 3.8;
screw_pilot_diameter_mm = 1.25;
screw_clearance_diameter_mm = 1.65;
screw_head_diameter_mm = 3.2;
screw_head_depth_mm = 1.2;

explode_gap_mm = 5.0;
show_exploded_view = true;
show_reference_solids = true;
show_reference_fasteners = true;

// Export modes:
//   shell
//   service_access_coupon
//   board_retention_coupon
//   lid_pad_coupon
//   battery_harness_coupon
//   service_lid_coupon
//   fit_coupons
//   collision_probe (must export an empty mesh)
//   contact_probe (must export a non-empty mesh)
export_mode = "shell";

coupon_base_thickness_mm = 1.2;
coupon_wall_mm = 1.8;
coupon_bridge_thickness_mm = 1.2;
coupon_board_slide_clearance_mm = 0.20;
service_lid_coupon_tongue_mm = [22.4, 12.4, lid_skirt_height_mm];

inner_diameter_mm = finger_circumference_mm / PI;
outer_diameter_mm = inner_diameter_mm + (2 * radial_thickness_mm);
inner_radius_mm = inner_diameter_mm / 2;
outer_radius_mm = outer_diameter_mm / 2;

seam_socket_outer_diameter_mm = outer_diameter_mm - 1.2;
seam_socket_inner_diameter_mm = seam_socket_outer_diameter_mm - 2.2;
male_skirt_outer_diameter_mm = seam_socket_outer_diameter_mm - seam_clearance_mm;
male_skirt_inner_diameter_mm = seam_socket_inner_diameter_mm + seam_clearance_mm;
lid_z_offset_mm = show_exploded_view ? explode_gap_mm : 0;

board_x_min_mm = pcb_pod_center_mm[0] - pcb_size_mm[0] / 2;
board_x_max_mm = pcb_pod_center_mm[0] + pcb_size_mm[0] / 2;
board_y_min_mm = pcb_pod_center_mm[1] - pcb_size_mm[1] / 2;
board_y_max_mm = pcb_pod_center_mm[1] + pcb_size_mm[1] / 2;
battery_x_min_mm = battery_center_mm[0] - battery_keepout_mm[0] / 2;
battery_x_max_mm = battery_center_mm[0] + battery_keepout_mm[0] / 2;
battery_y_min_mm = battery_center_mm[1] - battery_keepout_mm[1] / 2;
battery_y_max_mm = battery_center_mm[1] + battery_keepout_mm[1] / 2;
pod_inner_y_min_mm = pcb_pod_center_mm[1] - pcb_pod_size_mm[1] / 2;
module_x_min_mm = module_center_mm[0] - module_keepout_mm[0] / 2;
module_x_max_mm = module_center_mm[0] + module_keepout_mm[0] / 2;
module_y_min_mm = module_center_mm[1] - module_keepout_mm[1] / 2;
module_y_max_mm = module_center_mm[1] + module_keepout_mm[1] / 2;
module_z_min_mm = module_center_mm[2] - module_keepout_mm[2] / 2;
module_z_max_mm = module_center_mm[2] + module_keepout_mm[2] / 2;
antenna_x_min_mm = antenna_keepout_center_mm[0] - antenna_keepout_mm[0] / 2;
antenna_x_max_mm = antenna_keepout_center_mm[0] + antenna_keepout_mm[0] / 2;
antenna_y_max_mm = antenna_keepout_center_mm[1] + antenna_keepout_mm[1] / 2;

assert(sensor_angle_deg > 0 && sensor_angle_deg < 60);
assert(outer_diameter_mm > inner_diameter_mm + 2);
assert(shell_height_mm > service_seam_height_mm + 1.0);
assert(service_seam_height_mm > 3.8);
assert(lid_top_skin_mm > 0.8);
assert(band_width_mm >= sensor_aperture_diameter_mm + 6);
assert(seam_socket_inner_diameter_mm > inner_diameter_mm + 2.0);
assert(pcb_pod_size_mm[1] >= pcb_size_mm[1] + 2.0);
assert(pcb_pod_size_mm[0] >= pcb_size_mm[0] + 2.0);
assert(service_screw_centers_mm[0][1] + screw_boss_outer_diameter_mm / 2 <=
       pcb_pod_center_mm[1] + pcb_pod_size_mm[1] / 2 + 0.001);
assert(service_screw_centers_mm[0][1] - screw_boss_outer_diameter_mm / 2 > board_y_max_mm + pcb_clearance_mm);
assert(board_y_min_mm > inner_radius_mm + pcb_clearance_mm,
       str("PCB crosses finger passage: board y-min=", board_y_min_mm,
           " inner radius=", inner_radius_mm));
assert(pod_inner_y_min_mm > inner_radius_mm + pcb_clearance_mm,
       str("electronics pod crosses finger passage: pod y-min=", pod_inner_y_min_mm));
assert(module_x_min_mm >= board_x_min_mm - 0.001 && module_x_max_mm <= board_x_max_mm + 0.001,
       "populated radio body must stay within the PCB x envelope");
assert(module_y_min_mm > inner_radius_mm + pcb_clearance_mm,
       "populated radio body must clear the finger passage");
assert(module_y_max_mm <= pcb_pod_center_mm[1] + pcb_pod_size_mm[1] / 2 - pcb_clearance_mm);
assert(module_z_min_mm >= pcb_top_z_mm - 0.001 && module_z_max_mm <= shell_height_mm - lid_top_skin_mm + 0.001,
       "populated radio body must fit between PCB and lid roof");
assert(abs(antenna_x_min_mm - 16.1) < 0.001 && abs(antenna_x_max_mm - 21.5) < 0.001,
       "antenna copper void must match final KiCad x interval");
assert(antenna_y_max_mm <= board_y_max_mm + 0.001,
       "antenna copper void must span the complete board width");
assert(service_screw_centers_mm[0][1] - screw_boss_outer_diameter_mm / 2 > antenna_y_max_mm + 0.001,
       "electronics-pod screws must clear the full-width antenna void");
assert(battery_x_max_mm + pcb_clearance_mm < board_x_min_mm,
       str("battery/PCB x overlap: battery max=", battery_x_max_mm,
           " board min=", board_x_min_mm));
assert(battery_y_min_mm > inner_radius_mm + wall_thickness_mm - 0.001);
assert(battery_x_max_mm < pcb_pod_center_mm[0] - pcb_pod_size_mm[0] / 2 - pcb_clearance_mm,
       "battery pod must be separate from electronics pod");
assert(battery_service_screw_centers_mm[0][0] - screw_boss_outer_diameter_mm / 2 >=
       battery_pod_center_mm[0] - battery_pod_size_mm[0] / 2);
assert(battery_service_screw_centers_mm[1][0] + screw_boss_outer_diameter_mm / 2 <=
       battery_pod_center_mm[0] + battery_pod_size_mm[0] / 2);
assert(battery_service_screw_centers_mm[0][1] - screw_boss_outer_diameter_mm / 2 >
       battery_y_max_mm + pcb_clearance_mm);
assert(battery_x_min_mm - (battery_pod_center_mm[0] - battery_pod_size_mm[0] / 2) >= 1.5);
assert((battery_pod_center_mm[0] + battery_pod_size_mm[0] / 2) - battery_x_max_mm >= 1.5);
assert(battery_y_min_mm - (battery_pod_center_mm[1] - battery_pod_size_mm[1] / 2) >= 1.5);
assert((battery_pod_center_mm[1] + battery_pod_size_mm[1] / 2) - battery_y_max_mm >= 1.5);
assert(battery_center_mm[0] - battery_lift_window_mm[0] / 2 >=
       battery_pod_center_mm[0] - battery_pod_size_mm[0] / 2 + 0.5);
assert(battery_center_mm[0] + battery_lift_window_mm[0] / 2 <=
       battery_pod_center_mm[0] + battery_pod_size_mm[0] / 2 - 0.5);
assert(battery_center_mm[1] - battery_lift_window_mm[1] / 2 >=
       battery_pod_center_mm[1] - battery_pod_size_mm[1] / 2 + 0.5);
assert(battery_center_mm[1] + battery_lift_window_mm[1] / 2 <=
       battery_pod_center_mm[1] + battery_pod_size_mm[1] / 2 - 0.5);
assert(battery_harness_bridge_center_mm[0] - battery_harness_bridge_mm[0] / 2 <=
       battery_pod_center_mm[0] + battery_pod_size_mm[0] / 2);
assert(battery_harness_bridge_center_mm[0] + battery_harness_bridge_mm[0] / 2 >=
       pcb_pod_center_mm[0] - pcb_pod_size_mm[0] / 2);
assert(22.2 - board_end_stop_size_mm[0] / 2 > board_x_max_mm + 0.001,
       "positive board-end-stop gap required at +X end");
assert(-22.2 + board_end_stop_size_mm[0] / 2 < board_x_min_mm - 0.001,
       "positive board-end-stop gap required at -X end");
assert(lid_component_clearance_floor_z_mm - pcb_top_z_mm >= pcb_clearance_mm - 0.001,
       str("PCB top-to-lid clearance is below ", pcb_clearance_mm, " mm"));
assert(battery_top_z_mm <= shell_height_mm - lid_top_skin_mm + 0.4,
       "battery envelope reaches the lid skin without service-window clearance");
assert(battery_lift_window_top_z_mm - battery_top_z_mm >= 0.099,
       "battery top lacks positive clearance to the lid lift window");
assert(sensor_optical_gap_mm >= 2.4 && sensor_optical_gap_mm <= 3.2,
       str("sensor optical gap outside 2.4-3.2 mm: ", sensor_optical_gap_mm));
assert(shell_height_mm - glide_surface_z_mm <= 10.0,
       str("finger-to-surface height exceeds 10 mm: ", shell_height_mm - glide_surface_z_mm));
assert(pcb_bottom_z_mm > glide_surface_z_mm + 2.0);
assert(board_rail_center_z_mm + board_rail_size_mm[2] / 2 <= pcb_bottom_z_mm + 0.001,
       "board rail top must not intersect PCB thickness");
assert(abs(sensor_pcb_center_mm[0] - (-4.0)) < 0.01 &&
       abs(sensor_pcb_center_mm[1] - pcb_pod_center_mm[1]) < 0.01,
       "sensor datum must remain at KiCad U2 board-local (-4, 0)");
assert(service_access_opening_mm[1] > 7.5);
assert(service_screw_centers_mm[1][0] < antenna_keepout_center_mm[0] - antenna_keepout_mm[0] / 2);
assert(export_mode == "shell" ||
       export_mode == "service_access_coupon" ||
       export_mode == "board_retention_coupon" ||
       export_mode == "lid_pad_coupon" ||
       export_mode == "battery_harness_coupon" ||
       export_mode == "service_lid_coupon" ||
       export_mode == "fit_coupons" ||
       export_mode == "collision_probe" ||
       export_mode == "contact_probe",
       str("unknown export_mode: ", export_mode));

// Slice the full assembly, including both lateral pods, at the service seam.
module clip_height(bottom, top) {
    x_min = min(-outer_diameter_mm / 2, battery_pod_center_mm[0] - battery_pod_size_mm[0] / 2) - 1;
    x_max = max(outer_diameter_mm / 2, pcb_pod_center_mm[0] + pcb_pod_size_mm[0] / 2) + 1;
    y_min = -outer_diameter_mm / 2 - 1;
    y_max = max(outer_diameter_mm / 2, pcb_pod_center_mm[1] + pcb_pod_size_mm[1] / 2) + 1;
    intersection() {
        children();
        translate([x_min, y_min, bottom]) cube([x_max - x_min, y_max - y_min, top - bottom]);
    }
}

module clipped_to_lower_segment() {
    clip_height(glide_surface_z_mm - 0.1, service_seam_height_mm) children();
}

module clipped_to_upper_segment() {
    clip_height(service_seam_height_mm, shell_height_mm + 0.1) children();
}

module outer_shell_envelope() {
    union() {
        ring_body();
        board_pod_envelope();
        battery_pod_envelope();
        battery_pod_bridge_envelope();
        lower_rim();
        glide_pad_seats();
    }
}

module ring_body() {
    cylinder(h = shell_height_mm, d = outer_diameter_mm);
}

module rounded_box(size, radius) {
    hull() {
        for (x = [-size[0] / 2 + radius, size[0] / 2 - radius])
            for (y = [-size[1] / 2 + radius, size[1] / 2 - radius])
                translate([x, y, 0])
                    cylinder(h = size[2], r = radius);
    }
}

module board_pod_envelope() {
    translate(pcb_pod_center_mm)
        rounded_box(pcb_pod_size_mm, pcb_pod_corner_radius_mm);
}

module battery_pod_envelope() {
    translate(battery_pod_center_mm)
        rounded_box(battery_pod_size_mm, pcb_pod_corner_radius_mm);
}

module battery_pod_bridge_envelope() {
    // A narrow service-side bridge keeps the separate pods mechanically
    // joined while remaining wholly outside the finger passage.
    translate(battery_harness_bridge_center_mm)
        cube([battery_harness_bridge_mm[0] + 0.8,
              battery_harness_bridge_mm[1] + 3.8,
              shell_height_mm],
             center = true);
}

module lower_rim() {
    translate([0, 0, -rim_height_mm])
        difference() {
            cylinder(h = rim_height_mm, d = outer_diameter_mm - 1.2);
            cylinder(h = rim_height_mm + 0.2,
                     d = inner_diameter_mm + (2 * rim_radial_width_mm));
        }
}

module finger_opening() {
    translate([0, 0, -rim_height_mm - 0.5])
        cylinder(h = shell_height_mm + rim_height_mm + 1.0,
                 d = inner_diameter_mm);
}

module battery_pocket() {
    translate(battery_center_mm)
        cube(battery_keepout_mm + 2 * battery_pocket_clearance_mm, center = true);
}

module sensor_cavity() {
    translate(sensor_cavity_center_mm)
        cube(sensor_cavity_mm, center = true);
}

module module_pocket() {
    translate(module_center_mm)
        cube(module_keepout_mm + [2 * pcb_clearance_mm, 2 * pcb_clearance_mm, 0.3], center = true);
}

module pcb_tray_clearance() {
    translate(pcb_tray_center_mm)
        cube(pcb_tray_clearance_mm, center = true);
}

module top_component_clearance() {
    translate([pcb_pod_center_mm[0], pcb_pod_center_mm[1], pcb_top_z_mm + 1.7])
        cube([pcb_size_mm[0] + 0.8,
              pcb_size_mm[1] - 3.0,
              shell_height_mm - pcb_top_z_mm - lid_top_skin_mm + 0.2],
             center = true);
}

module service_access_opening() {
    translate(service_access_center_mm)
        cube(service_access_opening_mm, center = true);
}

module service_fixture_keepout() {
    translate([service_access_center_mm[0] + 1.2,
               service_access_center_mm[1],
               service_access_center_mm[2] - 0.1])
        cube(service_fixture_keepout_mm, center = true);
}

module dome_switch_pocket() {
    translate(dome_switch_center_mm)
        cylinder(h = dome_pocket_depth_mm,
                 d = dome_pocket_diameter_mm,
                 center = true);
}

module dome_actuator_relief() {
    translate([dome_switch_center_mm[0],
               dome_switch_center_mm[1],
               shell_height_mm - 0.35])
        cylinder(h = 0.8, d = dome_actuator_relief_diameter_mm, center = true);
}

module battery_harness_channel() {
    union() {
        translate(battery_harness_channel_center_mm)
            cube(battery_harness_channel_mm, center = true);
        translate(battery_harness_bridge_center_mm)
            cube(battery_harness_bridge_mm, center = true);
    }
}

module battery_service_loop_relief() {
    translate([battery_pod_center_mm[0] + 1.3,
               battery_pod_center_mm[1] - 2.0,
               battery_service_loop_center_z_mm])
        cube(battery_service_loop_relief_mm, center = true);
}

module battery_lift_window() {
    translate([battery_pod_center_mm[0],
               battery_pod_center_mm[1],
               battery_lift_window_center_z_mm])
        cube(battery_lift_window_mm, center = true);
}

module sensor_tunnel() {
    // The optical axis is perpendicular to the flat PCB and the glide plane.
    translate([sensor_pcb_center_mm[0], sensor_pcb_center_mm[1],
               sensor_lens_datum_z_mm - sensor_tunnel_length_mm / 2])
        cylinder(h = sensor_tunnel_length_mm, d = sensor_aperture_diameter_mm, center = true);
}

// The projected cell, optical axis and dome load fall inside these four supports.
glide_pad_centers_mm = [[battery_pod_center_mm[0] - 7, pcb_pod_center_mm[1] - 5],
                        [battery_pod_center_mm[0] - 7, pcb_pod_center_mm[1] + 8],
                        [17, pcb_pod_center_mm[1] + 8],
                        [0, -outer_diameter_mm / 2 + 2.4]];
module glide_pad_seats() {
    for (p = glide_pad_centers_mm)
        translate([p[0], p[1], -rim_height_mm])
            cylinder(h = rim_height_mm + 0.3, d = 4.0);
}

module installed_glide_pad_solids() {
    for (p = glide_pad_centers_mm)
        translate([p[0], p[1], glide_surface_z_mm])
            cylinder(h = glide_pad_thickness_mm, d = 3.2);
}

module seam_socket() {
    translate([0, 0, service_seam_height_mm - lid_skirt_height_mm - 0.02])
        difference() {
            cylinder(h = lid_skirt_height_mm + 0.04,
                     d = seam_socket_outer_diameter_mm + seam_clearance_mm);
            cylinder(h = lid_skirt_height_mm + 0.08,
                     d = seam_socket_inner_diameter_mm - seam_clearance_mm);
        }
}

module lid_skirt() {
    translate([0, 0, service_seam_height_mm - lid_skirt_height_mm])
        difference() {
            cylinder(h = lid_skirt_height_mm, d = male_skirt_outer_diameter_mm);
            cylinder(h = lid_skirt_height_mm + 0.04,
                     d = male_skirt_inner_diameter_mm);
        }
}

module service_pry_slot() {
    translate([0,
               -outer_radius_mm + service_pry_slot_mm[1] / 2,
               service_seam_height_mm - service_pry_slot_mm[2] / 2])
        cube(service_pry_slot_mm, center = true);
}

module lower_clearance_cuts() {
    union() {
        finger_opening();
        battery_pocket();
        sensor_cavity();
        module_pocket();
        pcb_tray_clearance();
        service_access_opening();
        service_fixture_keepout();
        battery_harness_channel();
        battery_service_loop_relief();
        sensor_tunnel();
        seam_socket();
        service_pry_slot();
    }
}

module lid_service_clearance() {
    union() {
        finger_opening();
        top_component_clearance();
        battery_lift_window();
        service_access_opening();
        dome_switch_pocket();
        dome_actuator_relief();
    }
}

module screw_boss_pair() {
    for (p = all_service_screw_centers_mm) {
        translate([p[0], p[1], service_seam_height_mm - 1.8])
            difference() {
                cylinder(h = 3.2, d = screw_boss_outer_diameter_mm);
                translate([0, 0, -0.05])
                    cylinder(h = 3.3, d = screw_pilot_diameter_mm);
            }
    }
}

module screw_boss_gussets() {
    for (p = all_service_screw_centers_mm) {
        hull() {
            translate([p[0], p[1], service_seam_height_mm - 1.8])
                cylinder(h = 1.0, d = screw_boss_outer_diameter_mm);

            translate([p[0], p[1] + 1.9, service_seam_height_mm - 1.8])
                cylinder(h = 1.0, d = 1.4);
        }
    }
}

module lid_fastener_holes() {
    for (p = all_service_screw_centers_mm) {
        translate([p[0], p[1], service_seam_height_mm - lid_skirt_height_mm - 0.05])
            cylinder(h = shell_height_mm - service_seam_height_mm + lid_skirt_height_mm + 0.15,
                     d = screw_clearance_diameter_mm);

        translate([p[0], p[1], shell_height_mm - screw_head_depth_mm])
            cylinder(h = screw_head_depth_mm + 0.05,
                     d = screw_head_diameter_mm);
    }
}

module board_support_rails() {
    for (y_sign = [-1, 1]) {
        translate([pcb_pod_center_mm[0],
                   pcb_pod_center_mm[1] + y_sign * (pcb_size_mm[1] / 2 + 0.15),
                   board_rail_center_z_mm])
            cube(board_rail_size_mm, center = true);
    }
}

module board_end_stops() {
    for (x_pos = [-22.2, 22.2]) {
        translate([pcb_pod_center_mm[0], pcb_pod_center_mm[1], pcb_center_z_mm])
            translate([x_pos, 0, 0])
            cube(board_end_stop_size_mm, center = true);
    }
}

module lid_board_compression_pads() {
    for (p = lid_compression_pad_centers_mm) {
        translate([p[0] + pcb_pod_center_mm[0],
                   p[1] + pcb_pod_center_mm[1],
                   p[2]])
            cube(lid_compression_pad_size_mm, center = true);
    }
}

module coupon_base(size) {
    translate([0, 0, coupon_base_thickness_mm / 2])
        cube([size[0], size[1], coupon_base_thickness_mm], center = true);
}

module service_access_fit_coupon() {
    wall_height = service_access_opening_mm[2] + 2.2;
    wall_center_z = coupon_base_thickness_mm + wall_height / 2;
    board_slot_center_z = coupon_base_thickness_mm + board_rail_size_mm[2] +
                          (pcb_size_mm[2] + coupon_board_slide_clearance_mm) / 2;

    difference() {
        union() {
            coupon_base([18.0, service_access_opening_mm[1] + 7.0]);

            translate([-6.0, 0, wall_center_z])
                cube([coupon_wall_mm,
                      service_access_opening_mm[1] + 5.0,
                      wall_height],
                     center = true);

            for (y_sign = [-1, 1]) {
                translate([1.8,
                           y_sign * (pcb_size_mm[1] / 2 + 0.15),
                           coupon_base_thickness_mm + board_rail_size_mm[2] / 2])
                    cube([13.0, board_rail_size_mm[1], board_rail_size_mm[2]],
                         center = true);
            }
        }

        translate([-6.0, 0, wall_center_z])
            cube([coupon_wall_mm + 0.4,
                  service_access_opening_mm[1],
                  service_access_opening_mm[2]],
                 center = true);

        translate([1.8, 0, board_slot_center_z])
            cube([13.2,
                  pcb_size_mm[1] + (2 * pcb_clearance_mm),
                  pcb_size_mm[2] + coupon_board_slide_clearance_mm],
                 center = true);
    }
}

module board_retention_fit_coupon() {
    base_size = [pcb_size_mm[0] + 5.0, pcb_size_mm[1] + 5.0];
    rail_z = coupon_base_thickness_mm + board_rail_size_mm[2] / 2;
    stop_z = coupon_base_thickness_mm + board_rail_size_mm[2] +
             pcb_size_mm[2] / 2;

    union() {
        coupon_base(base_size);

        for (y_sign = [-1, 1]) {
            translate([0,
                       y_sign * (pcb_size_mm[1] / 2 + 0.15),
                       rail_z])
                cube(board_rail_size_mm, center = true);
        }

        for (x_pos = [-22.2, 22.2]) {
            translate([x_pos, 0, stop_z])
                cube(board_end_stop_size_mm, center = true);
        }
    }
}

module lid_compression_pad_fit_coupon() {
    coupon_size = [24.0, pcb_size_mm[1] + 6.0];
    wall_height = board_rail_size_mm[2] + pcb_size_mm[2] +
                  lid_compression_pad_size_mm[2] + 0.55;
    wall_z = coupon_base_thickness_mm + wall_height / 2;
    bridge_z = coupon_base_thickness_mm + wall_height +
               coupon_bridge_thickness_mm / 2;
    pad_z = coupon_base_thickness_mm + board_rail_size_mm[2] +
            pcb_size_mm[2] + 0.20 + lid_compression_pad_size_mm[2] / 2;

    union() {
        coupon_base(coupon_size);

        for (y_sign = [-1, 1]) {
            translate([0,
                       y_sign * (pcb_size_mm[1] / 2 + 2.1),
                       wall_z])
                cube([coupon_size[0], coupon_wall_mm, wall_height], center = true);
        }

        translate([0, 0, bridge_z])
            cube([coupon_size[0],
                  coupon_size[1],
                  coupon_bridge_thickness_mm],
                 center = true);

        for (x_pos = [-6.0, 6.0]) {
            translate([x_pos, pcb_size_mm[1] / 2 - 0.55, pad_z])
                cube(lid_compression_pad_size_mm, center = true);
        }
    }
}

module battery_harness_service_loop_fit_coupon() {
    block_size = [26.0, 18.0, 5.2];

    difference() {
        translate([0, 0, block_size[2] / 2])
            cube(block_size, center = true);

        translate([-5.4, -4.4, 2.8])
            cube([battery_harness_channel_mm[0],
                  battery_harness_channel_mm[1],
                  battery_harness_channel_mm[2] + 0.2],
                 center = true);

        translate([0.0, 3.9, 3.0])
            cube([battery_service_loop_relief_mm[0],
                  battery_service_loop_relief_mm[1],
                  battery_service_loop_relief_mm[2] + 0.2],
                 center = true);

        translate([-8.9, -5.6, 2.45])
            cube([5.2, 3.4, 1.7], center = true);

        translate([-0.8, 0.6, 4.9])
            cube([battery_lift_window_mm[0],
                  battery_lift_window_mm[1],
                  1.7],
                 center = true);
    }
}

module service_lid_socket_coupon() {
    socket_size = [28.0, 18.0, service_seam_height_mm];

    difference() {
        translate([0, 0, socket_size[2] / 2])
            cube(socket_size, center = true);

        translate([0, 0, service_seam_height_mm - lid_skirt_height_mm / 2])
            cube([service_lid_coupon_tongue_mm[0] + (2 * seam_clearance_mm),
                  service_lid_coupon_tongue_mm[1] + (2 * seam_clearance_mm),
                  lid_skirt_height_mm + 0.08],
                 center = true);

        translate([0,
                   -socket_size[1] / 2 + service_pry_slot_mm[1] / 2,
                   service_seam_height_mm - service_pry_slot_mm[2] / 2])
            cube(service_pry_slot_mm, center = true);

        for (x_pos = [-8.0, 8.0]) {
            translate([x_pos, 5.2, -0.05])
                cylinder(h = socket_size[2] + 0.1, d = screw_pilot_diameter_mm);
        }
    }
}

module service_lid_tongue_coupon() {
    lid_size = [28.0, 18.0, lid_top_skin_mm];

    difference() {
        union() {
            translate([0, 0, lid_top_skin_mm / 2])
                cube(lid_size, center = true);

            translate([0, 0, lid_top_skin_mm + service_lid_coupon_tongue_mm[2] / 2])
                cube(service_lid_coupon_tongue_mm, center = true);
        }

        for (x_pos = [-8.0, 8.0]) {
            translate([x_pos, 5.2, -0.05])
                cylinder(h = lid_top_skin_mm + service_lid_coupon_tongue_mm[2] + 0.1,
                         d = screw_clearance_diameter_mm);
        }
    }
}

module service_lid_removal_fit_coupon() {
    translate([-18.0, 0, 0])
        service_lid_socket_coupon();

    translate([18.0, 0, 0])
        service_lid_tongue_coupon();
}

module fit_coupon_set() {
    translate([-34.0, 20.0, 0])
        service_access_fit_coupon();

    translate([34.0, 20.0, 0])
        board_retention_fit_coupon();

    translate([-34.0, -20.0, 0])
        lid_compression_pad_fit_coupon();

    translate([34.0, -20.0, 0])
        battery_harness_service_loop_fit_coupon();

    translate([0, -54.0, 0])
        service_lid_removal_fit_coupon();
}

module pcb_reference_volume() {
    translate(pcb_tray_center_mm)
        cube(pcb_size_mm, center = true);
}

module battery_reference_volume() {
    translate(battery_center_mm)
        cube(battery_keepout_mm, center = true);
}

module module_reference_volume() {
    translate(module_center_mm)
        cube(module_keepout_mm, center = true);
}

module finger_passage_volume() {
    translate([0, 0, -rim_height_mm - 0.5])
        cylinder(h = shell_height_mm + rim_height_mm + 1.0,
                 d = inner_diameter_mm);
}

module rail_reference_volume() {
    board_support_rails();
    board_end_stops();
}

module collision_probe() {
    // Each intersection must be empty. This is a CSG mesh regression against
    // the posed solids, not only a scalar bounding-box assertion.
    union() {
        // A missing wall or seat also fails this probe, including an over-short seam clipping box.
        difference() {
            union() {
                for (p = glide_pad_centers_mm)
                    translate([p[0], p[1], -rim_height_mm + 0.02]) cylinder(h = 0.10, d = 3.0);
                translate([battery_pod_center_mm[0] - 11.4, battery_pod_center_mm[1], 3]) cube(0.2);
            }
            lower_shell();
        }
        difference() {
            translate([battery_pod_center_mm[0] - 11.4, battery_pod_center_mm[1], 6]) cube(0.2);
            service_lid();
        }
        intersection() { pcb_reference_volume(); battery_reference_volume(); }
        intersection() { battery_reference_volume(); lower_shell(); }
        intersection() { module_reference_volume(); lower_shell(); }
        intersection() { pcb_reference_volume(); lower_shell(); }
        intersection() { module_reference_volume(); battery_reference_volume(); }
        intersection() { pcb_reference_volume(); finger_passage_volume(); }
        intersection() { battery_reference_volume(); finger_passage_volume(); }
        intersection() { module_reference_volume(); finger_passage_volume(); }
        intersection() { pcb_reference_volume(); rail_reference_volume(); }
        intersection() { battery_reference_volume(); service_lid(); }
        intersection() { module_reference_volume(); service_lid(); }
    }
}

module contact_probe() {
    // These positive solids must survive CSG evaluation: the installed pad
    // exposure and the joined service bridge are intentional geometry.
    union() {
        installed_glide_pad_solids();
        battery_pod_bridge_envelope();
    }
}

module lower_shell() {
    union() {
        difference() {
            clipped_to_lower_segment()
                outer_shell_envelope();
            lower_clearance_cuts();
        }

        board_support_rails();
        board_end_stops();
        screw_boss_pair();
        screw_boss_gussets();
    }
}

module service_lid() {
    union() {
        difference() {
            union() {
                clipped_to_upper_segment()
                    outer_shell_envelope();
                lid_skirt();
            }

            lid_service_clearance();
            lid_fastener_holes();
        }

        // Small molded pads only press board edge keep-out zones; they are not
        // snap hooks and should be shaved or omitted if assembly force is high.
        lid_board_compression_pads();
    }
}

module reference_solids() {
    %translate(pcb_tray_center_mm)
        color("limegreen") cube(pcb_size_mm, center = true);

    %translate(service_access_center_mm)
        color("silver") cube(service_fixture_keepout_mm, center = true);

    %translate(battery_center_mm)
        color("gold") cube(battery_keepout_mm, center = true);

    %translate(sensor_cavity_center_mm)
        color("firebrick") cube(sensor_cavity_mm, center = true);

    %translate(module_center_mm)
        color("deepskyblue") cube(module_keepout_mm, center = true);

    %translate(dome_switch_center_mm)
        color("orange") cylinder(h = 0.5, d = 5.3, center = true);

    %translate(battery_connector_center_mm)
        color("darkorange") cube([3.2, 2.9, 0.8], center = true);

    %translate(antenna_keepout_center_mm)
        color("violet") cube(antenna_keepout_mm, center = true);
}

module reference_fasteners() {
    for (p = all_service_screw_centers_mm) {
        %translate([p[0], p[1], shell_height_mm - 4.6 + lid_z_offset_mm])
            color("silver")
                cylinder(h = 4.6, d = screw_clearance_diameter_mm * 0.95);
    }
}

module shell_packet() {
    lower_shell();

    translate([0, 0, lid_z_offset_mm])
        service_lid();

    if (show_reference_solids) {
        reference_solids();
    }

    if (show_reference_fasteners) {
        reference_fasteners();
    }
}

if (export_mode == "shell") {
    shell_packet();
} else if (export_mode == "service_access_coupon") {
    service_access_fit_coupon();
} else if (export_mode == "board_retention_coupon") {
    board_retention_fit_coupon();
} else if (export_mode == "lid_pad_coupon") {
    lid_compression_pad_fit_coupon();
} else if (export_mode == "battery_harness_coupon") {
    battery_harness_service_loop_fit_coupon();
} else if (export_mode == "service_lid_coupon") {
    service_lid_removal_fit_coupon();
} else if (export_mode == "fit_coupons") {
    fit_coupon_set();
} else if (export_mode == "collision_probe") {
    collision_probe();
} else if (export_mode == "contact_probe") {
    contact_probe();
}
