#include "headers/RB_AssignmentQueue.h"
#include <stdlib.h>
#include <stdio.h>

#define RB_QUEUE_INDEX_UNQUEUED -1

struct RB_AssignmentQueue_s {
	RB_Coord* coords;
	RB_Size coordLen;
	RB_Size maxCoordLen;

	RB_Size** coordIndexes;
	RB_Size xRange;
	RB_Size yRange;
};

// Allocates an assignmentQueue capable of storing the specified number of pixels.
RB_AssignmentQueue* RB_createAssignmentQueue(RB_Size size, RB_Size xRange, RB_Size yRange) {
	RB_AssignmentQueue* ret = malloc(
		sizeof(RB_AssignmentQueue)
		+ (sizeof(RB_Coord) * size)
		+ (sizeof(RB_Size*) * xRange)
		+ (sizeof(RB_Size) * xRange * yRange)
	);

	if(ret == NULL) {
		return NULL;
	}

	ret->coords = (RB_Coord*) (ret + 1);
	ret->maxCoordLen = size;
	ret->coordLen = 0;

	ret->coordIndexes = (RB_Size**) (ret->coords + size);
	ret->xRange = xRange;
	ret->yRange = yRange;
	RB_Size* xyIndexesStart = (RB_Size*) (ret->coordIndexes + xRange);

	for(RB_Size x = 0; x < xRange; x++) {
		ret->coordIndexes[x] = xyIndexesStart + (x * yRange);
		for(RB_Size y = 0; y < yRange; y++) {
			ret->coordIndexes[x][y] = RB_QUEUE_INDEX_UNQUEUED;
		}
	} 

	return ret;
}

// Frees a previously allocated assignmentQueue
void RB_freeAssignmentQueue(RB_AssignmentQueue* queue) {
	printf("Freeing RB_AssignmentQueue!\n");
	free(queue);
}

// Returns true if the queue is empty. Otherwise, returns false.
bool RB_isQueueEmpty(RB_AssignmentQueue* queue) {
	return queue->coordLen == 0;
}

bool RB_isQueueFull(RB_AssignmentQueue* queue) {
	return queue->coordLen == queue->maxCoordLen;
}

RB_Size RB_getQueueSize(RB_AssignmentQueue* queue) {
	return queue->coordLen;
}

RB_Size RB_getQueueCapacity(RB_AssignmentQueue* queue) {
	return queue->maxCoordLen;
}

// Chooses (using an implementation-specific method) a coord from the queue and returns it.
RB_Coord RB_chooseCoordFromAssignmentQueue(RB_AssignmentQueue* queue) {
	if(RB_isQueueEmpty(queue)) {
		fprintf(stderr, "AssignmentQueue is empty!!\n");
		return (RB_Coord) { .x = -1, .y = -1 };
	}

	RB_Size retIndex = ((RB_Size) rand()) % queue->coordLen;
	return queue->coords[retIndex];
}

bool RB_coordIsWithinQueueBounds(RB_AssignmentQueue* queue, RB_Coord coord) {
	return (
		coord.x >= 0
		&& coord.x < queue->xRange
		&& coord.y >= 0
		&& coord.y < queue->yRange
	);
}

bool RB_coordIsInQueue(RB_AssignmentQueue* queue, RB_Coord coord) {
	return (
		RB_coordIsWithinQueueBounds(queue, coord)
		&& queue->coordIndexes[coord.x][coord.y] != -1
	);
}

// If the coord is in the Queue, removes it.
void RB_removeCoordFromAssignmentQueue(RB_AssignmentQueue* queue, RB_Coord coord) {
	if(RB_coordIsInQueue(queue, coord)) { // If the coord is in the queue, the queue is guaranteed not to be empty
		RB_Size coordIndex = queue->coordIndexes[coord.x][coord.y];

		RB_Size lastIndex = queue->coordLen - 1;
		RB_Coord lastCoord = queue->coords[lastIndex];

		queue->coords[coordIndex] = lastCoord;
		queue->coordIndexes[lastCoord.x][lastCoord.y] = coordIndex;

		queue->coordIndexes[coord.x][coord.y] = RB_QUEUE_INDEX_UNQUEUED;

		queue->coordLen--;
	} else {
		fprintf(stderr, "Error removing coord from queue: Coord(%d, %d) is not in queue.\n", coord.x, coord.y);
	}
}

/*
- If the pixel is not already queued or assigned, adds the pixel to the queue (with the specified priority, if
the RB_AssignmentQueue implementation supports prioritization).
- If the pixel is already queued and the RB_AssignmentQueue implementation supports prioritization, this function may update
the pixel's priority, though that is not guaranteed.

Higher positive values for priorityIndex correspond to lower prioritization. A priorityIndex of 0 corresponds to the maximum
possible prioritization.
Negative values for priorityIndex correspond to the lowest possible prioritization.
*/
void RB_addCoordToAssignmentQueue(RB_AssignmentQueue* queue, RB_Coord toAdd, RB_Size priorityIndex) {
	if(RB_isQueueFull(queue)) {
		fprintf(stderr, "Error adding coord to queue: Queue is full!\n");
		return;
	}

	if(RB_coordIsWithinQueueBounds(queue, toAdd)) {
		if(RB_coordIsInQueue(queue, toAdd)) return;
		queue->coords[queue->coordLen] = toAdd;
		queue->coordIndexes[toAdd.x][toAdd.y] = queue->coordLen;
		queue->coordLen++;
	} else {
		fprintf(stderr, "Error adding coord to queue: Coord(%d, %d) is out of Bounds(%d, %d)!\n",
			toAdd.x, toAdd.y, queue->xRange, queue->yRange
		);
	}
}