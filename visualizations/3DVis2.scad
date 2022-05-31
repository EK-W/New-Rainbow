r = 20;
sd = 12;
fx = 12;
fy = 8;
fr = sqrt(fx * fx + fy * fy);
echo(fr);

fmaj = (abs(fx) > abs(fy))? abs(fx) : abs(fy);
fmin = (abs(fx) > abs(fy))? abs(fy) : abs(fx);

for(x=[-r:r], y=[-r:r]) {
	xymaj = (abs(x) > abs(y))? x : y;
	xymin = (abs(x) > abs(y))? y : x;
	xyr = sqrt(x*x + y*y);
	if(xyr < r) {
		c = (abs(x) <= sd && abs(y) <= sd)?
			[1, 0, 0]
			: [1, 1, 0];
		c2 = (abs(xymaj) > abs(fmaj) && xyr <= fr)? [0, 1, 0] : c;
		c3 = (
			(abs(x) == abs(fx) && abs(y) == abs(fy))
			|| (abs(x) == abs(fy) && abs(y) == abs(fx))
		)? [0.5, 0.5, 0.5] : c2;
		c4 = (x == fx && y == fy)? [0, 0, 1] : c3;
		cMul = (abs(x) + abs(y)) % 2 == 1? 0.9 : 1;
		color(c4 * cMul)
			translate([x, y, 0])
				cube(1, center = true);
	}
}

//%cylinder(r=r, h=1, center = true);