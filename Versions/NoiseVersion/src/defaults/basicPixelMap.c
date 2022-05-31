#include "headers/RB_PixelMap.h"
#include "headers/RB_AssignmentQueue.h"
#include <stdlib.h>
#include <stdio.h>

// Added for the random unset pixel experiment
#include "headers/RB_Main.h"

struct RB_PixelMap_s {
	RB_Pixel** pixels;
	RB_Size width;
	RB_Size height;

	// Added for the random unset pixel experiment
	RB_Coord* unsetCoords;
	RB_Size* coordIndexes;
	RB_Size numUnsetCoords;
};

// allocates a pixel map with the specified dimensions
RB_PixelMap* RB_createPixelMap(RB_Size width, RB_Size height) {
	RB_PixelMap* ret = (RB_PixelMap*) malloc(
		sizeof(RB_PixelMap)
		+ (sizeof(RB_Pixel*) * width)
		+ (sizeof(RB_Pixel) * width * height)
	);

	if(ret == NULL) {
		return NULL;
	}

	ret->width = width;
	ret->height = height;

	ret->pixels = (RB_Pixel**) (ret + 1);
	RB_Pixel* pixelData = (RB_Pixel*) (ret->pixels + width);

	for(int x = 0; x < width; x++) {
		ret->pixels[x] = pixelData + (x * height);

		for(int y = 0; y < height; y++) {
			ret->pixels[x][y] = (RB_Pixel) {
				.loc = { .x = x, .y = y },
				.color = { .r = 0, .g = 0, .b = 0 },
				.status = RB_PIXEL_BLANK
			};
		}
	}

	// Added for the random unset pixel experiment
	ret->unsetCoords = malloc(sizeof(RB_Coord) * width * height);
	ret->coordIndexes = malloc(sizeof(RB_Size) * width * height);
	if(ret->unsetCoords == NULL || ret->coordIndexes == NULL) {
		return NULL;
	}
	ret->numUnsetCoords = width * height;
	for(RB_Size x = 0; x < width; x++) {
		for(RB_Size y = 0; y < height; y++) {
			RB_Size index = (x * height) + y;
			ret->unsetCoords[index] = (RB_Coord) { .x = x, .y = y };
			ret->coordIndexes[index] = index;
		}
	}

	return ret;
}

// deallocates the pixel map
void RB_freePixelMap(RB_PixelMap* map) {
	printf("Freeing RB_PixelMap!\n");
	free(map->unsetCoords);
	free(map->coordIndexes);
	free(map);
}

RB_Coord toRealCoord(RB_PixelMap* map, RB_Coord coord) {
	return coord;
	// int flipX = (coord.x < 0 || coord.x >= map->width);

	// RB_Coord ret = (RB_Coord) {
	// 	.x = ((coord.x % map->width) + map->width) % map->width,
	// 	//.y = ((coord.y % map->height) + map->height) % map->height
	// 	.y = coord.y
	// };
	// if(flipX) {
	// 	ret.y = map->height - 1 - ret.y;
	// }
	// return ret;
}

// returns the pixel that the coord maps to, or NULL if the coord does not map to a pixel.
RB_Pixel* RB_getPixel(RB_PixelMap* map, RB_Coord coord) {
	if(coord.x < 0 || coord.x >= map->width || coord.y < 0 || coord.y >= map->height) {
		return NULL;
	}

	return &(map->pixels[coord.x][coord.y]);
}

