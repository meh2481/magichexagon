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

class gravWell
{
public:
	float32 wellMass;
	b2Fixture* sensor;
	set<b2Body*> trappedObjects;
};

class physicsContact : public b2ContactListener
{
public:
	void BeginContact(b2Contact *contact);
	void EndContact(b2Contact *contact);
	void PreSolve(b2Contact *contact, const b2Manifold *oldManifold) {};
	void PostSolve(b2Contact *contact, const b2ContactImpulse *impulse) {};
};

class magichexagonEngine : public Engine
{
private:
  //Important general-purpose game variables
  ttvfs::VFSHelper vfs;
  Vec3 CameraPos;
  DebugDraw m_debugDraw;
  //HUD* m_hud;
  physicsContact* m_contactListener;
  bool m_bMouseGrabOnWindowRegain;
  
  //Game stuff!
  bool m_bDrawDebug;
  Color m_colors[8];
  float m_fRotateAngle;
  float m_fRotateAdd;
  float m_fPlayerAngle;
  
  Rect m_rcBounds;			//Camera bounds
  Rect m_rcDragArea;		//Area that the user can drag the apples around in
  float32 m_fDefCameraZ;	//Default position of camera on z axis

protected:
    void frame(float32 dt);
    void draw();
    void init(list<commandlineArg> sArgs);
    void handleEvent(SDL_Event event);

public:
    magichexagonEngine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sIcon, bool bResizable = false);
    ~magichexagonEngine();
	
	bool _shouldSelect(b2Fixture* fix);

    void hudSignalHandler(string sSignal);  //For handling signals that come from the HUD
	void handleKeys();						//Poll the keyboard state and update the game accordingly
	Point worldPosFromCursor(Point cursorpos);	//Get the worldspace position of the given mouse cursor position
	Point worldMovement(Point cursormove);		//Get the worldspace transform of the given mouse transformation
	void cameraBounds();					//Keep camera inside level bounds
	
	//Functions dealing with program defaults
	void loadConfig(string sFilename);
	void saveConfig(string sFilename);
	
	obj* objFromXML(string sXMLFilename, Point ptOffset, Point ptVel = Point(0,0));
	Rect getCameraView();		//Return the rectangle, in world position z=0, that the camera can see 
};

void signalHandler(string sSignal); //Stub function for handling signals that come in from our HUD, and passing them on to myEngine



#endif
