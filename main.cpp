/*
    magichexagon source - main.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "magichexagon.h"

#ifdef _WIN32
#define ICONNAME "res/icons/icon_32.png"	//For some reason, Windoze (Or SDL2, or something) doesn't like large (256x256) icons for windows. Using a 32x32 icon instead.
#else
#define ICONNAME "res/icons/icon_256.png"
#endif

#ifdef _WIN32
int WINAPI WinMain(
HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR lpCmdLine,
int nCmdShow)
#else
int main(int argc, char *argv[])
#endif
{
    FreeImage_Initialise();
	
    magichexagonEngine* eng = new magichexagonEngine(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Magic Hexagon", "magichexagon", ICONNAME, true); //Create our engine
#ifndef _WIN32
	eng->commandline(argc, argv);
#endif
    eng->start(); //Get the engine rolling
	
	//Done main loop; exit program	
	errlog << "Deleting engine" << endl;
    delete eng;
    errlog << "Closing FreeImage" << endl;
    FreeImage_DeInitialise();
    errlog << "Ending program happily" << endl;
	errlog.close();
    return 0;
}