// Determines, based on the current state of the pixelMap, the preferred color for the specified coordinate.
RB_Color RB_determinePreferredCoordColor(RB_PixelMap* pixelMap, RB_Coord coord) {
	const static RB_Size numCols = 6;
	const static RB_Size numRows = 6;
	static RB_Color cMap[numCols][numRows] = {0};
	RB_Coord cellSize = {
		(pixelMap->width + 1) / numCols,
		(pixelMap->height + 1) / numRows
	};

	RB_Coord gridCell = {
		.x = (coord.x * numCols) / pixelMap->width,
		.y = (coord.y * numRows) / pixelMap->height
	};

	RB_Coord cellPosition = {
		coord.x - (gridCell.x * cellSize.x + (cellSize.x / 2)),
		coord.y - (gridCell.y * cellSize.y + (cellSize.y / 2))
	};


	// if(
	// 	!RB_colorsAreEqual(cMap[gridCell.x][gridCell.y], (RB_Color) { 0, 0, 0 })
	// 	&& ((cellPosition.x * cellPosition.x) + (cellPosition.y * cellPosition.y)) < ((cellSize.x * cellSize.x) / 6)
	// 	//|| ((gridCell.x % 2) == (gridCell.y % 2)))
	// 	// && ((gridCell.x >= 5 && gridCell.x <= 6) || (gridCell.y >= 5 && gridCell.y <= 6))
	// ) {
	// 	//return cMap[gridCell.x][gridCell.y];
	// }

	const RB_Size searchRadius = 6;
	//const RB_Size searchRadius = ((gridCell.x >= 5 && gridCell.x <= 6) || (gridCell.y >= 5 && gridCell.y <= 6))? 2 : 1;
	RB_Size minX = ((coord.x - searchRadius) < 0)? 0 : coord.x - searchRadius;
	RB_Size maxX = ((coord.x + searchRadius) >= pixelMap->width)? pixelMap->width - 1 : coord.x + searchRadius;
	RB_Size minY = ((coord.y - searchRadius) < 0)? 0 : coord.y - searchRadius;
	RB_Size maxY = ((coord.y + searchRadius) >= pixelMap->height)? pixelMap->height - 1 : coord.y + searchRadius;

	// RB_Size minX = coord.x - searchRadius;
	// RB_Size maxX = coord.x + searchRadius;
	// RB_Size minY = coord.y - searchRadius;
	// RB_Size maxY = coord.y + searchRadius;


	// A ColorChannelSum is garanteed to be able to hold the sum of up to 256 colorChannel values
	// Because we're adding half of numNeighbors, this will have a lower capacity.
	// That doesn't matter here, though.
	// RB_ColorChannelSum rSum = 0;
	// RB_ColorChannelSum gSum = 0;
	// RB_ColorChannelSum bSum = 0;

	RB_Size rSum = 0;
	RB_Size gSum = 0;
	RB_Size bSum = 0;

	uint_fast32_t numNeighbors = 0;




	for(RB_Size x = minX; x <= maxX; x++) {
		for(RB_Size y = minY; y <= maxY; y++) {
			RB_Coord newCoord = { .x = x, .y = y };

			RB_Pixel* neighborPixel = RB_getPixel(pixelMap, newCoord);

			if(neighborPixel == NULL) {
				fprintf(stderr,
					"Somehow, determinePreferredCoordColor has encountered a NULL pixel"
					"even though that shouldn't be possible?\nThe pixel is at %d %d.\n",
					x, y
				);
				continue;
			}

			if(neighborPixel->status != RB_PIXEL_SET) continue;

			numNeighbors++;
			rSum += neighborPixel->color.r;
			gSum += neighborPixel->color.g;
			bSum += neighborPixel->color.b;

			// if(x > 0 && y > 0 && x == y) {
			// 	int weight = 10;
			// 	numNeighbors += weight;
			// 	rSum += neighborPixel->color.r * weight;
			// 	gSum += neighborPixel->color.g * weight;
			// 	bSum += neighborPixel->color.b * weight;
			// }
		}
	}

	int rRes = 64;
	int gRes = 64;
	int bRes = 64;

	// Added for the random unset pixel experiment
	if(numNeighbors == 0) {
		if(
			!RB_colorsAreEqual(cMap[gridCell.x][gridCell.y], (RB_Color) { 0, 0, 0 })
			&& 0
		) {
			

			RB_Color ret = cMap[gridCell.x][gridCell.y];
			int maxOffset = 4;
			int randMod = ((maxOffset * 2) + 1);
			int newR = ((int) ret.r) + ((rand() % randMod) - maxOffset);
			int newG = ((int) ret.g) + ((rand() % randMod) - maxOffset);
			int newB = ((int) ret.b) + ((rand() % randMod) - maxOffset);
			ret.r = (newR < 0)? 0 : ((newR >= rRes)? rRes - 1 : newR);
			ret.g = (newG < 0)? 0 : ((newG >= gRes)? gRes - 1 : newG);
			ret.b = (newB < 0)? 0 : ((newB >= bRes)? bRes - 1 : newB);
			return ret;
		} else {
			RB_Color ret = {
				.r = rand() % rRes,
				.g = rand() % gRes,
				.b = rand() % bRes
			};
			cMap[gridCell.x][gridCell.y] = ret;
			return ret;
		}
	}

	// if(
	// 	!RB_colorsAreEqual(cMap[gridCell.x][gridCell.y], (RB_Color) { 0, 0, 0 })
	// 	&& ((cellPosition.x * cellPosition.x) + (cellPosition.y * cellPosition.y)) < ((cellSize.x * cellSize.x) / 6)
	// 	// && ((gridCell.x >= 5 && gridCell.x <= 6) || (gridCell.y >= 5 && gridCell.y <= 6))
	// ) {
	// 	unsigned int weight = 8;
	// 	rSum += cMap[gridCell.x][gridCell.y].r * weight;
	// 	gSum += cMap[gridCell.x][gridCell.y].g * weight;
	// 	bSum += cMap[gridCell.x][gridCell.y].b * weight;
	// 	numNeighbors += weight;
	// 	//return cMap[gridCell.x][gridCell.y];
	// }

	uint_fast32_t halfNumNeighbors = numNeighbors / 2;
	// By adding half of numNeighbors, hopefully the sums will round instead of floor.
	RB_ColorChannelSum retR = (rSum + halfNumNeighbors) / numNeighbors;
	RB_ColorChannelSum retG = (gSum + halfNumNeighbors) / numNeighbors;
	RB_ColorChannelSum retB = (bSum + halfNumNeighbors) / numNeighbors;

	// RB_ColorChannelSum avg = (retR + retG + retB) / 3;
	// retR = avg;
	// retG = avg;
	// retB = avg;


	// if(
	// 	RB_colorsAreEqual(cMap[gridCell.x][gridCell.y], (RB_Color) { 0, 0, 0 })
	// 	&& ((cellPosition.x * cellPosition.x) + (cellPosition.y * cellPosition.y)) < ((cellSize.x * cellSize.x) / 4)
	// ) {
	// 	cMap[gridCell.x][gridCell.y] = (RB_Color) {
	// 		.r = retR,
	// 		.g = retG,
	// 		.b = retB
	// 	};
	// }

	

	return (RB_Color) {
		.r = retR,
		.g = retG,
		.b = retB
	};
}


