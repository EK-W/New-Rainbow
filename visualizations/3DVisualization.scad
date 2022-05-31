r = 5;
f = [3, 2, 0];

/*for(x = [-r:1:r]) {
	for(y = [-r:1:r]) {
		for(z = [-r:1:r]) {
			if(x*x + y*y + z*z <= r*r)
				translate([x, y, z])
					cube([1, 1, 1], center = true);
		}
	}
}*/


translate(f) cube([1, 1, 1], center = true);

fr = sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);

for(x = [f[0]:1:fr]) {
	for(y = [f[1]:1:fr]) {
		for(z = [f[2]:1:fr]) {
			if(x*x + y*y + z*z < fr*fr) {
				translate([x, y, z])
					cube([1, 1, 1], center = true);
			}
		}
	}
}
			