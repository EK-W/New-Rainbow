#include "headers/RB_Main.h"
#include "headers/RB_Display.h"
#include "headers/RB_ColorPool.h"
#include "headers/RB_PixelMap.h"
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

void drawStarPixel(RB_Coord coord, RB_Data* data, int distance) {
	RB_Color colorWhite = (RB_Color) {
		.r = data->config.rRes - 1 - distance,
		.g = data->config.gRes - 1 - distance,
		.b = data->config.bRes - 1 - distance
	};
	//const RB_Color colorWhite = RB_getRandomColor(data);

	RB_Pixel* pixel = RB_getPixel(data->pixelMap, coord);

	if(pixel == NULL || pixel->status == RB_PIXEL_SET) {
		return;
	}

	RB_Color colorToUse = RB_findIdealAvailableColor(data->colorPool, colorWhite);
	RB_setCoordColor(data, coord, colorToUse);
}

int main(int argc, char** argv) {
	RB_Config* config = RB_newConfig();
	// square resolutions are:
	// 1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 169, 196, 225, 256
	uint16_t rRes = 100;
	uint16_t gRes = 100;
	uint16_t bRes = 100;

	RB_setColorResolution(config, rRes, gRes, bRes);
	RB_setWindowDimensions(config, 720, 720);



	RB_Data* rainbow = RB_init(config);


	
	// for(int i = 0; i < 64; i++) {
	// 	RB_Coord starCenter = RB_getRandomCoord(rainbow);
	// 	int starRadius = ((rand() % 13) + 2) / 2;
		
	// 	for(int j = 0; j <= starRadius; j++) {
	// 		drawStarPixel((RB_Coord) { .x = starCenter.x - j, .y = starCenter.y }, rainbow, j);
	// 		drawStarPixel((RB_Coord) { .x = starCenter.x + j, .y = starCenter.y }, rainbow, j);
	// 		drawStarPixel((RB_Coord) { .x = starCenter.x, .y = starCenter.y - j }, rainbow, j);
	// 		drawStarPixel((RB_Coord) { .x = starCenter.x, .y = starCenter.y + j }, rainbow, j);
	// 	}
	// }

	/*const int widthStep = rainbow->config.width / 8;
	const int heightStep = rainbow->config.height / 8;
	for(int i = 0; i < 8; i++) {
		for(int j = 0; j < 8; j++) {
			RB_Coord starCenter = { .x = i * widthStep + (widthStep / 2), .y = j * heightStep + (heightStep / 2) };
			int starRadius = ((rand() % 13) + 2) / 2;

			if(rand() % 2 == 0) continue;

			int shouldBeBlack = rand() % 2;

			for(int k = 0; k <= starRadius; k++) {
				drawStarPixel((RB_Coord) { .x = starCenter.x - k, .y = starCenter.y }, rainbow, k, shouldBeBlack);
				drawStarPixel((RB_Coord) { .x = starCenter.x + k, .y = starCenter.y }, rainbow, k, shouldBeBlack);
				drawStarPixel((RB_Coord) { .x = starCenter.x, .y = starCenter.y - k }, rainbow, k, shouldBeBlack);
				drawStarPixel((RB_Coord) { .x = starCenter.x, .y = starCenter.y + k }, rainbow, k, shouldBeBlack);
			}
		}
	}*/
	// RB_setCoordColor(rainbow, RB_getRandomCoord(rainbow), RB_getRandomColor(rainbow));
	//RB_setCoordColor(rainbow, RB_getRandomCoord(rainbow), RB_getRandomColor(rainbow));

	// RB_Color startColor = {
	// 	.r = (0 * rRes) / 255,
	// 	.g = (0 * gRes) / 255,
	// 	.b = (0 * bRes) / 255
	// };
	// RB_setCoordColor(rainbow, (RB_Coord) { .x = (rainbow->config.width * 5) / 10, .y = rainbow->config.height / 2 }, startColor);

	// RB_setCoordColor(rainbow, RB_getRandomCoord(rainbow), (RB_Color) { .r = 128, .g = 0, .b = 128 });
	//RB_setCoordColor(rainbow, (RB_Coord) { .x = rainbow->config.width / 2, .y = rainbow->config.height / 2 }, (RB_Color) { .r = 128, .g = 0, .b = 128 });
	//RB_setCoordColor(rainbow, (RB_Coord) { .x = rainbow->config.width / 2, .y = rainbow->config.height / 2 }, (RB_Color) { .r = 255, .g = 0, .b = 128 });
	RB_setCoordColor(rainbow, (RB_Coord) { .x = rainbow->config.width / 2, .y = 0 }, (RB_Color) { .r = 255, .g = 38, .b = 159 });
	bool shouldQuit = false;

	bool shouldContinue = true;

	RB_updateDisplay(rainbow->display);

	while(shouldContinue) {
		switch(RB_handleWindowEvents(rainbow->display)) {
			case 0:
				shouldQuit = true;
				shouldContinue = false;
				break;
			case 1:
			case 2:
				if(RB_generateNextPixel(rainbow) == 0) {
					shouldContinue = false;
				}
				RB_updateDisplay(rainbow->display);
				break;
		}
	}

	printf("Done generating!\n");

	if(!shouldQuit) {
		RB_forceUpdateDisplay(rainbow->display, false);
		while(RB_handleWindowEvents(rainbow->display) != 0);
	}
	

	RB_free(rainbow);
	RB_freeConfig(config);

	return 0;
}