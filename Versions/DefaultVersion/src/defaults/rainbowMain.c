#include "headers/RB_Main.h"
#include "headers/RB_AssignmentQueue.h"
#include "headers/RB_BasicTypes.h"
#include "headers/RB_ColorPool.h"
#include "headers/RB_PixelMap.h"
#include "headers/RB_Display.h"
//#include "headers/RB_Random.c"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>





RB_Config* RB_newConfig() {
	RB_Config* ret = malloc(sizeof(RB_Config));

	if(ret == NULL) {
		fprintf(stderr, "Error creating config: malloc failed!\n");
		return NULL;
	}

	ret->mapDimensionsSet = false;
	ret->colorResSet = false;
	ret->windowDimensionsSet = false;
	ret->seedSet = false;

	return ret;
}

void RB_freeConfig(RB_Config* config) {
	printf("Freeing config!\n");
	free(config);
}

bool checkColorResAndMapDimCompatibility(
	RB_ColorChannelSize rRes,
	RB_ColorChannelSize gRes,
	RB_ColorChannelSize bRes,
	RB_Size width,
	RB_Size height
) {
	if(rRes * gRes * bRes != width * height) {
		fprintf(
			stderr,
			"Error configuring rainbow! width * height must be equal to rRes * gRes * bRes!\n"
			"width * height == %d * %d == %d\n"
			"rRes * gRes * bRes == %d * %d * %d == %d\n",
			width, height, width * height,
			rRes, gRes, bRes, rRes * gRes * bRes
		);
		return false;
	}

	return true;
}

void RB_setColorResolution(RB_Config* config, RB_ColorChannelSize rRes, RB_ColorChannelSize gRes, RB_ColorChannelSize bRes) {
	if(
		config->mapDimensionsSet
		&& !checkColorResAndMapDimCompatibility(rRes, gRes, bRes, config->width, config->height)
	) {
		return;
	}

	if(
		rRes < 1 || rRes > RB_MAXIMUM_COLOR_CHANNEL_RESOLUTION
		|| gRes < 1 || gRes > RB_MAXIMUM_COLOR_CHANNEL_RESOLUTION
		|| bRes < 1 || bRes > RB_MAXIMUM_COLOR_CHANNEL_RESOLUTION
	) {
		fprintf(
			stderr,
			"Error setting color resolution! All resolutions must be between 1 and %d, inclusive.\n"
			"rRes = %d, gRes = %d, bRes = %d.\n",
			RB_MAXIMUM_COLOR_CHANNEL_RESOLUTION,
			rRes, gRes, bRes
		);
		return;
	}

	config->rRes = rRes;
	config->gRes = gRes;
	config->bRes = bRes;
	config->colorResSet = true;
}

void RB_setMapDimensions(RB_Config* config, RB_Size width, RB_Size height) {
	if(
		config->colorResSet
		&& !checkColorResAndMapDimCompatibility(config->rRes, config->gRes, config->bRes, width, height)
	) {
		return;
	}

	if(width < 1 || height < 1) {
		fprintf(
			stderr,
			"Error setting map dimensions! width and height must be at least 1!\n"
			"width = %d, height = %d\n",
			width, height
		);
		return;
	}

	if((RB_MAXIMUM_POSSIBLE_NUMBER_OF_COLORS / width) < height) {
		fprintf(
			stderr,
			"Error setting map dimensions! width * height must be less than %d!\n"
			"width = %d, height = %d\n",
			RB_MAXIMUM_POSSIBLE_NUMBER_OF_COLORS,
			width, height
		);
		return;
	}

	config->width = width;
	config->height = height;
	config->mapDimensionsSet = true;
}

void RB_setWindowDimensions(RB_Config* config, int wWidth, int wHeight) {
	if(wWidth < 1 || wHeight < 1) {
		fprintf(
			stderr,
			"Error setting window dimensions! width and height must be at least 1!\n"
			"width = %d, height = %d\n",
			wWidth, wHeight
		);
		return;
	}

	config->windowWidth = wWidth;
	config->windowHeight = wHeight;
	config->windowDimensionsSet = true;
}

void RB_setRandomSeed(RB_Config* config, uint32_t seed) {
	config->seed = seed;
	config->seedSet = true;
}


