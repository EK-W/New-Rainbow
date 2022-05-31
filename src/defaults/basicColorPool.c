#include "headers/RB_ColorPool.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum {
	POOL_NODE_COLOR,
	POOL_NODE_OCTANT,
	POOL_NODE_EMPTY
} ColorPoolNodeType;

typedef uint_fast8_t NodeChildrenSize;
// length = 2^(dimensions_per_color)
#define RB_COLOR_POOL_NODE_NUM_CHILDREN 8

typedef struct ColorPoolOctant_s ColorPoolOctant;
typedef struct ColorPoolColorNode_s ColorPoolColorNode;

typedef struct ColorPoolNode_s {
	ColorPoolNodeType type;
	union {
		ColorPoolColorNode* colorNodePtr;
		ColorPoolOctant* octantNodePtr;
	};
} ColorPoolNode;

const ColorPoolNode emptyColorPoolNode = {
	.type = POOL_NODE_EMPTY,
	.octantNodePtr = NULL
};

typedef struct {
	// A pointer to the octant that contains the child.
	ColorPoolOctant* octant;
	// The index that the child is stored in in the parent's children array.
	NodeChildrenSize index;
} ChildNodeParentData;

struct ColorPoolColorNode_s {
	RB_Color color;
	bool isAvailable;

	ChildNodeParentData parentData;
};

struct ColorPoolOctant_s {
	// It is guaranteed that if a color's R, G, and B values are between those of minCorner and maxCorner,
	// that color will either be contained in this octant/this octant's descendants or it will not be contained
	// by *any* octant (for instance, if the color has already been removed from this one).
	RB_Color minCorner;
	RB_Color maxCorner;

	ChildNodeParentData parentData;

	ColorPoolNode children[RB_COLOR_POOL_NODE_NUM_CHILDREN];
	NodeChildrenSize numChildren;
};

typedef struct {
	// TODO: These first two fields probably don't need to use RB_Size
	RB_Size index;
	RB_Size divisor;
	// Note that the following sizes are in the coordinates of the layer, not global coordinates.
	RB_ColorChannelSize rSize;
	RB_ColorChannelSize gSize;
	RB_ColorChannelSize bSize;
	void* dataStart;
} OctantLayerMetaData;

struct RB_ColorPool_s {
	ColorPoolNode root;

	ColorPoolColorNode* colorNodes;
	ColorPoolOctant* octants;

	ColorPoolNode* nodeQueue;

	RB_ColorChannelSize rSize;
	RB_ColorChannelSize gSize;
	RB_ColorChannelSize bSize;
};

void printEntireTree(FILE* stream, ColorPoolNode node);
void printNode(FILE* stream, ColorPoolNode node);


void updateNodeParentData(ColorPoolNode node, ColorPoolOctant* newParent, NodeChildrenSize newIndex) {
	switch(node.type) {
		case POOL_NODE_EMPTY:
			fprintf(stderr, "Error: attempting to update parent data of an empty node!\n");
			return;
		case POOL_NODE_COLOR:
			node.colorNodePtr->parentData = (ChildNodeParentData) {
				.octant = newParent,
				.index = newIndex
			};
			return;
		case POOL_NODE_OCTANT:
			node.octantNodePtr->parentData = (ChildNodeParentData) {
				.octant = newParent,
				.index = newIndex
			};
			return;
	}
}

RB_Color getNodeMinCorner(ColorPoolNode node) {
	switch(node.type) {
		case POOL_NODE_OCTANT:
			return node.octantNodePtr->minCorner;
		case POOL_NODE_COLOR:
			return node.colorNodePtr->color;
		case POOL_NODE_EMPTY:
			fprintf(stderr, "Error: attempting to get the minimum corner of an empty node!\n");
			return (RB_Color) {
				.r = 0,
				.g = 0,
				.b = 0
			};
	}
}

