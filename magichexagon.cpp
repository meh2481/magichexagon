/*
    magichexagon source - magichexagon.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "magichexagon.h"
#include "tinyxml2.h"
#include <float.h>
#include <sstream>

//For our engine functions to be able to call our Engine class functions
static magichexagonEngine* g_pGlobalEngine;

void signalHandler(string sSignal)
{
    g_pGlobalEngine->hudSignalHandler(sSignal);
}

magichexagonEngine::magichexagonEngine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sIcon, bool bResizable) : 
Engine(iWidth, iHeight, sTitle, sIcon, bResizable)
{
	g_pGlobalEngine = this;
	vfs.Prepare();
	
	//Set camera position for this game
	m_fDefCameraZ = -15;
	CameraPos.x = 0;
	CameraPos.y = 0;
	CameraPos.z = m_fDefCameraZ;
	m_bMouseGrabOnWindowRegain = false;//TODO: true;
	m_iCurMenu = MENU_START;
	
	//Game vars
	resetLevel();
	
	showCursor();
	
	//m_hud = new HUD("hud");
	//m_hud->create("res/hud.xml");
	
	m_debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_centerOfMassBit);
	b2World* world = getWorld();
	world->SetDebugDraw(&m_debugDraw);
	
	setTimeScale(DEFAULT_TIMESCALE);	//Speed up time
}

magichexagonEngine::~magichexagonEngine()
{
	errlog << "~magichexagonEngine()" << endl;
	saveConfig("res/config.xml");
	//Delete stuffs
	
	//errlog << "delete hud" << endl;
	//delete m_hud;
}

void magichexagonEngine::frame(float32 dt)
{
	switch(m_iCurMenu)
	{
		case MENU_START:
			break;
			
		case MENU_LEVELSELECT:
			break;
			
		case MENU_NONE:
			updateColors(dt);
			updateLevel(dt);
			handleKeys();
			break;
	}
}

void magichexagonEngine::draw()
{
	//Set up camera and OpenGL flags
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glLoadIdentity();
	
	glTranslatef(0, 0, CameraPos.z);
	
	switch(m_iCurMenu)
	{
		case MENU_START:
			//TODO
			break;
			
		case MENU_LEVELSELECT:
			//TODO
			break;
			
		case MENU_NONE:
			//Draw level
			renderLevel();
			break;
	}
		
	//Draw HUD
	//glLoadIdentity();
	//glTranslatef(0.0f, 0.0f, MAGIC_ZOOM_NUMBER);
	//m_hud->draw(0);
}

void magichexagonEngine::init(list<commandlineArg> sArgs)
{
	//Run through list for arguments we recognize
	for(list<commandlineArg>::iterator i = sArgs.begin(); i != sArgs.end(); i++)
	{
		errlog << "Commandline argument. Switch: " << i->sSwitch << ", value: " << i->sValue << endl;
	}
	
	//Load our last screen position and such
	loadConfig("res/config.xml");
	loadPatterns("res/patterns.xml");
	
	//Set gravity to 0
	getWorld()->SetGravity(b2Vec2(0,0));
	
	//Create sounds up front
	//vox
	createSound("res/sfx/begin.ogg", "begin");
	createSound("res/sfx/awesome.ogg", "awesome");
	createSound("res/sfx/excellent.ogg", "excellent");
	createSound("res/sfx/gameover.ogg", "gameover");
	createSound("res/sfx/magichexagon.ogg", "magichexagon");
	createSound("res/sfx/nice.ogg", "nice");
	createSound("res/sfx/wonderful.ogg", "wonderful");
	createSound("res/sfx/generosity.ogg", "generosity");
	createSound("res/sfx/honesty.ogg", "honesty");
	createSound("res/sfx/kindness.ogg", "kindness");
	createSound("res/sfx/laughter.ogg", "laughter");
	createSound("res/sfx/loyalty.ogg", "loyalty");
	createSound("res/sfx/magic.ogg", "magic");
	//sfx
	createSound("res/sfx/beginlevel.ogg", "beginlevel");	//When you enter a level
	createSound("res/sfx/menubegin.ogg", "menubegin");		//Initial keypress: Title->menu
	createSound("res/sfx/die.ogg", "die");					//When you DIIIIIE
	createSound("res/sfx/select.ogg", "select");			//When you're selecting different menu items
	
	
	//Play music
	//playMusic("res/sfx/encore-micro_hexagon_courtesy.ogg");
	//pauseMusic();
	playSound("magichexagon");
	
	//TODO seekMusic(34.504028);
	
	hideCursor();
}


void magichexagonEngine::hudSignalHandler(string sSignal)
{
}

void magichexagonEngine::handleEvent(SDL_Event event)
{
    //m_hud->event(event);    //Let our HUD handle any events it needs to
    switch(event.type)
    {
        //Key pressed
        case SDL_KEYDOWN:
            switch(event.key.keysym.scancode)
            {
                case SDL_SCANCODE_ESCAPE:
					switch(m_iCurMenu)
					{
						case MENU_NONE:
							pauseMusic();
							playSound("menubegin");
							m_iCurMenu = MENU_LEVELSELECT;
							break;
							
						case MENU_LEVELSELECT:
							playSound("menubegin");
							m_iCurMenu = MENU_START;
							break;
							
						case MENU_START:
							quit();
							break;
					}
                    break;
				
				case SDL_SCANCODE_LEFT:
					if(m_iCurMenu == MENU_LEVELSELECT)
					{
						playSound("select");
						//TODO
					}
					break;
				
				case SDL_SCANCODE_RIGHT:
					if(m_iCurMenu == MENU_LEVELSELECT)
					{
						playSound("select");
						//TODO
					}
					break;
                
                case SDL_SCANCODE_RETURN:
                    if(keyDown(SDL_SCANCODE_ALT))
                      toggleFullscreen();
                    break;
					
				case SDL_SCANCODE_F:
					playSound("kindness");
					phaseColor(&m_colors[0], Fluttershy, 0.5);
					phaseColor(&m_colors[1], FluttershyEyes, 0.5);
					phaseColor(&m_colors[2], FluttershyMane, 0.5);
					phaseColor(&m_colors[3], Fluttershy, 0.5);
					phaseColor(&m_colors[4], FluttershyMane, 0.5);
					phaseColor(&m_colors[5], Fluttershy, 0.5);
					phaseColor(&m_colors[6], FluttershyMane, 0.5);
					phaseColor(&m_colors[7], Fluttershy, 0.5);
					centerCutie = getImage("res/gfx/fluttermark.png");
					break;
					
				case SDL_SCANCODE_R:
					playSound("generosity");
					phaseColor(&m_colors[0], Rarity, 0.5);
					phaseColor(&m_colors[1], RarityEyes, 0.5);
					phaseColor(&m_colors[2], RarityMane, 0.5);
					phaseColor(&m_colors[3], Rarity, 0.5);
					phaseColor(&m_colors[4], RarityMane, 0.5);
					phaseColor(&m_colors[5], Rarity, 0.5);
					phaseColor(&m_colors[6], RarityMane, 0.5);
					phaseColor(&m_colors[7], Rarity, 0.5);
					centerCutie = getImage("res/gfx/rarimark.png");
					break;
					
				case SDL_SCANCODE_P:
					playSound("laughter");
					phaseColor(&m_colors[0], Pinkie, 0.5);
					phaseColor(&m_colors[1], PinkieEyes, 0.5);
					phaseColor(&m_colors[2], PinkieMane, 0.5);
					phaseColor(&m_colors[3], Pinkie, 0.5);
					phaseColor(&m_colors[4], PinkieMane, 0.5);
					phaseColor(&m_colors[5], Pinkie, 0.5);
					phaseColor(&m_colors[6], PinkieMane, 0.5);
					phaseColor(&m_colors[7], Pinkie, 0.5);
					centerCutie = getImage("res/gfx/pinkiemark.png");
					break;
					
				case SDL_SCANCODE_A:
					playSound("honesty");
					phaseColor(&m_colors[0], AJ, 0.5);
					phaseColor(&m_colors[1], AJEyes, 0.5);
					phaseColor(&m_colors[2], AJMane, 0.5);
					phaseColor(&m_colors[3], AJ, 0.5);
					phaseColor(&m_colors[4], AJMane, 0.5);
					phaseColor(&m_colors[5], AJ, 0.5);
					phaseColor(&m_colors[6], AJMane, 0.5);
					phaseColor(&m_colors[7], AJ, 0.5);
					centerCutie = getImage("res/gfx/ajmark.png");
					break;
					
				case SDL_SCANCODE_T:
					playSound("magic");
					phaseColor(&m_colors[0], Twilight, 0.5);
					phaseColor(&m_colors[1], TwilightMane1, 0.5);
					phaseColor(&m_colors[2], TwilightMane2, 0.5);
					phaseColor(&m_colors[3], TwilightMane3, 0.5);
					phaseColor(&m_colors[4], TwilightMane2, 0.5);
					phaseColor(&m_colors[5], TwilightMane3, 0.5);
					phaseColor(&m_colors[6], TwilightMane2, 0.5);
					phaseColor(&m_colors[7], TwilightMane3, 0.5);
					centerCutie = getImage("res/gfx/twilimark.png");
					break;
					
				case SDL_SCANCODE_D:
					playSound("loyalty");
					phaseColor(&m_colors[0], Dash, 0.5);
					phaseColor(&m_colors[1], Color(0,0,0), 0.5);
					phaseColor(&m_colors[2], DashManeR, 0.5);
					phaseColor(&m_colors[3], DashManeO, 0.5);
					phaseColor(&m_colors[4], DashManeY, 0.5);
					phaseColor(&m_colors[5], DashManeG, 0.5);
					phaseColor(&m_colors[6], DashManeB, 0.5);
					phaseColor(&m_colors[7], DashManeV, 0.5);
					centerCutie = getImage("res/gfx/dashmark.png");
					break;
				
				case SDL_SCANCODE_F10:
				case SDL_SCANCODE_G:
					if(keyDown(SDL_SCANCODE_CTRL))
						grabMouse(!isMouseGrabbed());	//Toggle grabbing/releasing the mouse
					break;
					
				case SDL_SCANCODE_F5:
					m_Patterns.clear();
					loadPatterns("res/patterns.xml");	//Reload patterns
					break;
					
				case SDL_SCANCODE_SPACE:
					switch(m_iCurMenu)
					{
						case MENU_START:
							playSound("menubegin");
							m_iCurMenu = MENU_LEVELSELECT;
							break;
							
						case MENU_LEVELSELECT:
							playMusic("res/sfx/encore-micro_hexagon_courtesy.ogg");
							playSound("begin");
							playSound("beginlevel");
							restartMusic();
							resetLevel();
							m_iCurMenu = MENU_NONE;
							break;
					}
					break;
            }
            break;

        //Key released
        case SDL_KEYUP:
            switch(event.key.keysym.scancode)
            {
            }
            break;
		
		case SDL_MOUSEBUTTONDOWN:
            if(event.button.button == SDL_BUTTON_LEFT)
            {
				
            }
            else if(event.button.button == SDL_BUTTON_RIGHT)
            {
				
            }
			else if(event.button.button == SDL_BUTTON_MIDDLE)
			{
				
			}
            break;
			
		case SDL_MOUSEWHEEL:
			/*if(event.wheel.y > 0)
			{
				CameraPos.z = min(CameraPos.z + 1.5, -5.0);
			}
			else
			{
				CameraPos.z = max(CameraPos.z - 1.5, -3000.0);
			}
			cameraBounds();*/
			break;

        case SDL_MOUSEBUTTONUP:
            if(event.button.button == SDL_BUTTON_LEFT)
            {
				
            }
			else if(event.button.button == SDL_BUTTON_RIGHT)
			{
			
			}
			else if(event.button.button == SDL_BUTTON_MIDDLE)
			{
				
			}
            break;

        case SDL_MOUSEMOTION:
            break;
		
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
				case SDL_WINDOWEVENT_FOCUS_LOST:
					m_bMouseGrabOnWindowRegain = isMouseGrabbed();
					grabMouse(false);	//Ungrab mouse cursor if alt-tabbing out or such
					break;
				
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					grabMouse(m_bMouseGrabOnWindowRegain);	//Grab mouse on input regain, if we should
					break;
			}
			break;
	}
}

