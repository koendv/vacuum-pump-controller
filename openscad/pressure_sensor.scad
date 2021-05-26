/*
 * box for BME280 vacuum sensor modules, pcb size 16x12 mm.
 *
 * Slide the sensor module in at the top of the box, pressure sensor
 * facing the hole in the front of the box, connecting cable up.  
 * Put 6mm diameter pneumatic tubing in the hole in the front of the box.
 * Seal top and front with melt glue. 
 */
include <util.scad> 
 
eps1 = 0.001;
eps2 = 2*eps1;
$fn=32;
nozzle_size = 0.4;
wall = 3.2;

pcb_x = 15.8+2*nozzle_size;
pcb_z = 12.5+nozzle_size;
pcb_y = 5.0+nozzle_size;

box_z = pcb_z+3*wall;

tube_dia = 6.0;
tube_x = pcb_x/2;
tube_y = -pcb_y/2-wall+nozzle_size;
tube_z = pcb_z/2+wall;
 
screw_hole = 3.2;
screw_offset = pcb_y/2 + 4*wall + screw_hole/2;
washer_dia = 5.2;
 
module box() {
    difference() {
        box_body();
        box_holes();
    }
}

module box_body() {

    linear_extrude(box_z)
    offset(2*wall)
    square([pcb_x, pcb_y]);
    
    /* tab for screw */
    hull() {
        translate([pcb_x/2, 0, screw_offset+wall])
        cube([washer_dia+2*wall, eps1, eps1], center=true);
        linear_extrude(wall)
        offset(wall)
        hull() {
            translate([pcb_x/2, 0, 0])
            square([washer_dia, eps1], center=true);
            translate([pcb_x/2,screw_offset, 0])
            circle(d=washer_dia);
        }
    }
    
    /* tube */
    tube_body();
    
}

module box_holes() {
    translate([0,0,wall+pcb_z+wall])
    linear_extrude(pcb_z+2*wall)
    offset(wall)
    square([pcb_x, pcb_y]);   
    
    translate([0,0,wall])
    linear_extrude(pcb_z+2*wall)
    offset(nozzle_size)
    square([pcb_x, pcb_y]);
    
    translate([pcb_x/2,screw_offset, -pcb_z])
    linear_extrude(2*pcb_z)
    circle(d=screw_hole);
    
    translate([pcb_x/2,screw_offset, wall])
    linear_extrude(2*pcb_z)
    circle(d=washer_dia);
    
    tube_hole();

}

module tube_body() {
    translate([tube_x,tube_y,tube_z])
    rotate([90,0,0])
    hull() {
        cylinder_outer(d=tube_dia+2*wall, h=wall, fn=6);
        cylinder_outer(d=tube_dia+4*wall, h=eps1, fn=6);
    }
}

module tube_hole() {
    translate([tube_x,0,tube_z])
    rotate([90,0,0])
    cylinder_outer(d=tube_dia, h=100, fn=6);
    translate([tube_x,tube_y,tube_z])
    rotate([90,0,0])
    cylinder_outer(d=tube_dia+2*wall, h=100, fn=6);
}

module lid() {
    cable_width = 1.27*6;
    cable_thickness = 1.27;
    translate([0,0,0])
    linear_extrude(wall/2)
    difference() {
        offset(wall-2*nozzle_size)
        square([pcb_x, pcb_y]);
        translate([0, pcb_y/2])
        square([pcb_x + cable_width, cable_thickness], center=true);
    }
}

module assembly() {
    box();
    /* tube */
    color("Blue")
    translate([tube_x,tube_y+wall,tube_z])
    rotate([90,0,0])
    difference() {
        cylinder(d = tube_dia, h =20);
        translate([0,0,-eps2])
        cylinder(d = tube_dia/2, h =40);
    }
    /* pcb */
    color("Purple")
    translate([0,0,wall])
    cube([pcb_x, pcb_y, pcb_z]);
    /* lid */
    if (0)
    translate([0,0,wall+pcb_z+wall+eps1])
    lid();
}

assembly();
//box();
//lid();