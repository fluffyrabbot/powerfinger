// SPDX-License-Identifier: CERN-OHL-S-2.0
//
// Serviceable first-board shell packet for the active optical ring lane.
// This is an honest mechanical packet, not a validated enclosure.

$fn = 96;

finger_circumference_mm = 60;
sensor_angle_deg = 30;
band_width_mm = 18;
shell_height_mm = 8.0;
wall_thickness_mm = 1.6;
radial_thickness_mm = 7.0;
rim_height_mm = 2.8;
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
pcb_tray_clearance_mm = [pcb_size_mm[0] + (2 * pcb_clearance_mm),
                         pcb_size_mm[1] + (2 * pcb_clearance_mm),
                         pcb_size_mm[2] + 0.6];
pcb_tray_center_mm = [0, 0, pcb_center_z_mm];

pcb_pod_margin_mm = [1.8, 1.5, 0];
pcb_pod_size_mm = [pcb_size_mm[0] + (2 * pcb_pod_margin_mm[0]),
                   pcb_size_mm[1] + (2 * pcb_pod_margin_mm[1]),
                   shell_height_mm];
pcb_pod_corner_radius_mm = 2.2;

service_access_center_mm = [-17.85, 0.0, pcb_top_z_mm + 0.70];
service_access_opening_mm = [2.6, 8.2, 1.8];
service_fixture_keepout_mm = [4.2, 7.4, 1.8];

sensor_pcb_center_mm = [-4.00, 0.0, 1.7];
dome_switch_center_mm = [-6.15, -4.22, pcb_top_z_mm + 0.45];
battery_connector_center_mm = [-11.90, -5.75, pcb_top_z_mm + 0.35];
antenna_keepout_center_mm = [17.575, 0.0, pcb_top_z_mm + 1.2];
antenna_keepout_mm = [7.85, 16.7, 2.4];

board_rail_length_mm = 34.0;
board_rail_size_mm = [board_rail_length_mm, 0.9, 0.75];
board_rail_center_z_mm = pcb_center_z_mm - pcb_size_mm[2] / 2 - 0.30;
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
battery_harness_channel_center_mm = [-6.7, -3.9, pcb_top_z_mm + 0.45];
battery_lift_window_mm = [22.0, 16.5, shell_height_mm - service_seam_height_mm - lid_top_skin_mm + 0.1];
battery_lift_window_center_mm = [-4.6, 0.8, service_seam_height_mm + 0.85];
battery_service_loop_relief_mm = [12.0, 6.0, 1.6];
battery_service_loop_center_mm = [-1.3, 4.6, pcb_top_z_mm + 0.20];

service_screw_centers_mm = [[-15.0, 6.9], [6.8, 6.9]];

// Battery envelope remains a fit proxy until a real protected cell is chosen.
battery_keepout_mm = [20, 15, 4.2];
sensor_cavity_mm = [11.5, 8.0, 2.6];
module_keepout_mm = [14, 12, 3.2];

battery_center_mm = [-4.1, 0.8, 4.7];
sensor_cavity_center_mm = [sensor_pcb_center_mm[0], sensor_pcb_center_mm[1], 2.6];
module_center_mm = [5.3, 0.0, 3.2];

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

assert(sensor_angle_deg > 0 && sensor_angle_deg < 60);
assert(outer_diameter_mm > inner_diameter_mm + 2);
assert(shell_height_mm > service_seam_height_mm + 1.0);
assert(service_seam_height_mm > 3.8);
assert(lid_top_skin_mm > 0.8);
assert(band_width_mm >= sensor_aperture_diameter_mm + 6);
assert(seam_socket_inner_diameter_mm > inner_diameter_mm + 2.0);
assert(pcb_pod_size_mm[1] >= pcb_size_mm[1] + 2.0);
assert(service_access_opening_mm[1] > 7.5);
assert(service_screw_centers_mm[1][0] < antenna_keepout_center_mm[0] - antenna_keepout_mm[0] / 2);
assert(export_mode == "shell" ||
       export_mode == "service_access_coupon" ||
       export_mode == "board_retention_coupon" ||
       export_mode == "lid_pad_coupon" ||
       export_mode == "battery_harness_coupon" ||
       export_mode == "service_lid_coupon" ||
       export_mode == "fit_coupons",
       str("unknown export_mode: ", export_mode));

