// SPDX-License-Identifier: CERN-OHL-S-2.0
//
// Serviceable enclosure blank for the active USB hub lane.
// This is a mechanical starting point, not a validated enclosure.

$fn = 48;

board_size_mm = [24, 14, 1.6];
board_clearance_mm = 0.6;
wall_thickness_mm = 1.6;
bottom_floor_mm = 1.8;
lid_thickness_mm = 1.6;
module_height_mm = 4.5;
connector_opening_mm = [12.5, 5.5];
connector_support_depth_mm = 4.0;
explode_gap_mm = 5.0;

show_exploded_view = true;
show_reference_board = true;

outer_size_mm = [
    board_size_mm[0] + (2 * (wall_thickness_mm + board_clearance_mm)),
    board_size_mm[1] + (2 * (wall_thickness_mm + board_clearance_mm)),
    bottom_floor_mm + module_height_mm + lid_thickness_mm
];

module bottom_shell() {
    difference() {
        cube([outer_size_mm[0], outer_size_mm[1], bottom_floor_mm + module_height_mm]);

        translate([wall_thickness_mm,
                   wall_thickness_mm,
                   bottom_floor_mm])
            cube([outer_size_mm[0] - 2 * wall_thickness_mm,
                  outer_size_mm[1] - 2 * wall_thickness_mm,
                  module_height_mm + 0.2]);

        translate([-0.2,
                   (outer_size_mm[1] - connector_opening_mm[0]) / 2,
                   bottom_floor_mm])
            cube([wall_thickness_mm + 0.4,
                  connector_opening_mm[0],
                  connector_opening_mm[1]]);
    }

    translate([wall_thickness_mm,
               (outer_size_mm[1] - connector_opening_mm[0]) / 2,
               bottom_floor_mm])
        cube([connector_support_depth_mm,
              connector_opening_mm[0],
              1.5]);
}

module top_shell() {
    difference() {
        cube([outer_size_mm[0], outer_size_mm[1], lid_thickness_mm]);

        translate([wall_thickness_mm + 0.4,
                   wall_thickness_mm + 0.4,
                   -0.1])
            cube([outer_size_mm[0] - 2 * (wall_thickness_mm + 0.4),
                  outer_size_mm[1] - 2 * (wall_thickness_mm + 0.4),
                  lid_thickness_mm + 0.2]);
    }
}

module reference_board() {
    %translate([wall_thickness_mm + board_clearance_mm,
                wall_thickness_mm + board_clearance_mm,
                bottom_floor_mm])
        color("deepskyblue")
            cube(board_size_mm);
}

bottom_shell();

translate([0,
           0,
           show_exploded_view ? outer_size_mm[2] + explode_gap_mm : outer_size_mm[2]])
    top_shell();

if (show_reference_board) {
    reference_board();
}