// Add cords to the queue in an implementation-defined pattern relative to the given coord
void RB_addResultantCoordsToQueue(RB_PixelMap* map, RB_AssignmentQueue* queue, RB_Coord center) {
	// Added for the random unset pixel experiment
	RB_Size coordIndex = map->coordIndexes[(center.x * map->height) + center.y];
	RB_Coord lastCoord = map->unsetCoords[map->numUnsetCoords - 1];
	map->unsetCoords[coordIndex] = lastCoord;
	map->coordIndexes[(center.x * map->height) + center.y] = -1;
	map->coordIndexes[(lastCoord.x * map->height) + lastCoord.y] = coordIndex;
	map->numUnsetCoords--;

	// static RB_Coord lastRandomCoord = { -1, -1 };

	// if(map->numUnsetCoords > 0) {
	// 	RB_Pixel* lastCoordPixel = RB_getPixel(map, lastRandomCoord);

	// 	if(
	// 		lastCoordPixel == NULL
	// 		|| lastCoordPixel->status != RB_PIXEL_BLANK
	// 	) {
	// 		RB_Coord coordToAdd = map->unsetCoords[rand() % map->numUnsetCoords];
	// 		RB_Pixel* toAdd = RB_getPixel(map, coordToAdd);
	// 		RB_addCoordToAssignmentQueue(queue, toAdd->loc, -1);
	// 		lastRandomCoord = coordToAdd;
	// 	}
	// }


	if(map->numUnsetCoords > 0) {
		RB_Coord coordToAdd = map->unsetCoords[rand() % map->numUnsetCoords];
		RB_Pixel* toAdd = RB_getPixel(map, coordToAdd);
		RB_addCoordToAssignmentQueue(queue, toAdd->loc, -1);
	}
	
	// int addRadius = 1;

	// for(RB_Size dx = -addRadius; dx <= addRadius; dx++) {
	// 	for(RB_Size dy = -addRadius; dy <= addRadius; dy++) {
	// 		if(dx == 0 && dy == 0) continue;

	// 		// if(dx * dx == dy * dy) {
	// 		// 	continue;
	// 		// }

	// 		RB_Pixel* toAdd = RB_getPixel(map, (RB_Coord) { .x = center.x + dx, .y = center.y + dy });
	// 		if(toAdd == NULL) continue;
	// 		if(toAdd->status != RB_PIXEL_BLANK) continue;

	// 		RB_addCoordToAssignmentQueue(queue, toAdd->loc, -1);
	// 	}
	// }
}