Rect magichexagonEngine::getCameraView()
{
	Rect rcCamera;
	const float32 tan45_2 = tan(DEG2RAD*45/2);
	const float32 fAspect = (float32)getWidth() / (float32)getHeight();
	rcCamera.bottom = (tan45_2 * CameraPos.z);
	rcCamera.top = -(tan45_2 * CameraPos.z);
	rcCamera.left = rcCamera.bottom * fAspect;
	rcCamera.right = rcCamera.top * fAspect;
	rcCamera.offset(CameraPos.x, CameraPos.y);
	return rcCamera;
}

Point magichexagonEngine::worldMovement(Point cursormove)
{
	cursormove.x /= (float32)getWidth();
	cursormove.y /= (float32)getHeight();
	
	Rect rcCamera = getCameraView();
	cursormove.x *= rcCamera.width();
	cursormove.y *= -rcCamera.height();	//Flip y
	
	return cursormove;
}

Point magichexagonEngine::worldPosFromCursor(Point cursorpos)
{
	//Rectangle that the camera can see in world space
	Rect rcCamera = getCameraView();
	
	//Our relative position in window rect space (in rage 0-1)
	cursorpos.x /= (float32)getWidth();
	cursorpos.y /= (float32)getHeight();
	
	//Multiply this by the size of the world rect to get the relative cursor pos
	cursorpos.x = cursorpos.x * rcCamera.width() + rcCamera.left;
	cursorpos.y = cursorpos.y * rcCamera.height() + rcCamera.top;	//Flip on y axis
	
	return cursorpos;
}

