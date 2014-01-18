/*
    magichexagon source - cutsceneEngine.h
    Copyright (c) 2014 Mark Hutcheson
*/
#ifndef CUTSCENEENGINE_H
#define CUTSCENEENGINE_H

#include "Engine.h"
#include "DebugDraw.h"
#include <vector>
#include <set>

#define DEFAULT_WIDTH	800
#define DEFAULT_HEIGHT	600

#define DEFAULT_TIMESCALE	2.5

//const variables
const Color FluttershyMane(248, 185, 207);
const Color Fluttershy(253, 246, 175);
const Color FluttershyEyes(0, 173, 168);
const Color Twilight(218,172,232);
const Color TwilightMane1(234,218,240);
const Color TwilightMane2(233,72,143);
const Color TwilightMane3(102,47,137);
const Color Rarity(238,241,243);
const Color RarityMane(101,77,160);
const Color RarityEyes(57,120,187);
const Color Dash(157,217,247);
const Color DashManeR(239,64,53);
const Color DashManeO(243,119,54);
const Color DashManeY(255,247,151);
const Color DashManeG(122,193,66);
const Color DashManeB(0, 147, 208);
const Color DashManeV(109, 31, 126);
const Color Pinkie(248,185,207);
const Color PinkieMane(239,79,147);
const Color PinkieEyes(125,206,240);
const Color AJ(252,188,94);
const Color AJMane(255,246,152);
const Color AJEyes(95,187,83);

#define COLOR_EPSILON	0.001

class Wall
{
public:
	float32 speed;
	float32 height;
	float32 length;
};

#define WALL_START_HEIGHT 	15.0
#define WALL_SPEED 			3.5

class ColorPhase
{
public:
	Color* colorToChange;
	float32 destr, destg, destb;
	float32 amtr, amtg, amtb;
};

typedef struct
{
	int hex;
	float32 height, length;
} pattern;

class magichexagonEngine : public Engine
{
private:
  //Important general-purpose game variables
  ttvfs::VFSHelper vfs;
  Vec3 CameraPos;
  DebugDraw m_debugDraw;
  //HUD* m_hud;
  bool m_bMouseGrabOnWindowRegain;
  
  //Game stuff!
  Color m_colors[8];
  float m_fRotateAngle;
  float m_fRotateAdd;
  float m_fPlayerAngle;
  list<Wall> m_walls[6];
  Image* centerCutie;
  list<ColorPhase> m_ColorsChanging;
  vector<vector<list<pattern> > > m_Patterns;
  
  //More generic game stuff
  Rect m_rcBounds;			//Camera bounds
  Rect m_rcDragArea;		//Area that the user can drag the apples around in
  float32 m_fDefCameraZ;	//Default position of camera on z axis

protected:
    void frame(float32 dt);
    void draw();
    void init(list<commandlineArg> sArgs);
    void handleEvent(SDL_Event event);

public:
	//magichexagon.cpp functions - fairly generic 
    magichexagonEngine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sIcon, bool bResizable = false);
    ~magichexagonEngine();
	
	bool _shouldSelect(b2Fixture* fix);

    void hudSignalHandler(string sSignal);  //For handling signals that come from the HUD
	void handleKeys();						//Poll the keyboard state and update the game accordingly
	Point worldPosFromCursor(Point cursorpos);	//Get the worldspace position of the given mouse cursor position
	Point worldMovement(Point cursormove);		//Get the worldspace transform of the given mouse transformation
	
	//Functions dealing with program defaults
	void loadConfig(string sFilename);
	void saveConfig(string sFilename);
	
	obj* objFromXML(string sXMLFilename, Point ptOffset, Point ptVel = Point(0,0));
	Rect getCameraView();		//Return the rectangle, in world position z=0, that the camera can see 
		
	//level.cpp functions
	bool loadPatterns(string sFilename);
	void renderLevel();
	void addWall(float32 height, float32 speed, float32 length, int32_t hex);	//Add a wall to the current level
	void updateLevel(float32 dt);
	void updateWalls(float32 dt);			//Make walls fall inward and check player collision
	void nextPattern();						//Lay down next pattern of walls
	int  calcPlayerHex(float32* relAngle = NULL);
	void checkSides(float32 fOldAngle, int prevHex, int curHex);
	
	//color.cpp functions
	void updateColors(float32 dt);
	void phaseColor(Color* src, Color dest, float time);
	void colorFlip();
};

void signalHandler(string sSignal); //Stub function for handling signals that come in from our HUD, and passing them on to myEngine



#endif
