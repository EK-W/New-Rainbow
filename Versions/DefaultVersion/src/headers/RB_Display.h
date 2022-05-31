#ifndef EKW_RAINBOW_RB_DISPLAY_H
#define EKW_RAINBOW_RB_DISPLAY_H

#include "RB_BasicTypes.h"
#include "RB_Main.h"
#include <stdbool.h>
/*
Allocates a display object.
Parameters:
	wWidth, wHeight:
		The width and height of the display window.
	pWidth, pHeight:
		The width and height of the pixelMap that the display will be showing.
	rRes, gRes, bRes:
		The resolutions of each of the color channels
*/
RB_Display* RB_createDisplay(
	int wWidth, int wHeight,
	RB_Size pWidth, RB_Size pHeight,
	RB_ColorChannelSize rRes, RB_ColorChannelSize gRes, RB_ColorChannelSize bRes
);

void RB_freeDisplay(RB_Display*);


// Forces the display to update immediately.
// If the boolean is true, it will interrupt the frame rate, meaning the next unforced update will not happen until
// at least (1/frame_rate) seconds after this function was called. If the boolean is false, the minimum time until
// the next forced update will not be changed.
void RB_forceUpdateDisplay(RB_Display*, bool);

// If it has been a sufficiently long time since the last update, updates the display and returns true.
// Otherwise, does not update the display and returns false.
bool RB_updateDisplay(RB_Display*);

// Handles window events. If the window is closing, returns 0. Otherwise, returns 1.
int RB_handleWindowEvents(RB_Display*);

// Sets the pixel at the specified coordinate to the specified color
// Note: This function will convert the color to the displayed color format. You should NOT do that beforehand.
void RB_setDisplayedPixelColor(RB_Display*, RB_Coord, RB_Color);

#endif