void magichexagonEngine::loadConfig(string sFilename)
{
	errlog << "Parsing config file " << sFilename << endl;
	//Open file
	XMLDocument* doc = new XMLDocument;
	int iErr = doc->LoadFile(sFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		errlog << "Error parsing config file: Error " << iErr << ". Ignoring..." << endl;
		if(isFullscreen())
			setInitialFullscreen();
		delete doc;
		return;	//No file; ignore
	}
	
	//Grab root element
	XMLElement* root = doc->RootElement();
	if(root == NULL)
	{
		errlog << "Error: Root element NULL in XML file. Ignoring..." << endl;
		if(isFullscreen())
			setInitialFullscreen();
		delete doc;
		return;
	}
	
	XMLElement* window = root->FirstChildElement("window");
	if(window != NULL)
	{
		bool bFullscreen = isFullscreen();
		bool bMaximized = isMaximized();
		uint32_t width = getWidth();
		uint32_t height = getHeight();
		uint32_t framerate = getFramerate();
		bool bDoubleBuf = getDoubleBuffered();
		bool bPausesOnFocus = pausesOnFocusLost();
		int iVsync = getVsync();
		int iMSAA = getMSAA();
		float32 fGamma = getGamma();
		
		window->QueryUnsignedAttribute("width", &width);
		window->QueryUnsignedAttribute("height", &height);
		window->QueryBoolAttribute("fullscreen", &bFullscreen);
		window->QueryBoolAttribute("maximized", &bMaximized);
		window->QueryUnsignedAttribute("fps", &framerate);
		window->QueryBoolAttribute("doublebuf", &bDoubleBuf);
		window->QueryIntAttribute("vsync", &iVsync);
		window->QueryIntAttribute("MSAA", &iMSAA);
		window->QueryFloatAttribute("brightness", &fGamma);
		window->QueryBoolAttribute("pauseminimized", &bPausesOnFocus);
		
		const char* cWindowPos = window->Attribute("pos");
		if(cWindowPos != NULL)
		{
			Point pos = pointFromString(cWindowPos);
			setWindowPos(pos);
		}
		setFullscreen(bFullscreen);
		changeScreenResolution(width, height);
		if(bMaximized && !isMaximized() && !bFullscreen)
			maximizeWindow();
		setFramerate(framerate);
		setVsync(iVsync);
		setDoubleBuffered(bDoubleBuf);
		setMSAA(iMSAA);
		setGamma(fGamma);
		pauseOnKeyboard(bPausesOnFocus);
	}
	
	delete doc;
}

