// SPDX-License-Identifier: CERN-OHL-S-2.0
//
// First mechanical packet for the active USB-HUB direct-plug board.
// This captures the stepped PCB, shell-clamp holes, service access, and
// adjacent-port gauges from kicad/usb_hub.kicad_pcb. It is still a
// prototype-fit source, not measured mechanical evidence.
//
// Export modes:
//   enclosure
//   host_fit_coupon
//   clamp_alignment_gauge
//   service_hatch_reach_gauge
//   validation_set

$fn = 64;

board_outline_pts = [
    [0, 5.2],
    [14, 5.2],
    [14, 0],
    [54, 0],
    [54, 26],
    [14, 26],
    [14, 20.8],
    [0, 20.8]
];

board_thickness_mm = 1.6;
board_clearance_mm = 0.45;
wall_thickness_mm = 1.4;
bottom_floor_mm = 1.5;
top_lid_thickness_mm = 1.5;
lid_lip_depth_mm = 1.0;
top_component_clearance_mm = 5.3;
explode_gap_mm = 7.0;

// Board features copied from ../kicad/usb_hub.kicad_pcb.
body_shoulder_x_mm = 14;
board_rear_x_mm = 54;
shell_clamp_holes_mm = [[15.6, 6.6], [15.6, 19.4]];
shell_clamp_board_drill_mm = 1.4;
shell_clamp_screw_clearance_mm = 1.55;
shell_clamp_boss_od_mm = 3.6;
service_window_origin_mm = [17.0, 22.05];
service_window_size_mm = [34.2, 3.95];
service_hatch_clearance_mm = 0.25;
service_hatch_thickness_mm = 0.9;
antenna_keepout_origin_mm = [41.7, 5.3];
antenna_keepout_size_mm = [5.3, 15.4];

// Host-fit reference gauges. These are not passed/failed by CAD alone.
host_face_x_mm = 0;
host_insertion_zone_depth_mm = 14;
adjacent_port_pitch_mm = 14.0;
adjacent_usb_a_shell_width_ref_mm = 12.2;
adjacent_port_gauge_depth_mm = 18.0;

export_mode = "enclosure";
show_exploded_view = true;
show_reference_board = true;
show_service_hatch = true;
show_host_clearance_gauges = true;

quick_print_coupon_height_mm = 3.0;
quick_print_plate_thickness_mm = 1.2;
host_fit_coupon_depth_mm = 24.0;
clamp_alignment_gauge_depth_mm = 23.0;
validation_artifact_gap_mm = 11.0;

bottom_shell_height_mm =
    bottom_floor_mm + board_thickness_mm + top_component_clearance_mm;
assembled_lid_z_mm = bottom_shell_height_mm;
exploded_lid_z_mm = bottom_shell_height_mm + explode_gap_mm;

function lid_z() =
    show_exploded_view ? exploded_lid_z_mm : assembled_lid_z_mm;

module board_outline_2d(expand_mm = 0) {
    offset(delta = expand_mm)
        polygon(points = board_outline_pts);
}

module shell_prism(height_mm, expand_mm = 0) {
    // Do not let the printed shell protrude in front of the USB-A host face.
    intersection() {
        linear_extrude(height = height_mm)
            board_outline_2d(expand_mm);

        translate([host_face_x_mm,
                   -20,
                   -0.1])
            cube([board_rear_x_mm + expand_mm + 20,
                  26 + (2 * expand_mm) + 40,
                  height_mm + 0.2]);
    }
}

module front_slice_prism(height_mm, depth_mm, expand_mm = 0) {
    intersection() {
        shell_prism(height_mm, expand_mm);

        translate([host_face_x_mm,
                   -20,
                   -0.1])
            cube([depth_mm,
                  66,
                  height_mm + 0.2]);
    }
}

module screw_hole_stack(height_mm, z_start_mm = -0.1) {
    for (hole = shell_clamp_holes_mm) {
        translate([hole[0],
                   hole[1],
                   z_start_mm])
            cylinder(h = height_mm,
                     d = shell_clamp_screw_clearance_mm);
    }
}

module bottom_clamp_bosses() {
    for (hole = shell_clamp_holes_mm) {
        translate([hole[0],
                   hole[1],
                   0])
            cylinder(h = bottom_floor_mm,
                     d = shell_clamp_boss_od_mm);
    }
}

module top_compression_pads() {
    for (hole = shell_clamp_holes_mm) {
        translate([hole[0],
                   hole[1],
                   -top_component_clearance_mm + 0.05])
            cylinder(h = top_component_clearance_mm - 0.05,
                     d = shell_clamp_boss_od_mm);
    }
}

module host_fit_coupon() {
    shell_expand_mm = wall_thickness_mm + board_clearance_mm;

    union() {
        front_slice_prism(quick_print_coupon_height_mm,
                          host_fit_coupon_depth_mm,
                          shell_expand_mm);

