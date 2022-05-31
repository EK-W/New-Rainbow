#include "headers/RB_Display.h"
#include "SDL.h"
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

struct RB_Display_s {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;

	double framesPerSecond;
	double secondsPerFrame;
	clock_t lastFrameProcTime;

	RB_ColorChannelSize rRes;
	RB_ColorChannelSize gRes;
	RB_ColorChannelSize bRes;
};

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
) {
	// Error checking.
	if(wWidth <= 0 || wHeight <= 0) {
		fprintf(stderr,
			"Error in RB_createDisplay: window width and height must be positive!\n"
			"\tWindow width: %d, Window height: %d\n",
			pWidth, pHeight
		);
		return NULL;
	}

	// Allocating the struct and initializing its values;
	RB_Display* ret = (RB_Display*) malloc(sizeof(RB_Display));

	if(ret == NULL) {
		fprintf(stderr, "Error in RB_createDisplay: cannot allocate display!\n");
		return NULL;
	}

	ret->window = NULL;
	ret->renderer = NULL;
	ret->texture = NULL;

	ret->framesPerSecond = 60.0;
	ret->secondsPerFrame = 1.0/ret->framesPerSecond;
	ret->lastFrameProcTime = clock();

	ret->rRes = rRes;
	ret->gRes = gRes;
	ret->bRes = bRes;

	// Initializing SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Error in RB_createDisplay: Error initializing SDL!: %s\n", SDL_GetError());
		RB_freeDisplay(ret);
		return NULL;
	}

	// Creating window
	ret->window = SDL_CreateWindow("Rainbow", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wWidth, wHeight, 0);
	if(ret->window == NULL) {
		fprintf(stderr, "Error in RB_createDisplay: Error creating window!: %s\n", SDL_GetError());
		RB_freeDisplay(ret);
		return NULL;
	}

	// Creating renderer
	ret->renderer = SDL_CreateRenderer(ret->window, -1, 0);
	if(ret->renderer == NULL) {
		fprintf(stderr, "Error in RB_createDisplay: Error creating renderer!: %s\n", SDL_GetError());
		RB_freeDisplay(ret);
		return NULL;
	}

	// Creating texture
	ret->texture = SDL_CreateTexture(ret->renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, pWidth, pHeight);
	if(ret->texture == NULL) {
		fprintf(stderr, "Error in RB_createDisplay: Error creating texture!: %s\n", SDL_GetError());
		RB_freeDisplay(ret);
		return NULL;
	}

	// Setting render target to the texture
	SDL_SetRenderTarget(ret->renderer, ret->texture);

	// Giving the texture a background.
	SDL_SetRenderDrawColor(ret->renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(ret->renderer);
	RB_forceUpdateDisplay(ret, true);

	// Handling the window events. This is required to make the window show up.
	RB_handleWindowEvents(ret);

	return ret;
}

void RB_freeDisplay(RB_Display* ret) {
	printf("Freeing RB_Display!\n");
	if(ret->texture != NULL) {
		SDL_DestroyTexture(ret->texture);
		ret->texture = NULL;
	}
	if(ret->renderer != NULL) {
		SDL_DestroyRenderer(ret->renderer);
		ret->renderer = NULL;
	}
	if(ret->window != NULL) {
		SDL_DestroyWindow(ret->window);
		ret->window = NULL;
	}
	free(ret);

	// Technically, this does make it not-modular.
	SDL_Quit();
}



// Forces the display to update immediately.
void RB_forceUpdateDisplay(RB_Display* display, bool interruptFramerate) {
	if(interruptFramerate) {
		display->lastFrameProcTime = clock();
	}

	SDL_SetRenderTarget(display->renderer, NULL);
	SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
	SDL_RenderPresent(display->renderer);
	SDL_SetRenderTarget(display->renderer, display->texture);
}

// If it has been a sufficiently long time since the last update, updates the display and returns 1.
// Otherwise, does not update the display and returns 0.
bool RB_updateDisplay(RB_Display* display) {
	clock_t currentTime = clock();
	double deltaT = ((double) (currentTime - display->lastFrameProcTime)) / CLOCKS_PER_SEC;
	if(deltaT < 0 || deltaT >= display->secondsPerFrame) {
		RB_forceUpdateDisplay(display, false);
		display->lastFrameProcTime = currentTime;
		return true;
	}

	return false;
}

