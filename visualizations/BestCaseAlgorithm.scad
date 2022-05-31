$fn = 64;

// This uses openscad animations. If you don't want to deal with that,
// uncomment the following line (or give $t a value of your choosing):
// $t = 1/3;

function distFxn(a) = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);

function calculateFurthestPoints(index) = (
	allPoints[index] == undef? [0, [des]] : (
		let(
			point = allPoints[index],
			delta = point - des,
			dist = distFxn(delta),
			next = calculateFurthestPoints(index + 1)
		) dist > next[0]? [dist, [point]] : (
			dist == next[0]? [dist, concat(next[1], [point])] : next
		)
	)
);



cornerA = [5, 6, 7];
cornerB = [10, 12, 17];
minC = [for(i = [0:2]) min(cornerA[i], cornerB[i])];
maxC = [for(i = [0:2]) max(cornerA[i], cornerB[i])];
dimC = maxC - minC;
centerC = (minC + maxC) / 2; 

//des = [15, 10, 14];
desDist = 8;
angle = (($t % 0.1) / 0.1) * 360;
des = [
	round(cos(angle) * desDist + centerC[0]),
	round(sin(angle) * desDist + centerC[1]),
	round(minC[2] + dimC[2] * $t)
];

closest = [
	min(max(des[0], minC[0]), maxC[0]),
	min(max(des[1], minC[1]), maxC[1]),
	min(max(des[2], minC[2]), maxC[2])
];
closestDist = distFxn(closest - des);

corners = [
	for(
		x = [minC[0], maxC[0]],
		y = [minC[1], maxC[1]],
		z = [minC[2], maxC[2]]
	) [x, y, z]
];
	
allPoints = [
	for(
		x = [minC[0]:1:maxC[0]],
		y = [minC[1]:1:maxC[1]],
		z = [minC[2]:1:maxC[2]]
	) [x, y, z]
];
	
furthestData = calculateFurthestPoints(0);
furthestPointsDist = furthestData[0];
furthestPoints = furthestData[1];
	
furthestCorner = [
	let(desTrans = des - centerC)
	for(axis = [0:2]) desTrans[axis] > 0? (
		-dimC[axis] / 2
	) : dimC[axis] / 2
] + centerC;
furthestCornerDist = distFxn(furthestCorner - des);
	
echo(furthestCornerDist);

translate(-centerC) {
	for(point = corners) {
		color(point == minC || point == maxC? "green" : "blue")
			translate(point)
				sphere(r = 0.25);
	}
	for(point = furthestPoints) {
		if(point != furthestCorner) {
			echo(point);
			echo(furthestCorner);
			color("orange")
				translate(point)
					cube(1, center = true);
		}
	}
	for(point = allPoints) {
		testDist = distFxn(point - des);
		if(testDist <= closestDist && closest != point) {
			color("black")
				translate(point)
					cube(1.1, center = true);
		}
		if(testDist > furthestCornerDist) {
			color("black")
				translate(point)
					cube(1.1, center = true);
		}
	}
	translate(minC)
		%cube(maxC - minC);
	translate(des)
		sphere(r = 0.5);
	translate(closest)
		color("red")
			sphere(r = 0.5);
	
	translate(furthestCorner)
		color("purple")
			cube(1, center = true);
}