
RBHEADERS = $(addprefix src/headers/,RB_AssignmentQueue.h RB_BasicTypes.h RB_ColorPool.h RB_Main.h RB_Pixel.h RB_PixelMap.h RB_Display.h) 
IMPLEMENTATIONS = $(addprefix src/defaults/,basicAssignmentQueue.c basicColorPool.c basicPixelMap.c display.c rainbowMain.c basicTypes.c)

main: $(RBHEADERS) $(IMPLEMENTATIONS) src/main.c
	gcc -o main src/main.c $(IMPLEMENTATIONS) -I./src `sdl2-config --cflags --libs`

test: $(addprefix src/headers/,RB_ColorPool.h RB_BasicTypes.h) $(addprefix src/defaults/,basicColorPool.c basicTypes.c)
	gcc -o test $(addprefix src/defaults/,basicColorPool.c basicTypes.c) -I./src

# RBHEADERS = $(addprefix src/headers/,RB_AssignmentQueue.h RB_BasicTypes.h RB_ColorPool.h RB_Main.h RB_Pixel.h RB_PixelMap.h RB_Display.h) 
# IMPLEMENTATIONS = $(addprefix src/defaults/,basicAssignmentQueue.c basicColorPool.c basicPixelMap.c display.c rainbowMain.c basicTypes.c)

# main: $(RBHEADERS) $(IMPLEMENTATIONS) src/main.cpp
# 	gcc -o main src/main.cpp $(IMPLEMENTATIONS) -I./src `sdl2-config --cflags --libs`

# main: rainbowFactory.c display.c rainbowImageGen.h display.h
# #	gcc -o main display.c `sdl2-config --cflags --libs`
# 	gcc -o main rainbowFactory.c display.c `sdl2-config --cflags --libs`