// Handles window events. If the window is closing, returns 0. Otherwise, returns 1.
int RB_handleWindowEvents(RB_Display* disp) {
	SDL_Event e;
	while(SDL_PollEvent(&e) == 1) {
		if(e.type == SDL_QUIT) {
			return 0;
		}
		if(e.type == SDL_KEYDOWN) {
			if(e.key.keysym.sym == SDLK_a) {
				return 2;
			}
		}
	}

	return 1;
}

//rgb(213, 64, 166)

typedef struct {
	double r;
	double g;
	double b;
} FloatColor;

typedef struct {
	double h;
	double s;
	double v;
} HSVColor;

#define RB_FMAX(a, b) ((a > b)? a : b)
#define RB_FMIN(a, b) ((a < b)? a : b)

HSVColor rgbToHSV(FloatColor rgbCol) {
	double cMax = RB_FMAX(RB_FMAX(rgbCol.r, rgbCol.g), rgbCol.b);
	double cMin = RB_FMIN(RB_FMIN(rgbCol.r, rgbCol.g), rgbCol.b);
	double delta = cMax - cMin;

	HSVColor ret = {};

	if(delta == 0) {
		ret.h = 0;
	} else if(cMax == rgbCol.r) {
		ret.h = fmod((rgbCol.g - rgbCol.b) / delta, 6) / 6.0; 
	} else if(cMax == rgbCol.g) {
		ret.h = (((rgbCol.b - rgbCol.r) / delta) + 2) / 6.0;
	} else { // if cMax == rgbCol.b
		ret.h = (((rgbCol.r - rgbCol.g) / delta) + 4) / 6.0;
	}

	ret.s = (cMax == 0)? 0 : (delta / cMax);
	ret.v = cMax;

	return ret;
}

double colorDist(FloatColor col) {
	return sqrt(pow(col.r, 2) + pow(col.g, 2) + pow(col.b, 2));
}

FloatColor divideColor(FloatColor col, double div) {
	return (FloatColor) { .r = col.r / div, .g = col.g / div, .b = col.b / div };
}

double calculateColorSimilarity(FloatColor realColor, FloatColor targetColor) {
	// static FloatColor targetColor = {
	// 	.r = 213.0 / 255.0,
	// 	.g = 64.0 / 255.0,
	// 	.b = 166.0 / 255.0
	// };
	// static FloatColor targetColor = {
	// 	.r = 255.0 / 255.0,
	// 	.g = 238.0 / 255.0,
	// 	.b = 0.0 / 255.0
	// };
	// static FloatColor targetColor = {
	// 	.r = 255.0 / 255.0,
	// 	.g = 153.0 / 255.0,
	// 	.b = 0.0 / 255.0
	// };
	// static FloatColor targetColor = {
	// 	.r = 0.0 / 255.0,
	// 	.g = 76.0 / 255.0,
	// 	.b = 255.0 / 255.0
	// };
	// static FloatColor targetColor = {
	// 	.r = 105.0 / 255.0,
	// 	.g = 255.0 / 255.0,
	// 	.b = 165.0 / 255.0
	// };
	// static FloatColor targetColor = {
	// 	.r = 13.0 / 255.0,
	// 	.g = 0.0 / 255.0,
	// 	.b = 255.0 / 255.0
	// };
	// static FloatColor targetColor = {
	// 	.r = 214.0 / 255.0,
	// 	.g = 20.0 / 255.0,
	// 	.b = 133.0 / 255.0
	// };
	// static double targetColorDist;
	// static FloatColor normTargetColor;
	
	// static FloatColor furthestColor;
	// static double furthestColorDist;
	// static FloatColor furthestColorNorm;

	// static double bestColorSimilarity;
	// static double worstColorSimilarity;

	// static int valuesSet = 0;

	// if(!valuesSet) {
	// 	targetColorDist = colorDist(targetColor);
	// 	normTargetColor = divideColor(targetColor, targetColorDist);

	// 	furthestColor = (FloatColor) {
	// 		.r = targetColor.r < 0.5? 1 : 0,
	// 		.g = targetColor.g < 0.5? 1 : 0,
	// 		.b = targetColor.b < 0.5? 1 : 0,
	// 	};
	// 	furthestColorDist = colorDist(furthestColor);
	// 	furthestColorNorm = divideColor(furthestColor, furthestColorDist);

	// 	bestColorSimilarity = pow(normTargetColor.r, 2) + pow(normTargetColor.g, 2) + pow(normTargetColor.b, 2);
	// 	worstColorSimilarity = (
	// 		(furthestColorNorm.r * normTargetColor.r)
	// 		+ (furthestColorNorm.g * normTargetColor.g)
	// 		+ (furthestColorNorm.b * normTargetColor.b)
	// 	);

	// 	valuesSet = 1;
	// }


	// double realColorDist = colorDist(realColor);
	// FloatColor normRealColor = divideColor(realColor, realColorDist);

	// double colorSimilarity = ((
	// 	(normRealColor.r * normTargetColor.r)
	// 	+ (normRealColor.g * normTargetColor.g)
	// 	+ (normRealColor.b * normTargetColor.b)
	// ) - worstColorSimilarity) / (bestColorSimilarity - worstColorSimilarity);


	// return colorSimilarity;

	const HSVColor targetHSV = rgbToHSV(targetColor);
	const HSVColor realHSV = rgbToHSV(realColor);

	const double targetRadius = targetHSV.s * targetHSV.v;
	const double realRadius = realHSV.s * realHSV.v;

	const double targetAngle = targetHSV.h * 2 * M_PI;
	const double realAngle = realHSV.h * 2 * M_PI;
	
	const double dX = (cos(targetAngle) * targetRadius) - (cos(realAngle) * realRadius);
	const double dY = (sin(targetAngle) * targetRadius) - (sin(realAngle) * realRadius);
	const double dZ = targetHSV.v - realHSV.v;

	return 1.0 - (sqrt((dX * dX) + (dY * dY) + (dZ * dZ)) / 2);

	// double difference = fmod((realHSV.h - targetHSV.h) + 1.0, 1.0);
	// if(difference > 0.5) difference = 1.0 - difference;
	// return 1.0 - (difference / 0.5);
}

