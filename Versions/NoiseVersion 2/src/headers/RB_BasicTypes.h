#ifndef EKW_RAINBOW_RB_BASIC_TYPES_H
#define EKW_RAINBOW_RB_BASIC_TYPES_H


#include <stdint.h>
#include <stdbool.h>

// Value representing the maximum number of colors that this program can handle.
// Equal to (bits_per_color_channel^channels_per_color)
#define RB_MAXIMUM_POSSIBLE_NUMBER_OF_COLORS 0x1000000

#define RB_MAXIMUM_COLOR_CHANNEL_RESOLUTION 0x100

typedef uint_fast8_t RB_ColorChannel; 

// Type big enough to represent the bounds of a color channel.
// Basically, the smallest type that's at least 1 bit larger than RB_ColorChannel.
typedef uint_fast16_t RB_ColorChannelSize;

// Type containing at least 1 byte more than a colorChannel.
// A ColorChannelSum is garanteed to be able to hold the sum of up to 256 colorChannel values
typedef uint_fast16_t RB_ColorChannelSum;

// A signed type with at least (bits_per_color_channel) bits, in addition to a sign bit
// A ColorChannelDifference is guaranteed to be able to hold the result of subtracting any valid
// colorChannel value from any other valid colorChannel value.
typedef int_fast16_t RB_ColorChannelDifference;

// Type that contains at least ((bits_per_color_channel * 2) + log2(channels_per_color)) bits.
// A ColorSquareDistance is guaranteed to be able to hold the square of the distance between any two colors.
// This is because the square of the distances will always be an integral value, so we can compare
// distances based on their relative squared values instead of needing to take their square roots and deal with doubles.
typedef uint_fast32_t RB_ColorSquareDistance;


typedef struct {
	RB_ColorChannel r;
	RB_ColorChannel g;
	RB_ColorChannel b;
} RB_Color;




// Type big enough to represent the total number of colors/pixels in an image.
// In other words, a type larger than bits_per_color_channel * 3. Signed because we have bits to spare
// and it's convenient.
// Guaranteed to be:
// - A signed integral type
// - Contains, in addition to the sign bit, at least 7 bits more than are needed to represent each color.
// 		- In other words, contains at least ((bits_per_color_channel * 3) + 7) bits plus an additional sign bit
typedef int_fast32_t RB_Size;

// Guaranteed to be:
// - An unsigned integral type
// - Contains at least 8 bits more than are needed to represent each color.
//		- In other words, contains at least ((bits_per_color_channel * 3) + 8)
typedef uint_fast32_t RB_USize;

// In theory, one dimension of the screen could be only a single pixel large, meaning the other dimension would
// need to be as wide as there are colors/pixels. Therefore, coordinate components need to be as large as RB_Size
typedef struct {
	RB_Size x;
	RB_Size y;
} RB_Coord;


// Functions!


// Returns true if the two colors are equal. Otherwise returns false.
bool RB_colorsAreEqual(RB_Color, RB_Color);

// Returns true if the two coords are equal. Otherwise returns false.
bool RB_coordsAreEqual(RB_Coord, RB_Coord);

#endif