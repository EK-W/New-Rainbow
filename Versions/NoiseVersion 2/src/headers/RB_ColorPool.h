#ifndef EKW_RAINBOW_RB_COLOR_POOL_H
#define EKW_RAINBOW_RB_COLOR_POOL_H

#include "RB_Main.h"
#include "RB_BasicTypes.h"
#include <stdbool.h>

// Allocates a colorPool with the specified range of colors.
RB_ColorPool* RB_createColorPool(RB_ColorChannelSize, RB_ColorChannelSize, RB_ColorChannelSize);

// Frees a previously allocated color pool
void RB_freeColorPool(RB_ColorPool*);

RB_Color RB_findIdealAvailableColor(RB_ColorPool*, RB_Color);

bool RB_colorIsAvailableInPool(RB_ColorPool*, RB_Color);

// Attempts to remove the specified color from the pool.
// If the specified color is contained by the Color Pool, removes it and returns true.
// If the specified color is not contained by the Color Pool, returns false.
bool RB_removeColorFromPool(RB_ColorPool*, RB_Color);

#endif