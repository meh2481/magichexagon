/*
    magichexagon source - magichexagon.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "magichexagon.h"
#include "tinyxml2.h"
#include <float.h>
#include <sstream>
#include <iomanip>

//For our engine functions to be able to call our Engine class functions
static magichexagonEngine* g_pGlobalEngine;

void signalHandler(string sSignal)
{
    g_pGlobalEngine->hudSignalHandler(sSignal);
}

magichexagonEngine::magichexagonEngine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sAppName, string sIcon, bool bResizable) : 
Engine(iWidth, iHeight, sTitle, sAppName, sIcon, bResizable)
{
	g_pGlobalEngine = this;
	vfs.Prepare();
	
	//Set camera position for this game
	m_fDefCameraZ = -15;
	CameraPos.x = 0;
	CameraPos.y = 0;
	CameraPos.z = m_fDefCameraZ;
#ifdef DEBUG
	m_bMouseGrabOnWindowRegain = false;
#else
	m_bMouseGrabOnWindowRegain = true;
#endif
	m_iCurMenu = MENU_START;
	m_fRotateAngle = 0;
	m_iCurLevel = 0;
	m_bFirstRun = false;
	m_bLeftPressed = m_bRightPressed = false;
	m_bUnlocked = m_bSideComplete = false;
	//m_fPressTimer = 2.0;
	
	showCursor();
	
	m_hud = new HUD("hud");
	m_hud->create("res/hud.xml");
	m_hud->setScene("start");
	
	for(int i = 0; i < 6; i++)
		m_fBestTime[i] = 0;
	
	setTimeScale(DEFAULT_TIMESCALE);	//Speed up time
}

magichexagonEngine::~magichexagonEngine()
{
	errlog << "~magichexagonEngine()" << endl;
	saveConfig(getSaveLocation() + "config.xml");
	//Delete stuffs
	errlog << "delete hud" << endl;
	delete m_hud;
}

void magichexagonEngine::frame(float32 dt)
{
	switch(m_iCurMenu)
	{
		case MENU_START:
			m_fRotateAngle += -60 * dt;
			break;
			
		case MENU_LEVELSELECT:
			updateColors(dt*getTimeScale());
			break;
			
		case MENU_NONE:
			handleKeys();
			updateColors(dt*getTimeScale());
			updateLevel(dt*getTimeScale());
			break;
		
		case MENU_GAMEOVER:
			CameraPos.z += 0.2*dt;
			m_fRotateAngle += 2*dt;
			if(CameraPos.z > -5)
				CameraPos.z = -5;
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
	
	//Change color for arrows, depending on which keys pressed
	HUDItem* arrow = m_hud->getChild("arrow_l");
	if(arrow != NULL)
	{
		if(keyDown(SDL_SCANCODE_LEFT) || keyDown(SDL_SCANCODE_A) || getCursorDown(LMB))
			arrow->col.set(1,0.4,0.4);
		else
			arrow->col.set(1,1,1);
	}
	arrow = m_hud->getChild("arrow_r");
	if(arrow != NULL)
	{
		if(keyDown(SDL_SCANCODE_RIGHT) || keyDown(SDL_SCANCODE_D) || getCursorDown(RMB))
			arrow->col.set(1,0.4,0.4);
		else
			arrow->col.set(1,1,1);
	}
	
	switch(m_iCurMenu)
	{
		case MENU_START:
			drawStartMenu();
			break;
			
		case MENU_LEVELSELECT:
			drawLevelSelectMenu();
			break;
			
		case MENU_NONE:
			renderLevel();
			break;
			
		case MENU_GAMEOVER:
			renderLevel();
			break;
	}
	
	//Draw HUD always at this depth, on top of everything else
	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, m_fDefCameraZ);
	m_hud->draw(0);
}

void magichexagonEngine::init(list<commandlineArg> sArgs)
{
	//Run through list for arguments we recognize
	for(list<commandlineArg>::iterator i = sArgs.begin(); i != sArgs.end(); i++)
	{
		errlog << "Commandline argument. Switch: " << i->sSwitch << ", value: " << i->sValue << endl;
	}
	
	//Load our last screen position and such
	loadConfig(getSaveLocation() + "config.xml");
	loadPatterns("res/patterns.xml");
	
	//Set gravity to 0
	getWorld()->SetGravity(b2Vec2(0,0));
	
	//Create sounds up front
	//vox
	createSound("res/sfx/begin.ogg", "begin");
	createSound("res/sfx/excellent.ogg", "excellent");
	createSound("res/sfx/gameover.ogg", "gameover");
	createSound("res/sfx/magichexagon.ogg", "magichexagon");
	
	createSound("res/sfx/generosity.ogg", "generosity");
	createSound("res/sfx/honesty.ogg", "honesty");
	createSound("res/sfx/kindness.ogg", "kindness");
	createSound("res/sfx/laughter.ogg", "laughter");
	createSound("res/sfx/loyalty.ogg", "loyalty");
	createSound("res/sfx/magic.ogg", "magic");
	
	createSound("res/sfx/nice.ogg", "nice");
	createSound("res/sfx/wonderful.ogg", "wonderful");
	createSound("res/sfx/awesome.ogg", "awesome");
	//sfx
	createSound("res/sfx/beginlevel.ogg", "beginlevel");	//When you enter a level
	createSound("res/sfx/menubegin.ogg", "menubegin");		//Initial keypress: Title->menu
	createSound("res/sfx/die.ogg", "die");					//When you DIIIIIE
	createSound("res/sfx/select.ogg", "select");			//When you're selecting different menu items
	
	
	//Play music
	playMusic("res/sfx/encore-micro_hexagon_courtesy.ogg");
	pauseMusic();
	playSound("magichexagon");
	
	//TODO seekMusic(34.504028);
	
	hideCursor();
}


void magichexagonEngine::hudSignalHandler(string sSignal)
{
}

void magichexagonEngine::handleEvent(SDL_Event event)
{
    m_hud->event(event);    //Let our HUD handle any events it needs to
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
							die();
							break;
							
						case MENU_GAMEOVER:
							if(m_bSideComplete)
							{
								playSound("menubegin");
								setComplete();
							}
							else
							{
								pauseMusic();
								playSound("menubegin");
								m_iCurMenu = MENU_LEVELSELECT;
								m_iCurLevel = m_iStartLevel;
								CameraPos.z = m_fDefCameraZ;
								m_hud->setScene("levelselect");
								setMenuColors();
							}
							break;
							
						case MENU_LEVELSELECT:
							playSound("menubegin");
							m_iCurMenu = MENU_START;
							m_hud->setScene("start");
							break;
							
						case MENU_START:
							quit();
							break;
					}
                    break;
				
				case SDL_SCANCODE_LEFT:
				case SDL_SCANCODE_A:
					if(m_iCurMenu == MENU_LEVELSELECT)
					{
						playSound("select");
						m_iCurLevel--;
						if(m_iCurLevel < 0)
							m_iCurLevel = 5;
						highlightLevel();
					}
					break;
				
				case SDL_SCANCODE_RIGHT:
				case SDL_SCANCODE_D:
					if(m_iCurMenu == MENU_LEVELSELECT)
					{
						playSound("select");
						m_iCurLevel++;
						if(m_iCurLevel > 5)
							m_iCurLevel = 0;
						highlightLevel();
					}
					break;
                
                case SDL_SCANCODE_RETURN:
                    if(keyDown(SDL_SCANCODE_ALT))
                    {
						toggleFullscreen();
						break;
					}
				case SDL_SCANCODE_SPACE:
					switch(m_iCurMenu)
					{
						case MENU_START:
							playSound("menubegin");
							m_iCurMenu = MENU_LEVELSELECT;
							m_hud->setScene("levelselect");
							m_iCurLevel = 0;
							setMenuColors();
							break;
							
						case MENU_LEVELSELECT:
							if(m_iCurLevel < 3 || m_fBestTime[m_iCurLevel-3] >= 60.0)
							{
								playMusic("res/sfx/encore-micro_hexagon_courtesy.ogg");
								playSound("begin");
								playSound("beginlevel");
								restartMusic();
								m_iCurMenu = MENU_NONE;
								m_iStartLevel = m_iCurLevel;
								m_hud->setScene("level");
								resetLevel();
							}
							break;
						
						case MENU_GAMEOVER:
							if(m_bSideComplete)
							{
								playSound("menubegin");
								setComplete();
							}
							else
							{
								playMusic("res/sfx/encore-micro_hexagon_courtesy.ogg");
								playSound("begin");
								playSound("beginlevel");
								restartMusic();
								m_iCurLevel = m_iStartLevel;
								m_iCurMenu = MENU_NONE;
								m_hud->setScene("level");
								resetLevel();
							}
							break;	
					}
					break;
#ifdef DEBUG
				case SDL_SCANCODE_Q:
					changeLevel(LEVEL_FRIENDSHIP);
					m_fTargetSpinTime = FLT_MAX;
					break;
					
				case SDL_SCANCODE_W:
					changeLevel(LEVEL_HONESTY);
					m_fTargetSpinTime = FLT_MAX;
					break;
					
				case SDL_SCANCODE_E:
					changeLevel(LEVEL_KINDNESS);
					m_fTargetSpinTime = FLT_MAX;
					break;
					
				case SDL_SCANCODE_R:
					changeLevel(LEVEL_LOYALTY);
					m_fTargetSpinTime = FLT_MAX;
					break;
					
				case SDL_SCANCODE_T:
					changeLevel(LEVEL_GENEROSITY);
					m_fTargetSpinTime = FLT_MAX;
					break;
					
				case SDL_SCANCODE_Y:
					changeLevel(LEVEL_LAUGHTER);
					m_fTargetSpinTime = FLT_MAX;
					break;
					
				case SDL_SCANCODE_U:
					changeLevel(LEVEL_MAGIC);
					m_fTargetSpinTime = FLT_MAX;
					break;
#endif
				case SDL_SCANCODE_F10:
				case SDL_SCANCODE_G:
					if(keyDown(SDL_SCANCODE_CTRL))
					{
						grabMouse(!isMouseGrabbed());	//Toggle grabbing/releasing the mouse
					}
					break;
					
				case SDL_SCANCODE_F5:
				{
					m_Patterns.clear();
					loadPatterns("res/patterns.xml");	//Reload patterns
					string sScene = m_hud->getScene();
					delete m_hud;
					m_hud = new HUD("hud");
					m_hud->create("res/hud.xml");
					m_hud->setScene(sScene);
					break;
				}
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
				if(!isMouseGrabbed())
					grabMouse(true);
				else
				{
					if(m_iCurMenu == MENU_LEVELSELECT)
					{
						playSound("select");
						m_iCurLevel--;
						if(m_iCurLevel < 0)
							m_iCurLevel = 5;
						highlightLevel();
					}
				}
            }
            else if(event.button.button == SDL_BUTTON_RIGHT)
            {
				if(m_iCurMenu == MENU_LEVELSELECT)
				{
					playSound("select");
					m_iCurLevel++;
					if(m_iCurLevel > 5)
						m_iCurLevel = 0;
					highlightLevel();
				}
            }
			else if(event.button.button == SDL_BUTTON_MIDDLE)
			{
				switch(m_iCurMenu)
				{
					case MENU_START:
						playSound("menubegin");
						m_iCurMenu = MENU_LEVELSELECT;
						m_hud->setScene("levelselect");
						m_iCurLevel = 0;
						setMenuColors();
						break;
						
					case MENU_LEVELSELECT:
						if(m_iCurLevel < 3 || m_fBestTime[m_iCurLevel-3] >= 60.0)
						{
							playMusic("res/sfx/encore-micro_hexagon_courtesy.ogg");
							playSound("begin");
							playSound("beginlevel");
							restartMusic();
							m_iCurMenu = MENU_NONE;
							m_iStartLevel = m_iCurLevel;
							m_hud->setScene("level");
							resetLevel();
						}
						break;
					
					case MENU_GAMEOVER:
						if(m_bSideComplete)
						{
							playSound("menubegin");
							setComplete();
						}
						else
						{
							playMusic("res/sfx/encore-micro_hexagon_courtesy.ogg");
							playSound("begin");
							playSound("beginlevel");
							restartMusic();
							m_iCurLevel = m_iStartLevel;
							m_iCurMenu = MENU_NONE;
							m_hud->setScene("level");
							resetLevel();
						}
						break;	
				}
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

void magichexagonEngine::pause()
{
	if(m_iCurMenu == MENU_NONE)
		pauseMusic();
}

void magichexagonEngine::resume()
{
	if(m_iCurMenu == MENU_NONE)
		resumeMusic();
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
		m_bFirstRun = true;
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
		m_bFirstRun = true;
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
	
	XMLElement* times = root->FirstChildElement("besttimes");
	if(times != NULL)
	{
		times->QueryFloatAttribute("level1", &m_fBestTime[0]);
		times->QueryFloatAttribute("level2", &m_fBestTime[1]);
		times->QueryFloatAttribute("level3", &m_fBestTime[2]);
		times->QueryFloatAttribute("level4", &m_fBestTime[3]);
		times->QueryFloatAttribute("level5", &m_fBestTime[4]);
		times->QueryFloatAttribute("level6", &m_fBestTime[5]);
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
	
	XMLElement* times = doc->NewElement("besttimes");
	times->SetAttribute("level1", m_fBestTime[0]);
	times->SetAttribute("level2", m_fBestTime[1]);
	times->SetAttribute("level3", m_fBestTime[2]);
	times->SetAttribute("level4", m_fBestTime[3]);
	times->SetAttribute("level5", m_fBestTime[4]);
	times->SetAttribute("level6", m_fBestTime[5]);
	root->InsertEndChild(times);
	
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
#ifdef DEBUG
	if(keyDown(SDL_SCANCODE_B))
		setTimeScale(DEFAULT_TIMESCALE/3);
	else
		setTimeScale(DEFAULT_TIMESCALE);
#endif
	int prevHex = calcPlayerHex();
	float32 fPrevAngle = m_fPlayerAngle;
	if(keyDown(SDL_SCANCODE_LEFT) || keyDown(SDL_SCANCODE_A) || getCursorDown(LMB))
	{
		m_fPlayerAngle += m_fPlayerMove * getTimeScale() * 60.0/(float)getFramerate();
		m_bLeftPressed = true;
	}
	if(keyDown(SDL_SCANCODE_RIGHT) || keyDown(SDL_SCANCODE_D) || getCursorDown(RMB))
	{
		m_fPlayerAngle -= m_fPlayerMove * getTimeScale() * 60.0/(float)getFramerate();
		m_bRightPressed = true;
	}
	checkSides(fPrevAngle, prevHex, calcPlayerHex());
}

void magichexagonEngine::drawStartMenu()
{
	glPushMatrix();
	//Rotate by how much we're spinning
	glTranslatef(0, -8, 0);
	glRotatef(m_fRotateAngle, 0, 0, 1);
	glTexCoord2f(0.0, 0.0);
	//Get how large our screenspace is
	Point ptWorldSize(getWidth(), getHeight());
	ptWorldSize = worldMovement(ptWorldSize);	//Get the actual world movement in texels
	float fDrawSize = ptWorldSize.Length() * 2.0;
	for(int i = 0; i < 6; i++)
	{
		if(i % 2)
			glColor4f(0.8, 0.8, 0.8, 1.0);
		else
			glColor4f(0.6, 0.6, 0.6, 1.0);
		glBegin(GL_TRIANGLES);
		//center
		glVertex3f(0.0, 0.0, 0.0);
		//Left
		glVertex3f(-fDrawSize, 0.0, 0.0);
		//Top left
		glVertex3f(-0.5*fDrawSize, 0.866*fDrawSize, 0.0);
		glEnd();
		glRotatef(60, 0, 0, 1);
	}
	glPopMatrix();
	
	//Because we're spinning evenly anyway, hack in a blink effect to the seizure warning logo
	HUDItem* seizureWarning = m_hud->getChild("titlewarning");
	if(seizureWarning != NULL)
	{
		if((int)(m_fRotateAngle / 50.0) % 2)
			seizureWarning->hidden = true;
		else
			seizureWarning->hidden = false;
	}
}

void magichexagonEngine::bestTime(HUDTextbox* it, string s, float fTime)
{
	if(fTime > 0)
	{
		ostringstream oss;
		oss.precision(2);
		oss.setf(ios::fixed, ios::floatfield);
		oss << s << fTime;
		it->setText(oss.str());
	}
	else
		it->hidden = true;
}

void magichexagonEngine::drawLevelSelectMenu()
{
	m_fRotateAngle = 0.0f;
	//centerCutie = NULL;
	m_fPlayerAngle = -60.0f * m_iCurLevel - 90;
	for(int i = 0; i < 6; i++)
	{
		if(m_walls[i].size())
			m_walls[i].clear();
	}
	
	renderLevel();

	//Update level locked/unlocked labels
	HUDItem* locked = m_hud->getChild("lev4locked");
	HUDItem* levelname = m_hud->getChild("lev4name");
	HUDItem* leveldif = m_hud->getChild("lev4diff");
	if(locked != NULL && levelname != NULL && leveldif != NULL)
	{
		if(m_fBestTime[0] >= 60.0)
		{
			locked->hidden = true;
			levelname->hidden = false;
			leveldif->hidden = false;
		}
		else
		{
			locked->hidden = false;
			levelname->hidden = true;
			leveldif->hidden = true;
		}
	}
	
	locked = m_hud->getChild("lev5locked");
	levelname = m_hud->getChild("lev5name");
	leveldif = m_hud->getChild("lev5diff");
	if(locked != NULL && levelname != NULL && leveldif != NULL)
	{
		if(m_fBestTime[1] >= 60.0)
		{
			locked->hidden = true;
			levelname->hidden = false;
			leveldif->hidden = false;
		}
		else
		{
			locked->hidden = false;
			levelname->hidden = true;
			leveldif->hidden = true;
		}
	}
	
	locked = m_hud->getChild("lev6locked");
	levelname = m_hud->getChild("lev6name");
	leveldif = m_hud->getChild("lev6diff");
	if(locked != NULL && levelname != NULL && leveldif != NULL)
	{
		if(m_fBestTime[2] >= 60.0)
		{
			locked->hidden = true;
			levelname->hidden = false;
			leveldif->hidden = false;
		}
		else
		{
			locked->hidden = false;
			levelname->hidden = true;
			leveldif->hidden = true;
		}
	}
	
	//Update best level times
	HUDItem* besttime = m_hud->getChild("lev1time");
	bestTime((HUDTextbox*)besttime, "best time: ", m_fBestTime[0]);
	besttime = m_hud->getChild("lev2time");
	bestTime((HUDTextbox*)besttime, "best time: ", m_fBestTime[1]);
	besttime = m_hud->getChild("lev3time");
	bestTime((HUDTextbox*)besttime, "best time: ", m_fBestTime[2]);
	besttime = m_hud->getChild("lev4time");
	bestTime((HUDTextbox*)besttime, "best time: ", m_fBestTime[3]);
	besttime = m_hud->getChild("lev5time");
	bestTime((HUDTextbox*)besttime, "best time: ", m_fBestTime[4]);
	besttime = m_hud->getChild("lev6time");
	bestTime((HUDTextbox*)besttime, "best time: ", m_fBestTime[5]);
}