RB_Color getNodeMaxCorner(ColorPoolNode node) {
	switch(node.type) {
		case POOL_NODE_OCTANT:
			return node.octantNodePtr->maxCorner;
		case POOL_NODE_COLOR:
			return node.colorNodePtr->color;
		case POOL_NODE_EMPTY:
			fprintf(stderr, "Error: attempting to get the maximum corner of an empty node!\n");
			return (RB_Color) {
				.r = 0,
				.g = 0,
				.b = 0
			};
	}
}

RB_Color calculateOctantMinCorner(ColorPoolOctant* octant) {
	RB_Color ret = getNodeMinCorner(octant->children[0]);

	for(NodeChildrenSize i = 1; i < octant->numChildren; i++) {
		RB_Color childMinCorner = getNodeMinCorner(octant->children[i]);

		if(childMinCorner.r < ret.r) {
			ret.r = childMinCorner.r;
		}
		if(childMinCorner.g < ret.g) {
			ret.g = childMinCorner.g;
		}
		if(childMinCorner.b < ret.b) {
			ret.b = childMinCorner.b;
		}
	}

	return ret; 
}

RB_Color calculateOctantMaxCorner(ColorPoolOctant* octant) {
	RB_Color ret = getNodeMaxCorner(octant->children[0]);

	for(NodeChildrenSize i = 1; i < octant->numChildren; i++) {
		RB_Color childMaxCorner = getNodeMaxCorner(octant->children[i]);

		if(childMaxCorner.r > ret.r) {
			ret.r = childMaxCorner.r;
		}
		if(childMaxCorner.g > ret.g) {
			ret.g = childMaxCorner.g;
		}
		if(childMaxCorner.b > ret.b) {
			ret.b = childMaxCorner.b;
		}
	}

	return ret;
}


void pruneNewNodeTree(ColorPoolNode node) {
	if(node.type == POOL_NODE_OCTANT) {
		ColorPoolOctant* oct = node.octantNodePtr;

		// Prune all children.
		for(NodeChildrenSize i = 0; i < oct->numChildren; i++) {
			pruneNewNodeTree(oct->children[i]);
		}

		// If this octant only has one child, replace it with the child.
		if(oct->numChildren == 1) {
			// Make sure that the child has the same min and max corners as the parent. This should always be the case,
			// assuming pruneNewNodeTree is only run immediately after the tree's creation.
			ColorPoolNode child = oct->children[0];
			RB_Color childMinCorner = getNodeMinCorner(child);
			RB_Color childMaxCorner = getNodeMaxCorner(child);
			if((!RB_colorsAreEqual(childMinCorner, oct->minCorner)) || (!RB_colorsAreEqual(childMaxCorner, oct->maxCorner))) {
				fprintf(
					stderr,
					"Error pruning the new ColorPool tree! The parent to be pruned has different bounds than its child!\n"
					"This should never happen!\n"
				);
			}

			if(oct->parentData.octant != NULL) {
				oct->parentData.octant->children[oct->parentData.index] = child;
				updateNodeParentData(child, oct->parentData.octant, oct->parentData.index);
			}
		}
	}
}

size_t calculateMaximumOctants(RB_ColorChannelSize rSize, RB_ColorChannelSize gSize, RB_ColorChannelSize bSize) {
	size_t ret = 0;
	RB_Size levelRSize = rSize;
	RB_Size levelGSize = gSize;
	RB_Size levelBSize = bSize;
	RB_Size octantsOnLevel;

	do {
		levelRSize = (levelRSize + 1) / 2;
		levelGSize = (levelGSize + 1) / 2;
		levelBSize = (levelBSize + 1) / 2;
		octantsOnLevel = levelRSize * levelGSize * levelBSize;
		ret += octantsOnLevel;
	} while(octantsOnLevel != 1);

	return ret;
}

RB_Size getDataPosition(
	RB_ColorChannel r,
	RB_ColorChannel g,
	RB_ColorChannel b,
	RB_ColorChannelSize gSize,
	RB_ColorChannelSize bSize
) {
	return (((r * gSize) + g) * bSize) + b;
}

