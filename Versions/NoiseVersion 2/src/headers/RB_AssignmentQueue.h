#ifndef EKW_RAINBOW_RB_ASSIGNMENT_QUEUE_H
#define EKW_RAINBOW_RB_ASSIGNMENT_QUEUE_H

#include "RB_Main.h"
#include "RB_BasicTypes.h"

// Allocates an assignmentQueue capable of storing the specified number of coords.
RB_AssignmentQueue* RB_createAssignmentQueue(RB_Size, RB_Size, RB_Size);

// Frees a previously allocated assignmentQueue
void RB_freeAssignmentQueue(RB_AssignmentQueue*);

// Returns true if the queue is empty. Otherwise, returns false.
bool RB_isQueueEmpty(RB_AssignmentQueue*);

// Returns true if the queue is full. Otherwise, returns false.
bool RB_isQueueFull(RB_AssignmentQueue*);

RB_Size RB_getQueueSize(RB_AssignmentQueue*);

RB_Size RB_getQueueCapacity(RB_AssignmentQueue*);

// Chooses (using an implementation-specific method) a coord from the queue and returns it.
RB_Coord RB_chooseCoordFromAssignmentQueue(RB_AssignmentQueue*);

bool RB_coordIsWithinQueueBounds(RB_AssignmentQueue*, RB_Coord);

bool RB_coordIsInQueue(RB_AssignmentQueue*, RB_Coord);

// Removes the specified coord from the queue. If the coord is not in the queue, throws an error.
void RB_removeCoordFromAssignmentQueue(RB_AssignmentQueue*, RB_Coord);

/*
- If the coord is not already queued or assigned, adds the coord to the queue (with the specified priority, if
the RB_AssignmentQueue implementation supports prioritization).
- If the coord is already queued and the RB_AssignmentQueue implementation supports prioritization,this function may update
the coord's priority, though that is not guaranteed.

Higher positive values for priorityIndex correspond to lower prioritization. A priorityIndex of 0 corresponds to the maximum
possible prioritization.
Negative values for priorityIndex correspond to the lowest possible prioritization.
*/
void RB_addCoordToAssignmentQueue(RB_AssignmentQueue*, RB_Coord, RB_Size priorityIndex);

#endif