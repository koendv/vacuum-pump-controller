include <pcb.scad>
include <util.scad>

$fn=$preview?16:32;
nozzle_size = 0.4;
wall_thickness = 4 * nozzle_size;
z_inside = 21;
pcb_pillar = 3.0;
pcb_thickness = 1.6;

w = 60;
h = 95;
border = 5;
slot_len = 10;

offset_x = pcb_offset_x+10;
offset_y = pcb_offset_y+1;
offset = [offset_x, offset_y];

m3_nut_h = 2.4+nozzle_width;
m3_nut_d = 5.5+nozzle_width;
m3_nut_coords = [
    [offset_x, offset_y, 0],
    [offset_x, offset_y+47, 0],
    [offset_x+47, offset_y, 0],
    [offset_x+47, offset_y+47, 0],
    [border, border, 0],
    [h-border, w-border, 0]
    ];

bottom_thickness = m3_nut_h + nozzle_width;

// oled dimensions from oled.jpg
oled_w = 27.3;
oled_h = 27.8;
oled_x = 21.744;
oled_y = 10.864;
oled_z = bottom_thickness+pcb_pillar+pcb_thickness+11.0;

oled_dist = (27.8 - 19.268)/2 + 2.1 - 1.5; // distance from connector pin to first pixel 

// cutout for oled display
module oled_screen() {
    corner = pins_screws[0];
    pin3 = (pin3_oled-corner) / 1000;
    translate(offset+pin3)
    translate([oled_dist, -oled_x/2])
    offset(1.0)
    square([oled_y, oled_x]);
}

// circle same diameter as M3 screw
module m3() {
    circle(d=3.2);
}

module m3_nut_trap() {
    translate([0, 0, -eps2])
    cylinder_outer(d=m3_nut_d, h=m3_nut_h, fn=6);
}

module m3_nut_traps() {
    for (p = m3_nut_coords) {
        translate(p)
        m3_nut_trap();
    }
}

// this is a one layer bridging layer obove the nut trap
// needed for fdm printing
module m3_nut_bridging() {
    translate([0, 0, m3_nut_h])
    for (p = m3_nut_coords) {
        translate(p)
        cylinder(d=m3_nut_d, h=nozzle_width);
    }
}

// screws to join top and bottom
module through_holes() {
    translate([border, border]) m3();
    translate([h-border, w-border]) m3();
}

// slots for wall-mount
module slots() {
    hull() {
        translate([border, w-border]) m3();
        translate([border, w-border-slot_len]) m3();
    }
    hull() {
        translate([h-border, border]) m3();
        translate([h-border-slot_len, border]) m3();
    }
    hull() {
        translate([border+slot_len, w/2]) m3();
        translate([border+2*slot_len, w/2]) m3();
    }
    hull() {
        translate([h-border-slot_len, w/2-slot_len/2]) m3();
        translate([h-border-slot_len, w/2+slot_len/2]) m3();
    }
}

module top_holes() {
    translate(offset)
    pcb_top_holes();
    through_holes();
    oled_screen();
}

module bottom_holes() {
    translate(offset)
    pcb_bottom_holes();
    through_holes();
    slots();
}

module top() {
    difference(){
        linear_extrude(bottom_thickness+z_inside+wall_thickness)
        offset(nozzle_size+wall_thickness)
        square([h,w]);
        translate([0, 0, -2*eps2])
        linear_extrude(bottom_thickness+z_inside)
        offset(nozzle_size)
        square([h,w]);
        linear_extrude(2*z_inside)
        top_holes();
        dc005_jack();
        pj320a_jack();
        usb_connector();
    }
    // small triangular corners
    translate([0, 0, bottom_thickness])
    linear_extrude(z_inside)
    intersection() {
        for (p = [[0,0], [0,w], [h,0], [h,w]]) {
            translate(p)
            rotate([0, 0, 45])
            square([2*wall_thickness, 2*wall_thickness], center=true);
        }
        offset(nozzle_size)
        square([h,w]);
    }
}

module bottom() {
    difference() {
        bottom_body();
        translate([0, 0, -bottom_thickness])
        linear_extrude(z_inside)
        bottom_holes();
        m3_nut_traps();
    }
    m3_nut_bridging();
}

module bottom_body() {
    linear_extrude(bottom_thickness)
    square([h,w]);
    // pillars
    linear_extrude(pcb_pillar+bottom_thickness)
    offset(wall_thickness) {
        translate(offset)
        pcb_bottom_holes();
        through_holes();
    }
    // buttress under oled
    oled_support();
}

module dc005_jack() {
    translate([19.5,-5,bottom_thickness+pcb_pillar+pcb_thickness+nozzle_width])
    cube([9+2*nozzle_width,14.2+2*nozzle_width,11+2*nozzle_width]);
}

module pj320a_jack() {
    translate([47.75,0,bottom_thickness+pcb_pillar+pcb_thickness+3.7])
    rotate([90,0,0])
    cylinder_outer(d=6.1,h=12.9,fn=6);
}

module usb_connector(){
    translate([47.8, 60, 14.8])
    translate([0, 0, bottom_thickness+pcb_pillar+pcb_thickness])
    rotate([90, 0, 0])
    translate([0, 0, -5])
    linear_extrude(10)
    offset(2*nozzle_width)
    //square([8.4, 2.6], center=true); // usb c  connector
    square([12.5, 7.8], center=true); // connector with overmold
}


module oled_support() {
    corner = pins_screws[0];
    pin3 = (pin3_oled-corner) / 1000;
    translate(offset+pin3)
    translate([oled_h, -oled_w/2, 0])
    difference() {
        hull() {
            translate([-2*wall_thickness,0,0])
            cube([2*wall_thickness, oled_w, oled_z]);
            translate([-oled_w/2,0,0])
            cube([oled_w/2, oled_w, eps]);
        }
        translate([-oled_w-wall_thickness, wall_thickness, -eps])
        cube([oled_w,oled_w-2*wall_thickness,2*oled_z]);
    }
}

// 3d model
module 3d_model() {
    color("Blue")
    translate([39.3,28.2,bottom_thickness+pcb_pillar+8.1]) // trial and error
    scale(0.254)
    translate([-4166.994629,3158.121338,-31.578953]) // from meshlab, Filters -> Quality measures and computations -> Compute Geometric Measures
    import("vpc.stl", 20); // exported from easyeda
}

if (1)
translate([0, 0, eps])
//rotate([180,0,0])
top();
if (1)
bottom();
if (0)
3d_model();

//not truncated
