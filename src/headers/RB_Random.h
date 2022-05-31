#ifndef EKW_RAINBOW_RB_RANDOM_H
#define EKW_RAINBOW_RB_RANDOM_H

#include <stdint.h>

typedef uint_fast32_t RB_RandomUInt;

void RB_RandomSetSeed(RB_RandomUInt seed);

RB_RandomUInt RB_RandomGetUInt();
// Generates a random unsigned int within the specified bounds, inclusive.
RB_RandomUInt RB_RandomGetUIntInBounds(RB_RandomUInt lower, RB_RandomUInt upper);



#endif