ColorPoolNode getDataFromLayer(
	OctantLayerMetaData layerDat,
	RB_ColorChannelSize layerR,
	RB_ColorChannelSize layerG,
	RB_ColorChannelSize layerB
) {
	if(layerR >= layerDat.rSize || layerG >= layerDat.gSize || layerB >= layerDat.bSize) {
		return emptyColorPoolNode;
	}

	RB_Size dataPosition = getDataPosition(layerR, layerG, layerB, layerDat.gSize, layerDat.bSize);

	if(layerDat.index == 0) {
		// This means its the color layer
		ColorPoolColorNode* colorNodePtr = ((ColorPoolColorNode*) layerDat.dataStart) + dataPosition;
		return (ColorPoolNode) {
			.type = POOL_NODE_COLOR,
			.colorNodePtr = colorNodePtr
		};
	} else {
		// This means its an octant layer
		ColorPoolOctant* octPtr = ((ColorPoolOctant*) layerDat.dataStart) + dataPosition;
		return (ColorPoolNode) {
			.type = POOL_NODE_OCTANT,
			.octantNodePtr = octPtr
		};
	}
}

RB_ColorPool* RB_createColorPool(RB_ColorChannelSize rSize, RB_ColorChannelSize gSize, RB_ColorChannelSize bSize) {
	RB_ColorPool* ret = (RB_ColorPool*) malloc(sizeof(RB_ColorPool));
	
	if(ret == NULL) {
		return NULL;
	}

	ret->rSize = rSize;
	ret->gSize = gSize;
	ret->bSize = bSize;
	ret->colorNodes = NULL;
	ret->octants = NULL;
	ret->nodeQueue = NULL;


	// ALLOCATE THE NODE QUEUE
	// TODO: I know for sure that the nodeQueue will never need to be larger than numPixels,
	// but I'm pretty sure it's possible to figure out an even smaller upper bound.
	ret->nodeQueue = (ColorPoolNode*) malloc(sizeof(ColorPoolNode) * rSize * gSize * bSize);
	if(ret->nodeQueue == NULL) {
		RB_freeColorPool(ret);
		return NULL;
	}

	// DEAL WITH COLORS
	ret->colorNodes = (ColorPoolColorNode*) malloc(sizeof(ColorPoolColorNode) * rSize * gSize * bSize);

	if(ret->colorNodes == NULL) {
		RB_freeColorPool(ret);
		return NULL;
	}


	RB_Size colorIndex = 0;
	for(RB_ColorChannelSize r = 0; r < rSize; r++) {
		for(RB_ColorChannelSize g = 0; g < gSize; g++) {
			for(RB_ColorChannelSize b = 0; b < bSize; b++) {
				RB_Color col = {
					.r = (RB_ColorChannel) r,
					.g = (RB_ColorChannel) g,
					.b = (RB_ColorChannel) b
				};
				ret->colorNodes[colorIndex] = (ColorPoolColorNode) {
					.color = col,
					.isAvailable = true,
					.parentData = {
						.octant = NULL
					}
				};
				colorIndex++;
			}
		}
	}


	// DEAL WITH OCTANTS
	size_t maxOctants = calculateMaximumOctants(rSize, gSize, bSize);
	ret->octants = (ColorPoolOctant*) malloc(sizeof(ColorPoolOctant) * maxOctants);

	if(ret->octants == NULL) {
		RB_freeColorPool(ret);
		return NULL;
	}

	RB_Size octantDataIndex = 0;

	OctantLayerMetaData lastLayer = {
		.index = 0,
		.divisor = 1,
		.rSize = rSize,
		.gSize = gSize,
		.bSize = bSize,
		.dataStart = ret->colorNodes
	};

	do {
		OctantLayerMetaData layer = {
			.index = lastLayer.index + 1,
			.divisor = lastLayer.divisor * 2,
			.rSize = (lastLayer.rSize + 1) / 2,
			.gSize = (lastLayer.gSize + 1) / 2,
			.bSize = (lastLayer.bSize + 1) / 2,
			.dataStart = (ret->octants + octantDataIndex)
		};

		for(RB_ColorChannelSize layerR = 0; layerR < layer.rSize; layerR++) {
			for(RB_ColorChannelSize layerG = 0; layerG < layer.gSize; layerG++) {
				for(RB_ColorChannelSize layerB = 0; layerB < layer.bSize; layerB++) {
					if(octantDataIndex >= maxOctants) {
						fprintf(stderr, "Too many octants are being generated!\n");
						RB_freeColorPool(ret);
						return NULL;
					}

					ColorPoolOctant* newOct = ret->octants + octantDataIndex;
					octantDataIndex++;

					// The minimum r, g, and b of this octant translated into the global coordinates
					RB_Color globalColor = {
						.r = layerR * layer.divisor,
						.g = layerG * layer.divisor,
						.b = layerB * layer.divisor
					};

					newOct->parentData.octant = NULL;

					newOct->minCorner = globalColor;
					newOct->maxCorner = newOct->minCorner;
					newOct->numChildren = 0;

					// The minimum r, g, and b of this octant translated into the coordinates of the previous layer.
					// minLLay stands for minimum last layer
					RB_ColorChannelSize minLLayR = layerR * 2;
					RB_ColorChannelSize minLLayG = layerG * 2;
					RB_ColorChannelSize minLLayB = layerB * 2;

					for(RB_ColorChannelSize lLayR = minLLayR; lLayR < (minLLayR + 2); lLayR++) {
						for(RB_ColorChannelSize lLayG = minLLayG; lLayG < (minLLayG + 2); lLayG++) {
							for(RB_ColorChannelSize lLayB = minLLayB; lLayB < (minLLayB + 2); lLayB++) {
								ColorPoolNode child = getDataFromLayer(lastLayer, lLayR, lLayG, lLayB);

								// If the node we're looking at isn't valid (for instance if it's out of bounds), skip it.
								if(child.type == POOL_NODE_EMPTY) {
									continue;
								}

								updateNodeParentData(child, newOct, newOct->numChildren);

								newOct->children[newOct->numChildren] = child;
								newOct->numChildren++;
							}
						}		
					}

					// calculate newOct's maximum corner
					newOct->maxCorner = calculateOctantMaxCorner(newOct);

					// Make sure the rest of the children are empty nodes.
					// This step arguably isn't necessary, but I'm doing it anyway.
					for(NodeChildrenSize i = newOct->numChildren; i < RB_COLOR_POOL_NODE_NUM_CHILDREN; i++) {
						newOct->children[i] = emptyColorPoolNode;
					}
				}
			}
		}

		lastLayer = layer;
	} while(lastLayer.rSize > 1 || lastLayer.gSize > 1 || lastLayer.bSize > 1);

	// set the root node
	ret->root = (ColorPoolNode) {
		.type = POOL_NODE_OCTANT,
		.octantNodePtr = (ColorPoolOctant*) lastLayer.dataStart
	};

	//prune the tree
	pruneNewNodeTree(ret->root);

	return ret;
}

