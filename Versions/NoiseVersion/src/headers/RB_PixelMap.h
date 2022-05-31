#ifndef EKW_RAINBOW_RB_PIXEL_MAP_H
#define EKW_RAINBOW_RB_PIXEL_MAP_H

#include "RB_Main.h"
#include "RB_BasicTypes.h"
#include "RB_Pixel.h"

// allocates a pixel map with the specified dimensions
RB_PixelMap* RB_createPixelMap(RB_Size, RB_Size);

// deallocates the pixel map
void RB_freePixelMap(RB_PixelMap*);

// returns the pixel that the coord maps to, or NULL if the coord does not map to a pixel.
RB_Pixel* RB_getPixel(RB_PixelMap*, RB_Coord);

// Determines, based on the current state of the pixelMap, the preferred color for the specified coordinate.
RB_Color RB_determinePreferredCoordColor(RB_PixelMap*, RB_Coord);

// Add cords to the queue in an implementation-defined pattern relative to the given coord
void RB_addResultantCoordsToQueue(RB_PixelMap*, RB_AssignmentQueue*, RB_Coord);

#endif