// SPDX-License-Identifier: CERN-OHL-S-2.0
//
// Parametric shell blank for the active optical ring lane.
// This is an honest mechanical starting point, not a validated enclosure.

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

battery_keepout_mm = [20, 15, 4.2];
module_keepout_mm = [14, 19, 3.2];

show_reference_solids = true;

inner_diameter_mm = finger_circumference_mm / PI;
outer_diameter_mm = inner_diameter_mm + (2 * radial_thickness_mm);

assert(sensor_angle_deg > 0 && sensor_angle_deg < 60);
assert(outer_diameter_mm > inner_diameter_mm + 2);

module shell_blank() {
    difference() {
        union() {
            ring_body();
            lower_rim();
        }

        finger_opening();
        battery_pocket();
        module_pocket();
        sensor_aperture();
        service_seam_groove();
        glide_pad_pockets();
    }
}

module ring_body() {
    cylinder(h = shell_height_mm, d = outer_diameter_mm);
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
    translate([0, radial_thickness_mm * 0.35, shell_height_mm - battery_keepout_mm[2] / 2])
        cube(battery_keepout_mm, center = true);
}

module module_pocket() {
    translate([0, -radial_thickness_mm * 0.55, shell_height_mm - module_keepout_mm[2] / 2])
        cube(module_keepout_mm, center = true);
}

module sensor_aperture() {
    translate([0, sensor_platform_offset_mm, 1.2])
        rotate([sensor_angle_deg, 0, 0])
            cylinder(h = 24, d = sensor_aperture_diameter_mm, center = true);
}

module service_seam_groove() {
    translate([0, 0, service_seam_height_mm - 0.45])
        difference() {
            cylinder(h = 0.9, d = outer_diameter_mm + 0.2);
            cylinder(h = 1.1, d = outer_diameter_mm - 1.4);
        }
}

module glide_pad_pockets() {
    for (a = [45, 135, 225, 315]) {
        rotate([0, 0, a])
            translate([outer_diameter_mm / 2 - 2.4, 0, -rim_height_mm + 0.15])
                cylinder(h = glide_pad_thickness_mm + 0.2, d = 3.2);
    }
}

module reference_solids() {
    %translate([0, radial_thickness_mm * 0.35, shell_height_mm - battery_keepout_mm[2] / 2])
        color("gold") cube(battery_keepout_mm, center = true);

    %translate([0, -radial_thickness_mm * 0.55, shell_height_mm - module_keepout_mm[2] / 2])
        color("deepskyblue") cube(module_keepout_mm, center = true);
}

shell_blank();

if (show_reference_solids) {
    reference_solids();
}

