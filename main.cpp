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

int main(int argc, char *argv[])
{
    errlog << "Starting program" << endl;
    errlog << "Starting FreeImage" << endl;
    FreeImage_Initialise();
	
    magichexagonEngine* eng = new magichexagonEngine(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Magic Hexagon", ICONNAME, true); //Create our engine
	eng->commandline(argc, argv);
    eng->start(); //Get the engine rolling
	
	//Done main loop; exit program	
	errlog << "Deleting engine" << endl;
    delete eng;
    errlog << "Closing FreeImage" << endl;
    FreeImage_DeInitialise();
    errlog << "Ending program happily" << endl;
    return 0;
}
