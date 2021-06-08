include <pcb.scad>

nozzle_size = 0.4;
wall_thickness = 2.4;
z_inside = 21; // was 21 XXX
pcb_pillar = 3.0;
pcb_thickness = 1.6;

w = 60;
h = 85;
border = 5;
slot_len = 10;

offset_x = pcb_offset_x+10;
offset_y = pcb_offset_y+1;
offset = [offset_x, offset_y];

module m3() {
    circle(d=3.2);
}


module through_holes() {
    translate([border, border]) m3();
    translate([h-border, w-border]) m3();
}

module slots() {
    hull() {
        translate([border, w-border]) m3();
        translate([border, w-border-slot_len]) m3();
    }
    hull() {
        translate([h-border, border]) m3();
        translate([h-border-slot_len, border]) m3();
    }
}

module top_holes() {
    translate(offset)
    pcb_top_holes();
    through_holes();
}

module bottom_holes() {
    translate(offset)
    pcb_bottom_holes();
    through_holes();
    slots();
}

module top() {
    difference(){
        linear_extrude(2*wall_thickness+z_inside)
        offset(nozzle_size+wall_thickness)
        square([h,w]);
        translate([0, 0, -eps])
        linear_extrude(wall_thickness+z_inside)
        offset(nozzle_size)
        square([h,w]);
        linear_extrude(2*z_inside)
        top_holes();
        dc005_jack();
        pj320a_jack();
        usb_connector();
    }
    translate([0, 0, wall_thickness])
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
        translate([0, 0, -wall_thickness])
        linear_extrude(z_inside)
        bottom_holes();
    }
}

module bottom_body() {
    linear_extrude(wall_thickness)
    square([h,w]);
    
    linear_extrude(pcb_pillar+wall_thickness)
    offset(wall_thickness) {
        translate(offset)
        pcb_bottom_holes();
        through_holes();
    }
}

module dc005_jack() {
    translate([19.5,-5,wall_thickness+pcb_pillar+pcb_thickness+nozzle_width])
    cube([9+2*nozzle_width,14.2+2*nozzle_width,11+2*nozzle_width]);
}

module pj320a_jack() {
    translate([47.75,0,wall_thickness+pcb_pillar+pcb_thickness+3.7])
    rotate([90,0,0])
    cylinder(d=6.1/cos(30),h=12.9,$fn=6,center=true);
}

module usb_connector(){
    translate([47.8, 60, 21.6])
    rotate([90, 0, 0])
    translate([0, 0, -5])
    linear_extrude(10)
    offset(2*nozzle_width)
    square([12.5, 7.8], center=true);
}

module 3d_model() {
    color("Blue")
    translate([39.3,28.2,13.5])
    scale(0.254)
    translate([-4166.994629,3158.121338,-31.578953])
    import("vpc.stl", 20);
}
if (0) {
    color("Red")
    top_holes();
    color("Blue")
    bottom_holes();
    translate([0,0,-eps])
    square([h,w]);
}

if (1){
   translate([0,0,eps]) 
top();
%bottom();
};
3d_model();