module clipped_to_lower_segment() {
    intersection() {
        children();
        translate([-outer_diameter_mm,
                   -outer_diameter_mm,
                   -rim_height_mm - 0.2])
            cube([2 * outer_diameter_mm,
                  2 * outer_diameter_mm,
                  service_seam_height_mm + rim_height_mm + 0.2]);
    }
}

module clipped_to_upper_segment() {
    intersection() {
        children();
        translate([-outer_diameter_mm,
                   -outer_diameter_mm,
                   service_seam_height_mm])
            cube([2 * outer_diameter_mm,
                  2 * outer_diameter_mm,
                  shell_height_mm - service_seam_height_mm + 0.2]);
    }
}

module outer_shell_envelope() {
    union() {
        ring_body();
        board_pod_envelope();
        lower_rim();
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
    rounded_box(pcb_pod_size_mm, pcb_pod_corner_radius_mm);
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
        cube(battery_keepout_mm, center = true);
}

module sensor_cavity() {
    translate(sensor_cavity_center_mm)
        cube(sensor_cavity_mm, center = true);
}

module module_pocket() {
    translate(module_center_mm)
        cube(module_keepout_mm, center = true);
}

module pcb_tray_clearance() {
    translate(pcb_tray_center_mm)
        cube(pcb_tray_clearance_mm, center = true);
}

module top_component_clearance() {
    translate([0, 0, pcb_top_z_mm + 1.7])
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
    translate(battery_harness_channel_center_mm)
        cube(battery_harness_channel_mm, center = true);
}

module battery_service_loop_relief() {
    translate(battery_service_loop_center_mm)
        cube(battery_service_loop_relief_mm, center = true);
}

module battery_lift_window() {
    translate(battery_lift_window_center_mm)
        cube(battery_lift_window_mm, center = true);
}

module sensor_tunnel() {
    translate([0, sensor_platform_offset_mm, 1.2])
        rotate([sensor_angle_deg, 0, 0])
            cylinder(h = 8.4, d = sensor_aperture_diameter_mm, center = true);
}

module glide_pad_pockets() {
    for (a = [45, 135, 225, 315]) {
        rotate([0, 0, a])
            translate([outer_diameter_mm / 2 - 2.4, 0, -rim_height_mm + 0.15])
                cylinder(h = glide_pad_thickness_mm + 0.2, d = 3.2);
    }
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
        glide_pad_pockets();
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
    for (p = service_screw_centers_mm) {
        translate([p[0], p[1], service_seam_height_mm - 1.8])
            difference() {
                cylinder(h = 3.2, d = screw_boss_outer_diameter_mm);
                translate([0, 0, -0.05])
                    cylinder(h = 3.3, d = screw_pilot_diameter_mm);
            }
    }
}

module screw_boss_gussets() {
    for (p = service_screw_centers_mm) {
        hull() {
            translate([p[0], p[1], service_seam_height_mm - 1.8])
                cylinder(h = 1.0, d = screw_boss_outer_diameter_mm);

            translate([p[0], p[1] + 1.9, service_seam_height_mm - 1.8])
                cylinder(h = 1.0, d = 1.4);
        }
    }
}

module lid_fastener_holes() {
    for (p = service_screw_centers_mm) {
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
        translate([0,
                   y_sign * (pcb_size_mm[1] / 2 + 0.15),
                   board_rail_center_z_mm])
            cube(board_rail_size_mm, center = true);
    }
}

module board_end_stops() {
    for (x_pos = [-14.0, 9.8]) {
        translate([x_pos, -pcb_size_mm[1] / 2 - 0.10, pcb_center_z_mm])
            cube(board_end_stop_size_mm, center = true);
    }
}

module lid_board_compression_pads() {
    for (p = lid_compression_pad_centers_mm) {
        translate(p)
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

        for (x_pos = [-14.0, 9.8]) {
            translate([x_pos, -pcb_size_mm[1] / 2 - 0.10, stop_z])
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
    for (p = service_screw_centers_mm) {
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
}