// Frees a previously allocated color pool
void RB_freeColorPool(RB_ColorPool* pool) {
	if(pool == NULL) {
		return;
	}

	printf("Freeing RB_ColorPool!\n");

	free(pool->colorNodes);
	pool->colorNodes = NULL;

	free(pool->octants);
	pool->octants = NULL;

	free(pool->nodeQueue);
	pool->nodeQueue = NULL;

	free(pool);
}

RB_ColorChannel getChannelValueWithinBoundaries(RB_ColorChannel minVal, RB_ColorChannel maxVal, RB_ColorChannel toBound) {
	RB_ColorChannel lowerBounded = toBound < minVal? minVal : toBound;
	return lowerBounded > maxVal? maxVal : lowerBounded;
}


RB_ColorSquareDistance getSquareDistance(RB_Color a, RB_Color b) {
	RB_ColorChannelDifference dR = a.r - b.r;
	RB_ColorChannelDifference dG = a.g - b.g;
	RB_ColorChannelDifference dB = a.b - b.b;

	return (
		((RB_ColorSquareDistance) dR * dR)
		+ ((RB_ColorSquareDistance) dG * dG)
		+ ((RB_ColorSquareDistance) dB * dB)
	);
}

// RB_ColorSquareDistance getSquareDistance(RB_Color aRaw, RB_Color bRaw) {
// 	HSVColor a = rgbToHSV(
		