RB_Data* RB_init(RB_Config* config) {
	if(!config->colorResSet) {
		fprintf(stderr, "Error initializing rainbow: Color Resolution not set!\n");
		return NULL;
	}

	RB_Size width;
	RB_Size height;

	if(config->mapDimensionsSet) {
		width = config->width;
		height = config->height;
	} else {
		RB_Size numPixels = config->rRes * config->gRes * config->bRes;
		RB_Size potentialWidth = (RB_Size) sqrt(numPixels);
		while(potentialWidth * (numPixels / potentialWidth) != numPixels) {
			potentialWidth++;
		}
		width = potentialWidth;
		height = numPixels / potentialWidth;
	}

	int wWidth = config->windowDimensionsSet? config->windowWidth : width;
	int wHeight = config->windowDimensionsSet? config->windowHeight : height;

	uint32_t seed = config->seedSet? config->seed : time(NULL);

	RB_Size numPixels = height * width;

	printf(
		"Initializing Rainbow Image Generator.\n"
		"| Color Resolutions: %d, %d, %d.\n"
		"| Pixel Map Dimensions: %d, %d.\n"
		"| Total pixels: %d.\n"
		"| Display Window Dimensions: %d, %d.\n"
		"| Seed: %u.\n",
		config->rRes, config->gRes, config->bRes,
		width, height,
		numPixels,
		wWidth, wHeight,
		seed
	);

	srand(seed);
	//RB_RandomSetSeed(seed);

	RB_Data* ret = (RB_Data*) malloc(sizeof(RB_Data));

	if(ret == NULL) {
		fprintf(stderr, "Failed to initialize Rainbow Data!\n");
		return NULL;
	}

	ret->assignmentQueue = NULL;
	ret->colorPool = NULL;
	ret->pixelMap = NULL;
	ret->display = NULL;

	ret->config = (RB_Config) {
		.rRes = config->rRes,
		.gRes = config->gRes,
		.bRes = config->bRes,
		.width = width,
		.height = height,
		.windowWidth = wWidth,
		.windowHeight = wHeight,
		.seed = seed
	};
	
	ret->assignmentQueue = RB_createAssignmentQueue(numPixels, width, height);

	if(ret->assignmentQueue == NULL) {
		fprintf(stderr, "Failed to initialize Assignment Queue!\n");
		RB_free(ret);
		return NULL;
	}

	ret->colorPool = RB_createColorPool(config->rRes, config->gRes, config->bRes);

	if(ret->colorPool == NULL) {
		fprintf(stderr, "Failed to initialize Color Pool!\n");
		RB_free(ret);
		return NULL;
	}

	ret->pixelMap = RB_createPixelMap(width, height);

	if(ret->pixelMap == NULL) {
		fprintf(stderr, "Failed to initialize Pixel Map!\n");
		RB_free(ret);
		return NULL;
	}

	ret->display = RB_createDisplay(
		wWidth, wHeight,
		width, height,
		config->rRes, config->gRes, config->bRes
	);

	if(ret->display == NULL) {
		fprintf(stderr, "Failed to initialize Display!\n");
		RB_free(ret);
		return NULL;
	}

	return ret;
}

void RB_free(RB_Data* data) {
	if(data != NULL) {
		printf("Freeing RB_Data!\n");
		RB_freeAssignmentQueue(data->assignmentQueue);
		RB_freeColorPool(data->colorPool);
		RB_freePixelMap(data->pixelMap);
		RB_freeDisplay(data->display);
		free(data);
	}
}


RB_Color RB_getRandomColor(RB_Data* data) {
	return (RB_Color) {
		.r = rand() % data->config.rRes,
		.g = rand() % data->config.gRes,
		.b = rand() % data->config.bRes
	};
}

RB_Coord RB_getRandomCoord(RB_Data* data) {
	return (RB_Coord) {
		.x = rand() % data->config.width,
		.y = rand() % data->config.height
	};
}

void RB_setCoordColor(RB_Data* data, RB_Coord coord, RB_Color color) {
	RB_Pixel* toSet = RB_getPixel(data->pixelMap, coord);

	if(toSet->status == RB_PIXEL_SET) {
		fprintf(
			stderr,
			"Attempting to set Pixel at (%d,%d) even though it is already set!\n"
			"\tQueue size: %d\n",
			toSet->loc.x,
			toSet->loc.y,
			RB_getQueueSize(data->assignmentQueue)
		);
		return;
	}
	if(RB_coordIsInQueue(data->assignmentQueue, toSet->loc)) {
		RB_removeCoordFromAssignmentQueue(data->assignmentQueue, toSet->loc);	
	}
	RB_removeColorFromPool(data->colorPool, color);

	toSet->color = color;
	toSet->status = RB_PIXEL_SET;

	RB_setDisplayedPixelColor(data->display, toSet->loc, color);

	RB_addResultantCoordsToQueue(data->pixelMap, data->assignmentQueue, toSet->loc);
}

bool RB_generateNextPixel(RB_Data* data) {
	if(RB_isQueueEmpty(data->assignmentQueue)) {
		return false;
	}

	RB_Coord nextCoord = RB_chooseCoordFromAssignmentQueue(data->assignmentQueue);
	RB_Color preferredColor = RB_determinePreferredCoordColor(data->pixelMap, nextCoord);
	RB_Color idealColor = RB_findIdealAvailableColor(data->colorPool, preferredColor);

	// RB_Coord nextCoord = RB_chooseCoordFromAssignmentQueue(data->assignmentQueue);
	// RB_Color preferredColor_raw = RB_determinePreferredCoordColor(data->pixelMap, nextCoord);

	// RB_ColorChannelSum avg = (((RB_ColorChannelSum) preferredColor_raw.r) + preferredColor_raw.g + preferredColor_raw.b) / 3;

	// uint_fast64_t oMul = 2499;
	// uint_fast64_t oDiv = 2500;
	// uint_fast64_t uMul = 2500;
	// uint_fast64_t uDiv = 2499;

	// RB_Color preferredColor = (avg > (((uint_fast64_t)data->config.rRes * 1) / 4))? (RB_Color) {
	// 	.r = (((uint_fast64_t) preferredColor_raw.r) * oMul) / oDiv,
	// 	.g = (((uint_fast64_t) preferredColor_raw.g) * oMul) / oDiv,
	// 	.b = (((uint_fast64_t) preferredColor_raw.b) * oMul) / oDiv,
	// } : (RB_Color) {
	// 	.r = (((uint_fast64_t) preferredColor_raw.r) * uMul) / uDiv,
	// 	.g = (((uint_fast64_t) preferredColor_raw.g) * uMul) / uDiv,
	// 	.b = (((uint_fast64_t) preferredColor_raw.b) * uMul) / uDiv
	// };
	// RB_Color idealColor = RB_findIdealAvailableColor(data->colorPool, preferredColor);


	RB_setCoordColor(data, nextCoord, idealColor);

	return !RB_isQueueEmpty(data->assignmentQueue);
}