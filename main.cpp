/*
    magichexagon source - main.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "magichexagon.h"

#ifdef _WIN32
#define ICONNAME "res/icons/icon_32.png"	//For some reason, Windoze (Or SDL2, or something) doesn't like large (256x256) icons for windows. Using a 32x32 icon instead.
#include <shellapi.h>	//For CommandLineToArgv() and friends
string ws2s(const wstring& s)
{
    int len;
    int slength = (int)s.length();
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0); 
    string r(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0); 
    return r;
}
#else
#define ICONNAME "res/icons/icon_256.png"
#endif

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char *argv[])
#endif
{
    FreeImage_Initialise();
	
    magichexagonEngine* eng = new magichexagonEngine(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Magic Hexagon", "magichexagon", ICONNAME, true); //Create our engine
	list<string> lCommandLine;
#ifndef _WIN32
	//Standard C++ way
	for(int i = 1; i < argc; i++)
		lCommandLine.push_back(argv[i]);
#else
	//But of course Windoze has to be different
	LPWSTR *szArglist;
	int argc;

	szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
	if(szArglist != NULL)
	{
		for(int i = 1; i < argc; i++)
			lCommandLine.push_back(ws2s(szArglist[i]));

		//Free memory allocated for CommandLineToArgvW arguments.
		LocalFree(szArglist);
	}
#endif
	eng->commandline(lCommandLine);
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