        // Raised witness line: if this shoulder contacts the host before the
        // plug seats, the wider body is too close to the USB-A nose.
        translate([body_shoulder_x_mm - 0.2,
                   5.2 - shell_expand_mm,
                   quick_print_coupon_height_mm])
            cube([0.4,
                  15.6 + (2 * shell_expand_mm),
                  0.6]);

        // A shallow host-face datum keeps the coupon honest about x=0 without
        // creating a printed tongue meant to enter the USB port.
        translate([host_face_x_mm,
                   5.2 - shell_expand_mm,
                   quick_print_coupon_height_mm])
            cube([0.35,
                  15.6 + (2 * shell_expand_mm),
                  0.45]);
    }
}

module clamp_alignment_gauge() {
    difference() {
        union() {
            front_slice_prism(quick_print_plate_thickness_mm,
                              clamp_alignment_gauge_depth_mm,
                              0.25);

            for (hole = shell_clamp_holes_mm) {
                translate([hole[0],
                           hole[1],
                           0])
                    cylinder(h = quick_print_plate_thickness_mm + 0.55,
                             d = shell_clamp_boss_od_mm);
            }

            translate([body_shoulder_x_mm - 0.15,
                       5.2,
                       quick_print_plate_thickness_mm])
                cube([0.3,
                      15.6,
                      0.45]);
        }

        screw_hole_stack(quick_print_plate_thickness_mm + 0.95,
                         -0.2);
    }
}

module service_hatch_reach_gauge() {
    frame_margin_mm = 2.0;
    frame_origin = [
        service_window_origin_mm[0] - frame_margin_mm,
        service_window_origin_mm[1] - frame_margin_mm
    ];
    frame_size = [
        service_window_size_mm[0] + (2 * frame_margin_mm),
        service_window_size_mm[1] + (2 * frame_margin_mm)
    ];

    union() {
        difference() {
            translate([frame_origin[0],
                       frame_origin[1],
                       0])
                cube([frame_size[0],
                      frame_size[1],
                      quick_print_plate_thickness_mm]);

            translate([service_window_origin_mm[0],
                       service_window_origin_mm[1],
                       -0.1])
                cube([service_window_size_mm[0],
                      service_window_size_mm[1],
                      quick_print_plate_thickness_mm + 0.2]);

            // Spudger approach witness for the removable hatch notch.
            translate([service_window_origin_mm[0] + service_window_size_mm[0] - 3.0,
                       service_window_origin_mm[1] + service_window_size_mm[1] - 0.35,
                       -0.1])
                cylinder(h = quick_print_plate_thickness_mm + 0.2,
                         d = 1.8);
        }

        translate([0,
                   -(service_window_size_mm[1]
                     + frame_margin_mm
                     + validation_artifact_gap_mm),
                   0])
            service_hatch();
    }
}

module validation_set() {
    host_fit_coupon();

    translate([0,
               26 + (2 * (wall_thickness_mm + board_clearance_mm))
                   + validation_artifact_gap_mm,
               0])
        clamp_alignment_gauge();

    translate([0,
               26 + (2 * (wall_thickness_mm + board_clearance_mm))
                   + 40,
               0])
        service_hatch_reach_gauge();
}

module bottom_shell() {
    difference() {
        union() {
            difference() {
                shell_prism(bottom_shell_height_mm,
                            wall_thickness_mm + board_clearance_mm);

                translate([0,
                           0,
                           bottom_floor_mm])
                    shell_prism(bottom_shell_height_mm,
                                board_clearance_mm);
            }

            bottom_clamp_bosses();

            // Small connector-side floor shelf; the real load path is the
            // screw-clamped MH1/MH2 pair plus J1 shell tabs, not solder alone.
            translate([host_face_x_mm,
                       5.2 - board_clearance_mm,
                       bottom_floor_mm])
                cube([body_shoulder_x_mm + 0.8,
                      15.6 + (2 * board_clearance_mm),
                      0.7]);
        }

        screw_hole_stack(bottom_floor_mm + 0.2);
    }
}

module top_lid() {
    difference() {
        union() {
            shell_prism(top_lid_thickness_mm,
                        wall_thickness_mm + board_clearance_mm);

            translate([0,
                       0,
                       -lid_lip_depth_mm])
                difference() {
                    shell_prism(lid_lip_depth_mm,
                                wall_thickness_mm * 0.55);
                    translate([0,
                               0,
                               -0.1])
                        shell_prism(lid_lip_depth_mm + 0.2,
                                    board_clearance_mm + 0.2);
                }

            top_compression_pads();
        }

        screw_hole_stack(top_lid_thickness_mm + top_component_clearance_mm + 0.4,
                         -top_component_clearance_mm - 0.2);

        translate([service_window_origin_mm[0],
                   service_window_origin_mm[1],
                   -lid_lip_depth_mm - 0.1])
            cube([service_window_size_mm[0],
                  service_window_size_mm[1],
                  top_lid_thickness_mm + lid_lip_depth_mm + 0.2]);
    }
}

