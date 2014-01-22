/*
    magichexagon source - cutsceneEngine.h
    Copyright (c) 2014 Mark Hutcheson
*/
#ifndef CUTSCENEENGINE_H
#define CUTSCENEENGINE_H

#include "Engine.h"
#include <vector>
#include <set>

#define DEBUG	//Debug mode; cheat keys and such

#define DEFAULT_WIDTH	800
#define DEFAULT_HEIGHT	600

#define DEFAULT_TIMESCALE	1.0

//const variables
const Color FluttershyMane(248, 185, 207);
const Color Fluttershy(253, 246, 175);
const Color FluttershyEyes(0, 173, 168);
const Color Twilight(218,172,232);
const Color TwilightMane(102,47,137);
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

#define MENU_START			0
#define MENU_LEVELSELECT	1
#define MENU_NONE			2
#define MENU_GAMEOVER		3

#define LEVELTIME			60.0

#define LEVEL_FRIENDSHIP	0
#define LEVEL_HONESTY		1
#define LEVEL_KINDNESS		2
#define LEVEL_LOYALTY		3
#define LEVEL_GENEROSITY	4
#define LEVEL_LAUGHTER		5
#define LEVEL_MAGIC			6

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

class Wall
{
public:
	float32 speed;
	float32 height;
	float32 length;
};

class magichexagonEngine : public Engine
{
private:
  //Important general-purpose game variables
  ttvfs::VFSHelper vfs;
  Vec3 CameraPos;
  HUD* m_hud;
  bool m_bMouseGrabOnWindowRegain;
  float32 m_fDefCameraZ;	//Default position of camera on z axis
  
  //Game stuff!
  Color m_colors[8];
  float m_fRotateAngle;
  float m_fRotateAdd;
  float m_fPlayerAngle;
  list<Wall> m_walls[6];
  Image* centerCutie;
  list<ColorPhase> m_ColorsChanging;
  vector<vector<list<pattern> > > m_Patterns;
  int m_iCurMenu;
  float m_fTotalSpinTime;
  float m_fTargetSpinReverse;
  float m_fTargetSpinIncrease;
  float m_fTargetSpinTime;
  int m_iTargetSpinLevel;
  int m_iCurLevel;
  int m_iStartLevel;
  float m_fWallSpeed;
  float m_fPlayerMove;
  float m_fWallStartHeight;
  float m_fTargetMadSpin;
  float m_fMadSpinLength;
  Wall* m_wTop;
  float m_gap;
  float m_fLastChecked;	//Last time we checked the level (so we don't change levels constantly)
  float m_fBestTime[6];	//Best time in each level
	

protected:
    void frame(float32 dt);
    void draw();
    void init(list<commandlineArg> sArgs);
    void handleEvent(SDL_Event event);

public:
	//magichexagon.cpp functions - fairly generic 
    magichexagonEngine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sAppName, string sIcon, bool bResizable = false);
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
		
	void drawStartMenu();
	void drawLevelSelectMenu();
		
	//level.cpp functions
	bool loadPatterns(string sFilename);
	void renderLevel();
	void addWall(float32 height, float32 speed, float32 length, int32_t hex);	//Add a wall to the current level
	void updateLevel(float32 dt);
	void updateWalls(float32 dt);			//Make walls fall inward and check player collision
	void checkLevel();						//See if we should change levels, speed up, or what
	void nextPattern();						//Lay down next pattern of walls
	int  calcPlayerHex(float32* relAngle = NULL);
	void checkSides(float32 fOldAngle, int prevHex, int curHex);
	void changeLevel(int iNewLevel);	//Change to given level
	void die();							//Kill player
	Wall* top();						//Get top (closest to player) wall
	
	//color.cpp functions
	void updateColors(float32 dt);
	void phaseColor(Color* src, Color dest, float time);
	void colorFlip();
	void resetLevel();
};

void signalHandler(string sSignal); //Stub function for handling signals that come in from our HUD, and passing them on to myEngine
float myAbs(float v);	//Because stinking namespace stuff


#endif