// 	)

// 	RB_ColorChannelDifference dR = a.r - b.r;
// 	RB_ColorChannelDifference dG = a.g - b.g;
// 	RB_ColorChannelDifference dB = a.b - b.b;

// 	return (
// 		((RB_ColorSquareDistance) dR * dR)
// 		+ ((RB_ColorSquareDistance) dG * dG)
// 		+ ((RB_ColorSquareDistance) dB * dB)
// 	);
// }

// Using only the bounds of the octant and not the actual elements inside of it, what's the closest color
// that this octant could possibly contain?
RB_ColorSquareDistance getBlindClosestDistance(ColorPoolNode node, RB_Color color) {
	switch(node.type) {
		case POOL_NODE_EMPTY:
			fprintf(stderr, "Attemping to get the closest distance to an empty node!\n");
			return ~((RB_ColorSquareDistance) 0); // This should return the maximum possible value
		case POOL_NODE_OCTANT: {
			ColorPoolOctant* octant = node.octantNodePtr;
			RB_Color closest = (RB_Color) {
				.r = getChannelValueWithinBoundaries(octant->minCorner.r, octant->maxCorner.r, color.r),
				.g = getChannelValueWithinBoundaries(octant->minCorner.g, octant->maxCorner.g, color.g),
				.b = getChannelValueWithinBoundaries(octant->minCorner.b, octant->maxCorner.b, color.b),
			};
			return getSquareDistance(color, closest);
		}
		case POOL_NODE_COLOR: {
			ColorPoolColorNode* colorNodePtr = node.colorNodePtr;
			return getSquareDistance(color, colorNodePtr->color);
		}
	}
}

RB_ColorSquareDistance getBlindWorstDistance(ColorPoolNode node, RB_Color color) {
	switch(node.type) {
		case POOL_NODE_EMPTY:
			fprintf(stderr, "Attemping to get the worst distance to an empty node!\n");
			return 0;
		case POOL_NODE_OCTANT: {
			ColorPoolOctant* octant = node.octantNodePtr;
			RB_Color furthestColor = {
				.r = ((color.r * 2) - (octant->minCorner.r + octant->maxCorner.r)) > 0?
					octant->minCorner.r : octant->maxCorner.r,
				.g = ((color.g * 2) - (octant->minCorner.g + octant->maxCorner.g)) > 0?
					octant->minCorner.g : octant->maxCorner.g,
				.b = ((color.b * 2) - (octant->minCorner.b + octant->maxCorner.b)) > 0?
					octant->minCorner.b : octant->maxCorner.b
			};
			return getSquareDistance(color, furthestColor);
		}
		case POOL_NODE_COLOR: {
			ColorPoolColorNode* colorNodePtr = node.colorNodePtr;
			return getSquareDistance(color, colorNodePtr->color);
		}
	}
}


