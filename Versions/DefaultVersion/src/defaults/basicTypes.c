#include "headers/RB_BasicTypes.h"


// Returns true if the two colors are equal. Otherwise returns false.
bool RB_colorsAreEqual(RB_Color color0, RB_Color color1) {
	return color0.r == color1.r && color0.g == color1.g && color0.b == color1.b;
}

// Returns true if the two coords are equal. Otherwise returns false.
bool RB_coordsAreEqual(RB_Coord coord0, RB_Coord coord1) {
	return coord0.x == coord1.x && coord0.y == coord1.y;
}