module service_hatch() {
    hatch_size_mm = [
        service_window_size_mm[0] - (2 * service_hatch_clearance_mm),
        service_window_size_mm[1] - (2 * service_hatch_clearance_mm),
        service_hatch_thickness_mm
    ];

    difference() {
        translate([service_window_origin_mm[0] + service_hatch_clearance_mm,
                   service_window_origin_mm[1] + service_hatch_clearance_mm,
                   0])
            cube(hatch_size_mm);

        // Spudger notch on the service edge, away from the host plug.
        translate([service_window_origin_mm[0] + service_window_size_mm[0] - 3.0,
                   service_window_origin_mm[1] + service_window_size_mm[1] - 0.35,
                   -0.1])
            cylinder(h = 1.1,
                     d = 1.8);
    }
}

module reference_board() {
    %translate([0,
                0,
                bottom_floor_mm + 0.02])
        color("deepskyblue")
            linear_extrude(height = board_thickness_mm)
                polygon(points = board_outline_pts);
}

module reference_board_features() {
    if (show_reference_board) {
        for (hole = shell_clamp_holes_mm) {
            %translate([hole[0],
                        hole[1],
                        bottom_floor_mm + board_thickness_mm + 0.04])
                color("orange")
                    cylinder(h = 0.12,
                             d = shell_clamp_board_drill_mm);
        }

        // TP1-TP9 service row: VBUS, 3V3, GND, D-/D+ trace access, EN,
        // BOOT_N, UART_TX_DBG, and UART_RX_DBG.
        for (pad_x = [18 : 3 : 42]) {
            %translate([pad_x,
                        24.2,
                        bottom_floor_mm + board_thickness_mm + 0.08])
                color("gold")
                    cylinder(h = 0.12,
                             d = 1.0);
        }

        // SW1 pad-actuated BOOT_N service pair.
        %translate([48.2,
                    22.8,
                    bottom_floor_mm + board_thickness_mm + 0.08])
            color("gold")
                cube([1.6, 1.2, 0.12]);

        // Plastic-only rear antenna clearance reference.
        %translate([antenna_keepout_origin_mm[0],
                    antenna_keepout_origin_mm[1],
                    bottom_floor_mm + board_thickness_mm + 0.15])
            color([1, 0, 0, 0.25])
                cube([antenna_keepout_size_mm[0],
                      antenna_keepout_size_mm[1],
                      top_component_clearance_mm]);
    }
}

module host_clearance_gauges() {
    if (show_host_clearance_gauges) {
        // Insertion zone: wider body must begin behind this shoulder.
        %translate([body_shoulder_x_mm,
                    -3.0,
                    0])
            color([1, 0.65, 0, 0.25])
                cube([0.25,
                      32.0,
                      bottom_shell_height_mm + top_lid_thickness_mm]);

        // Adjacent-port reference spaces on both sides of the USB-A nose.
        for (side = [-1, 1]) {
            %translate([host_face_x_mm,
                        13 + (side * adjacent_port_pitch_mm)
                            - (adjacent_usb_a_shell_width_ref_mm / 2),
                        bottom_floor_mm])
                color([1, 0.65, 0, 0.18])
                    cube([adjacent_port_gauge_depth_mm,
                          adjacent_usb_a_shell_width_ref_mm,
                          4.8]);
        }

        // Host insertion/removal depth reference for the direct-plug nose.
        %translate([host_face_x_mm,
                    5.2,
                    bottom_floor_mm])
            color([0, 1, 0.2, 0.15])
                cube([host_insertion_zone_depth_mm,
                      15.6,
                      5.2]);
    }
}

module enclosure_packet() {
    bottom_shell();

    translate([0,
               0,
               lid_z()])
        top_lid();

    if (show_service_hatch) {
        translate([0,
                   0,
                   show_exploded_view
                       ? lid_z() + top_lid_thickness_mm + (explode_gap_mm * 0.45)
                       : lid_z() + top_lid_thickness_mm
                           - service_hatch_thickness_mm + 0.03])
            service_hatch();
    }

    if (show_reference_board) {
        reference_board();
        reference_board_features();
    }

    host_clearance_gauges();
}

if (export_mode == "enclosure") {
    enclosure_packet();
} else if (export_mode == "host_fit_coupon") {
    host_fit_coupon();
} else if (export_mode == "clamp_alignment_gauge") {
    clamp_alignment_gauge();
} else if (export_mode == "service_hatch_reach_gauge") {
    service_hatch_reach_gauge();
} else if (export_mode == "validation_set") {
    validation_set();
}