/*
Basic algorithm (figured out by me!):
1) Add the root node to the "node queue." At the start, it will be the only node in the queue.
2) Initialize minWorstCase to the worst case of the root node, I guess.
3) Iterate through each node in the node queue.
	3.1) If the node's best case is greater than minWorstCase, remove it from the queue.
	3.2) If the node is an octant node, iterate through its children.
		3.2.1) If the child's best case is greater than minWorstCase, do nothing.
		3.2.2) If the child's best case is less than or equal to minWorstCase, add it to the node queue.
		3.2.3) If the child's worst case is less than minWorstCase, set minWorstCase to the child's worst case value.
	3.3) If the node is an octant node, remove it from the queue.
4) If, during step 3, minWorstCase was updated or an octant was added to the queue, repeat step 3
5) At this point, we know that the node queue only contains ideal colors. Choose one and return.
*/
RB_Color RB_findIdealAvailableColor(RB_ColorPool* colorPool, RB_Color desired) {
	ColorPoolNode* nodeQueue = colorPool->nodeQueue;
	RB_Size nodeQueueSize = 1;
	RB_Size nodeQueueNextSize = 0;

	if(colorPool->root.type == POOL_NODE_EMPTY) {
		fprintf(stderr, "Error: attempting to find ideal available color in an empty color pool!");
		return (RB_Color) {
			.r = 0,
			.g = 0,
			.b = 0
		};
	}

	nodeQueue[0] = colorPool->root;
	RB_ColorSquareDistance minWorstCase = getBlindWorstDistance(colorPool->root, desired);
	bool shouldIterateAgain = true;

	while(shouldIterateAgain) {
		shouldIterateAgain = false;

		for(RB_Size i = 0; i < nodeQueueSize; i++) {
			ColorPoolNode node = nodeQueue[i];
			RB_ColorSquareDistance nodeBestCase = getBlindClosestDistance(node, desired);

			if(nodeBestCase <= minWorstCase) {
				if(node.type == POOL_NODE_OCTANT) {
					ColorPoolOctant* octantNode = node.octantNodePtr;
					for(NodeChildrenSize j = 0; j < octantNode->numChildren; j++) {
						ColorPoolNode child = octantNode->children[j];

						RB_ColorSquareDistance childBestCase = getBlindClosestDistance(child, desired);
						RB_ColorSquareDistance childWorstCase = getBlindWorstDistance(child, desired);

						if(childBestCase <= minWorstCase) {
							// add child to node queue

							if(nodeQueueNextSize <= i) {
								// If there's room at the start of the queue, add it there
								// We're able to overwrite the node at position `i` because we already have its value saved
								// in the node variable, and because it's an octant, it's going to be deleted anyway.

								if(child.type == POOL_NODE_OCTANT) {
									// Despite my algorithm saying that we should iterate again whenever we add
									// an octant to the queue, the way we're handling adding nodes to the queue
									// means that octants added to the end of the queue will be handled as though
									// they were part of this iteration, so we don't need to iterate again for them.
									// That's why we're only setting shouldIterateAgain to true when we're adding
									// an octant to the beginning of the queue.
									shouldIterateAgain = true;
								}

								nodeQueue[nodeQueueNextSize] = child;
								nodeQueueNextSize++;


							} else {
								// If there's no room at the start of the queue, add it to the end.
								nodeQueue[nodeQueueSize] = child;
								nodeQueueSize++;
							}
						}
						if(childWorstCase < minWorstCase) {
							shouldIterateAgain = true;
							minWorstCase = childWorstCase;
						}
					}
				} else {
					// At this point, we know the node is a color that meets the threshold for being kept
					// We know that nodeQueueNextSize must be less than or equal to `i`, because `i` has necessarily
					// increased by one since the last time nodeQueueNextSize could potentially have been equal to it.
					
					nodeQueue[nodeQueueNextSize] = node;
					nodeQueueNextSize++;	
				}
			}
		}

		nodeQueueSize = nodeQueueNextSize;
		nodeQueueNextSize = 0;
	}

	// So, at this point, the node queue should only contain ideal colors. 
	RB_Size colorNodeIndex = ((RB_Size) rand()) % nodeQueueSize;
	ColorPoolNode nodeToReturn = nodeQueue[colorNodeIndex];
	RB_Color ret = nodeToReturn.colorNodePtr->color;

	return ret;
}

bool RB_colorIsAvailableInPool(RB_ColorPool* pool, RB_Color toFind) {
	if(toFind.r >= pool->rSize || toFind.g >= pool->gSize || toFind.b >= pool->bSize) {
		return false;
	}
	RB_Size colorNodeIndex = getDataPosition(toFind.r, toFind.g, toFind.b, pool->gSize, pool->bSize);

	return pool->colorNodes[colorNodeIndex].isAvailable;
}