void magichexagonEngine::saveConfig(string sFilename)
{
	errlog << "Saving config XML " << sFilename << endl;
	XMLDocument* doc = new XMLDocument;
	XMLElement* root = doc->NewElement("config");
	
	XMLElement* window = doc->NewElement("window");
	window->SetAttribute("width", getWidth());
	window->SetAttribute("height", getHeight());
	window->SetAttribute("fullscreen", isFullscreen());
	window->SetAttribute("maximized", isMaximized());
	window->SetAttribute("pos", pointToString(getWindowPos()).c_str());
	window->SetAttribute("fps", (uint32_t)(getFramerate()));
	window->SetAttribute("vsync", getVsync());
	window->SetAttribute("doublebuf", getDoubleBuffered());
	window->SetAttribute("MSAA", getMSAA());
	window->SetAttribute("brightness", getGamma());
	window->SetAttribute("pauseminimized", pausesOnFocusLost());
	root->InsertEndChild(window);
	
	doc->InsertFirstChild(root);
	doc->SaveFile(sFilename.c_str());
	delete doc;
}

obj* magichexagonEngine::objFromXML(string sXMLFilename, Point ptOffset, Point ptVel)
{
	return NULL;
}

void magichexagonEngine::handleKeys()
{
	int prevHex = calcPlayerHex();
	float32 fPrevAngle = m_fPlayerAngle;
	if(keyDown(SDL_SCANCODE_LEFT))
		m_fPlayerAngle += 5;
	if(keyDown(SDL_SCANCODE_RIGHT))
		m_fPlayerAngle -= 5;
	checkSides(fPrevAngle, prevHex, calcPlayerHex());
}