#define RB_RGB(rVal, gVal, bVal) { .r = rVal / 255.0, .g = gVal / 255.0, .b = bVal / 255.0 }

// Sets the pixel at the specified coordinate to the specified color
// Note: This function will convert the color to the displayed color format. You should NOT do that beforehand.
void RB_setDisplayedPixelColor(RB_Display* disp, RB_Coord coord, RB_Color color) {
	static double greatestSimilarity = 0;

	// Using this type because it is guaranteed to be twice as many bits long as a color channel is;
	RB_ColorChannel realR = ((RB_ColorSquareDistance) color.r * RB_MAXIMUM_COLOR_CHANNEL_RESOLUTION) / disp->rRes;
	RB_ColorChannel realG = ((RB_ColorSquareDistance) color.g * RB_MAXIMUM_COLOR_CHANNEL_RESOLUTION) / disp->gRes;
	RB_ColorChannel realB = ((RB_ColorSquareDistance) color.b * RB_MAXIMUM_COLOR_CHANNEL_RESOLUTION) / disp->bRes;
	
	FloatColor realColor = {
		.r = ((double) color.r) / (disp->rRes - 1),
		.g = ((double) color.g) / (disp->gRes - 1),
		.b = ((double) color.b) / (disp->bRes - 1)
	};

	// Pink Blue
	// static FloatColor targetColorA = RB_RGB(214, 20, 133);
	// static FloatColor targetColorB = RB_RGB(0, 76, 255);

	// Blue Yellow
	// static FloatColor targetColorA = RB_RGB(0, 200, 255);
	// static FloatColor targetColorB = RB_RGB(255, 255, 0);

	// Red Orange
	// static FloatColor targetColorA = RB_RGB(255, 0, 0);
	// static FloatColor targetColorB = RB_RGB(255, 128, 0);

	// static FloatColor targetColorA = RB_RGB(0, 0, 255);
	// static FloatColor targetColorB = RB_RGB(255, 0, 255);

	// static FloatColor targetColorA = RB_RGB(0, 0, 255);
	// static FloatColor targetColorB = RB_RGB(0, 255, 255);

	// static FloatColor targetColorA = RB_RGB(0, 0, 255);
	// static FloatColor targetColorB = RB_RGB(0, 0, 255);

	// Blood
	static FloatColor targetColorA = RB_RGB(113, 21, 24);
	static FloatColor targetColorB = RB_RGB(56, 10, 11);


	// // if(colorSimilarity < lowestSimilarity) {
	// // 	lowestSimilarity = colorSimilarity;
	// // 	printf("Found new lowest similarity: %lf\n", lowestSimilarity);
	// // }

	// double colorSimilarity = calculateColorSimilarity(realColor);
	// double adjustment = pow(colorSimilarity, 2.5);
	// double adjustment = colorSimilarity;
	
	double colorSimilarityA = calculateColorSimilarity(realColor, targetColorA);
	double colorSimilarityB = calculateColorSimilarity(realColor, targetColorB);
	double colorSimilarity = RB_FMAX(colorSimilarityA, colorSimilarityB);
	double cutoff = 0.2;
	double adjustment = RB_FMAX((pow(colorSimilarity, 3) / cutoff) - 1, 0.0) / ((1 / cutoff) - 1);
	//double adjustment = 1.0 - pow(1.0 - (RB_FMAX((colorSimilarity / cutoff) - 1, 0) / ((1.0/cutoff) - 1)), 2.5);

	// FloatColor adjustedRealColor = {
	// 	.r = realColor.r * adjustment,
	// 	.g = realColor.g * adjustment,
	// 	.b = realColor.b * adjustment
	// };

	FloatColor adjustedRealColor = {
		.r = 1.0 - ((1.0 - realColor.r) * (adjustment)),
		.g = 1.0 - ((1.0 - realColor.g) * (adjustment)),
		.b = 1.0 - ((1.0 - realColor.b) * (adjustment))
	};

	realR = (adjustedRealColor.r * 255);
	realG = (adjustedRealColor.g * 255);
	realB = (adjustedRealColor.b * 255);

	// if(colorSimilarity < 0) {
	// 	realR = 255;
	// 	realG = 0;
	// 	realB = 0;	
	// } else if(colorSimilarity > 1) {
	// 	realR = 0;
	// 	realG = 255;
	// 	realB = 0;	
	// } else {
	// 	realR = 0;
	// 	realG = 0;
	// 	realB = 255;	
	// }

	// HSVColor realHSV = rgbToHSV(realColor);
	// realR = (colorSimilarity * 255);
	// realG = (colorSimilarity * 255);
	// realB = (colorSimilarity * 255);

	// // RB_ColorChannelSum avg = (((RB_ColorChannelSum) realR) + realG + realB) / 3;
	// // double avgProp = avg / ((double) RB_MAXIMUM_COLOR_CHANNEL_RESOLUTION - 1.0);
	// // double scale = pow((avg / ((double) RB_MAXIMUM_COLOR_CHANNEL_RESOLUTION - 1.0)), 2.5);
	// // //double scale = 1.0 - pow(1.0 - avgProp, 0.35);


	// // realR = (RB_ColorChannel)(realR * scale);
	// // realG = (RB_ColorChannel)(realG * scale);
	// // realB = (RB_ColorChannel)(realB * scale);

	// // double colorfulness = avgProp;

	// // realR = (RB_ColorChannel) ((realR * colorfulness) + (avg * (1.0 - colorfulness)));
	// // realG = (RB_ColorChannel) ((realG * colorfulness) + (avg * (1.0 - colorfulness)));
	// // realB = (RB_ColorChannel) ((realB * colorfulness) + (avg * (1.0 - colorfulness)));

	// // RB_Color pink = { .r = 255, .g = 0, .b = 255 };
	// // RB_Color blue = { .r = 0, .g = 255, .b = 255 };
	// // RB_Color white = { .r = 255, .g = 255, .b = 255 };

	// // realR = ((RB_ColorSquareDistance) realR) + pink.r + 


	// // RB_ColorChannel realR = color.r;
	// // RB_ColorChannel realG = color.g;
	// // RB_ColorChannel realB = color.b;

	// // RB_ColorChannel realG = (realR > realB)? realR : realB;

	// // RB_ColorChannel realG = (realR < realB)? realR : realB;
	// // realB = 255;

	SDL_SetRenderDrawColor(disp->renderer, realR, realG, realB, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawPoint(disp->renderer, coord.x, coord.y);
}