bool RB_removeColorFromPool(RB_ColorPool* pool, RB_Color toRemove) {
	if(toRemove.r >= pool->rSize || toRemove.g >= pool->gSize || toRemove.b >= pool->bSize) {
		return false;
	}

	RB_Size colorNodeIndex = getDataPosition(toRemove.r, toRemove.g, toRemove.b, pool->gSize, pool->bSize);
	ColorPoolColorNode* colorNode = &(pool->colorNodes[colorNodeIndex]);

	// If colorNode has already been removed, it can't be removed again. 
	if(!colorNode->isAvailable) {
		return false;
	}

	colorNode->isAvailable = false;

	// If the colorNode has no parent, then it is presumably the root. Set the root to empty and return.
	if(colorNode->parentData.octant == NULL) {
		printf("Removing last color from the pool.\n");
		pool->root = emptyColorPoolNode;
		return true;
	}

	// Remove the colorNode from its parent
	ColorPoolOctant* octant = colorNode->parentData.octant;
	octant->numChildren--;
	octant->children[colorNode->parentData.index] = octant->children[octant->numChildren];
	updateNodeParentData(octant->children[colorNode->parentData.index], octant, colorNode->parentData.index);

	// if the parent now only has one node, replace it with its one node.
	if(octant->numChildren == 1) {
		if(octant->parentData.octant == NULL) {
			// If the octant is the root node, replace the root node.
			pool->root = octant->children[0];
		} else {
			octant->parentData.octant->children[octant->parentData.index] = octant->children[0];
		}
		updateNodeParentData(octant->children[0], octant->parentData.octant, octant->parentData.index);

		// This octant no longer exists. Advance to its parent octant.
		octant = octant->parentData.octant;
	}

	// Update the bounds of the ancestor octants.
	while(octant != NULL) {
		RB_Color oldMinCorner = octant->minCorner;
		RB_Color oldMaxCorner = octant->maxCorner;

		octant->minCorner = calculateOctantMinCorner(octant);
		octant->maxCorner = calculateOctantMaxCorner(octant);

		// If the bounds do not change, they won't change for the parent, either. The updating-bounds phase is over.
		if(RB_colorsAreEqual(octant->minCorner, oldMinCorner) && RB_colorsAreEqual(octant->maxCorner, oldMaxCorner)) {
			break;
		}

		octant = octant->parentData.octant;
	}

	return true;
}

void printNode(FILE* stream, ColorPoolNode node) {
	switch(node.type) {
		case POOL_NODE_EMPTY:
			fprintf(stream, "Empty Node");
			break;
		case POOL_NODE_COLOR: {
			RB_Color col = node.colorNodePtr->color;
			fprintf(stream, "Color Node(%u,%u,%u)", col.r, col.g, col.b);
			break;
		}
		case POOL_NODE_OCTANT: {
			ColorPoolOctant* oct = node.octantNodePtr;
			fprintf(
				stream,
				"Octant Node(%u children) min = (%u,%u,%u) max = (%u,%u,%u)",
				oct->numChildren,
				oct->minCorner.r,
				oct->minCorner.g,
				oct->minCorner.b,
				oct->maxCorner.r,
				oct->maxCorner.g,
				oct->maxCorner.b
			);
			break;
		}
	}
}

void printEntireTreeRecursive(FILE* stream, ColorPoolNode node, int depth) {
	for(int i = 0; i < depth; i++) {
		fprintf(stream, "   |");
	}
	printNode(stream, node);
	fprintf(stream, "\n");

	if(node.type != POOL_NODE_OCTANT) {
		return;
	}

	ColorPoolOctant* octant = node.octantNodePtr; 

	for(NodeChildrenSize i = 0; i < octant->numChildren; i++) {
		printEntireTreeRecursive(stream, octant->children[i], depth + 1);
	}
}

void printEntireTree(FILE* stream, ColorPoolNode node) {
	printEntireTreeRecursive(stream, node, 0);
}