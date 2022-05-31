#ifndef EKW_RAINBOW_RB_DATA_H
#define EKW_RAINBOW_RB_DATA_H

#include "RB_BasicTypes.h"
#include <stdbool.h>
#include <stdint.h>

// forward declaring structs here because the public-facing part of the library doesn't need to know their functions.
// In fact, the forward facing part might not even need to know the contents of RB_Data. That's something to consider.
typedef struct RB_AssignmentQueue_s RB_AssignmentQueue;
typedef struct RB_ColorPool_s RB_ColorPool;
typedef struct RB_PixelMap_s RB_PixelMap;
typedef struct RB_Display_s RB_Display;

typedef struct RB_Data_s RB_Data;

typedef struct RB_Config_s RB_Config;

// TODO: Decouple display from the rest of rainbow so that these structs don't need to be visible.
struct RB_Config_s {
	RB_Size width;
	RB_Size height;
	bool mapDimensionsSet;

	RB_ColorChannelSize rRes;
	RB_ColorChannelSize gRes;
	RB_ColorChannelSize bRes;
	bool colorResSet;

	int windowWidth;
	int windowHeight;
	bool windowDimensionsSet;

	uint32_t seed;
	bool seedSet;
};

struct RB_Data_s {
	RB_AssignmentQueue* assignmentQueue; // the queue of coordinates that should be assigned a color.
	RB_ColorPool* colorPool;
	RB_PixelMap* pixelMap;
	RB_Display* display;

	RB_Config config;
};

// CONFIG FUNCTIONS:
RB_Config* RB_newConfig();
void RB_freeConfig(RB_Config*);

void RB_setColorResolution(RB_Config*, RB_ColorChannelSize, RB_ColorChannelSize, RB_ColorChannelSize);

void RB_setMapDimensions(RB_Config*, RB_Size, RB_Size);

void RB_setWindowDimensions(RB_Config*, int, int);

void RB_setRandomSeed(RB_Config*, uint32_t);


// ALLOCATION FUNCTIONS:
RB_Data* RB_init(RB_Config*);

void RB_free(RB_Data*);


// HELPER FUNCTIONS
RB_Color RB_getRandomColor(RB_Data*);

RB_Coord RB_getRandomCoord(RB_Data*);

// GENERATION FUNCTIONS:
void RB_setCoordColor(RB_Data*, RB_Coord, RB_Color);

// Sets the color for another pixel. Returns true if there are pixels left to generate, otherwise returns false.
bool RB_generateNextPixel(RB_Data*);

#endif