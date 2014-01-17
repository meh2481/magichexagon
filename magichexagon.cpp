/*
    magichexagon source - myEngine.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "magichexagon.h"
#include "tinyxml2.h"
#include <float.h>
#include <sstream>

//For our engine functions to be able to call our Engine class functions - Note that this means there can be no more than one Engine at a time
//TODO: Think of workaround? How should everything communicate now?
static magichexagonEngine* g_pGlobalEngine;

void signalHandler(string sSignal)
{
    g_pGlobalEngine->hudSignalHandler(sSignal);
}

/*void fillRect(Rect rc, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    g_pGlobalEngine->fillRect(rc, red, green, blue, alpha);
}*/

class QueryCallback : public b2QueryCallback
{
public:
	QueryCallback(const b2Vec2& point)
	{
		m_point = point;
		m_fixture = NULL;
	}

	bool ReportFixture(b2Fixture* fixture)
	{
		b2Body* body = fixture->GetBody();
		if(body->GetType() != b2_dynamicBody)	//Can't click on parasprites directly
		{	
			bool inside = fixture->TestPoint(m_point);
			if(inside && g_pGlobalEngine->_shouldSelect(fixture))	//Let global game engine determine if we should select this
			{
				m_fixture = fixture;

				// We are done, terminate the query.
				return false;
			}
		}
		// Continue the query.
		return true;
	}

	b2Vec2 m_point;
	b2Fixture* m_fixture;
};

bool magichexagonEngine::_shouldSelect(b2Fixture* fix)
{
	if(!fix->IsSensor() && fix->GetBody()->GetUserData() != NULL)
		return true;
	return false;
}

magichexagonEngine::magichexagonEngine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sIcon, bool bResizable) : 
Engine(iWidth, iHeight, sTitle, sIcon, bResizable)
{
	g_pGlobalEngine = this;
	vfs.Prepare();
	
	//Set camera position for this game
	m_fDefCameraZ = -12;
	CameraPos.x = 0;
	CameraPos.y = 0;
	CameraPos.z = m_fDefCameraZ;
	m_bMouseGrabOnWindowRegain = false;//TODO: true;
	
	//Game vars
	m_fRotateAngle = 0;
	m_fRotateAdd = 20;
	m_colors[0].from256(253, 246, 175);
	m_colors[1].from256(248, 185, 207);
	m_colors[2].from256(0, 173, 168);
	
	showCursor();
	
	//m_hud = new HUD("hud");
	//m_hud->create("res/hud.xml");
	
	m_debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_centerOfMassBit);
	b2World* world = getWorld();
	world->SetDebugDraw(&m_debugDraw);
	m_contactListener = new physicsContact();
	world->SetContactListener(m_contactListener);
	
	setTimeScale(DEFAULT_TIMESCALE);	//Speed up time
	m_bDrawDebug = true;
}

magichexagonEngine::~magichexagonEngine()
{
	errlog << "~magichexagonEngine()" << endl;
	saveConfig("res/config.xml");
	//Delete stuffs
	errlog << "Delete contact listener" << endl;
	delete m_contactListener;
	
	//errlog << "delete hud" << endl;
	//delete m_hud;
}

void magichexagonEngine::frame(float32 dt)
{
	handleKeys();
	stepPhysics(dt);	//Update our physics simulation
	updateObjects(dt);
	m_fRotateAngle += m_fRotateAdd * dt;
}

void magichexagonEngine::draw()
{
	//Set up camera and OpenGL flags
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glLoadIdentity();
	glTranslatef(-CameraPos.x, -CameraPos.y, CameraPos.z);
	glRotatef(m_fRotateAngle, 0, 0, 1);
	
	//Draw center hex
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f(m_colors[0].r, m_colors[0].g, m_colors[0].b, m_colors[0].a);
	glPushMatrix();
	glTexCoord2f(0.0, 0.0);
	for(int i = 0; i < 6; i++)
	{
		glBegin(GL_TRIANGLES);
		//center
		glVertex3f(0.0, 0.0, 0.0);
		//left
		glVertex3f(-1.0, 0.0, 0.0);
		//Top left
		glVertex3f(-0.5, 0.866, 0.0);
		glEnd();
		glRotatef(60, 0, 0, 1);
	}
	glPopMatrix();
	
	//Draw objects
	drawObjects();
	
	//Draw debug stuff
	if(m_bDrawDebug)
	{
		glClear(GL_DEPTH_BUFFER_BIT);	//Reset depth buffer (draw geom over everything else)
		b2World* world = getWorld();
		glBindTexture(GL_TEXTURE_2D, 0);
		world->DrawDebugData();
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
	
	//Set gravity to 0
	getWorld()->SetGravity(b2Vec2(0,0));
	
	//Load level
	//levelFromXML("res/levels/scene.xml");
	
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
                    quit();
                    break;
                
                case SDL_SCANCODE_RETURN:
                    if(keyDown(SDL_SCANCODE_ALT))
                      toggleFullscreen();
                    break;
					
				case SDL_SCANCODE_V:	//DEBUG: Visual debug
					m_bDrawDebug = !m_bDrawDebug;
					break;
					
				case SDL_SCANCODE_F:
					//objFromXML("res/objects/parasprite/parasprite.xml", worldPosFromCursor(getCursorPos()));
					break;
				
				case SDL_SCANCODE_F10:
				case SDL_SCANCODE_G:
					if(keyDown(SDL_SCANCODE_CTRL))
						grabMouse(!isMouseGrabbed());	//Toggle grabbing/releasing the mouse
					break;
					
				case SDL_SCANCODE_F5:
					
					break;
					
				case SDL_SCANCODE_1:
					setFramerate(getFramerate() - 1);
					break;
					
				case SDL_SCANCODE_2:
					setFramerate(getFramerate() + 1);
					break;
					
				case SDL_SCANCODE_3:
					setFramerate(5);
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
            if(event.button.button == SDL_BUTTON_LEFT)	//Left mouse button: pick up apple if there
            {
				
            }
            else if(event.button.button == SDL_BUTTON_RIGHT)	//Right mouse button: Place apple
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

//Do some trig to make sure the camera stays within bounds, zooming in as needed
void magichexagonEngine::cameraBounds()
{	
	Rect rcCamera = getCameraView();
	const float32 tan45_2 = tan(DEG2RAD*45/2);
	const float32 fAspect = (float32)getWidth() / (float32)getHeight();
	
	//Move camera to stay within bounds
	if(rcCamera.top > m_rcBounds.top)	//Set to right vertical position if we're too far up
		CameraPos.y = m_rcBounds.top - tan45_2 * -CameraPos.z;
	if(rcCamera.bottom < m_rcBounds.bottom)
		CameraPos.y = m_rcBounds.bottom + tan45_2 * -CameraPos.z;
	if(rcCamera.left < m_rcBounds.left)
		CameraPos.x = m_rcBounds.left + tan45_2 * -CameraPos.z * fAspect;
	if(rcCamera.right > m_rcBounds.right)
		CameraPos.x = m_rcBounds.right - tan45_2 * -CameraPos.z * fAspect;
		
	//Recalculate camera view rect
	rcCamera = getCameraView();
	
	//Zoom camera in if we're still too far out
	/*if(rcCamera.top > m_rcBounds.top)
	{
		//If we're too far out
		if(-rcCamera.height() > -m_rcBounds.height())
			CameraPos.y = m_rcBounds.bottom + (m_rcBounds.top - m_rcBounds.bottom)/2.0;	//Center first
		else
			CameraPos.y -= (rcCamera.top - m_rcBounds.top);	//Go back
		float32 len = m_rcBounds.top - CameraPos.y;
		CameraPos.z = -(len/tan45_2);	//Zoom in as needed
	}*/
	if(rcCamera.left < m_rcBounds.left)
	{
		if(rcCamera.width() > m_rcBounds.width())
			CameraPos.x = m_rcBounds.left + (m_rcBounds.right - m_rcBounds.left)/2.0;
		else
			CameraPos.x += m_rcBounds.left - rcCamera.left;
		float32 len = (CameraPos.x - m_rcBounds.left)/(fAspect);
		CameraPos.z = -(len/tan45_2);
	}
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
		delete doc;
		return;	//No file; ignore
	}
	
	//Grab root element
	XMLElement* root = doc->RootElement();
	if(root == NULL)
	{
		errlog << "Error: Root element NULL in XML file. Ignoring..." << endl;
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
		window->QueryBoolAttribute("visualdebug", &m_bDrawDebug);
		window->QueryBoolAttribute("pauseminimized", &bPausesOnFocus);
		
		const char* cWindowPos = window->Attribute("pos");
		if(cWindowPos != NULL)
		{
			Point pos = pointFromString(cWindowPos);
			setWindowPos(pos);
		}
		
		changeScreenResolution(width, height);
		setFullscreen(bFullscreen);
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
	window->SetAttribute("visualdebug", m_bDrawDebug);
	window->SetAttribute("pauseminimized", pausesOnFocusLost());
	root->InsertEndChild(window);
	
	doc->InsertFirstChild(root);
	doc->SaveFile(sFilename.c_str());
	delete doc;
}

void physicsContact::BeginContact(b2Contact *contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	if(fixtureA->IsSensor())
	{
		if(fixtureB->IsSensor()) return;							//Both are sensors; oops
		if(fixtureB->GetBody()->GetType() == b2_staticBody)	return;	//Can't gravitate static objects
		
		void* userData = fixtureA->GetUserData();
		if (userData != NULL)
		{
			//gravWell* gw = (gravWell*)userData;
			//b2Body* gravObj = fixtureB->GetBody();
			//gw->trappedObjects.insert(gravObj);		//Add this object to this gravity well
		}
	}

	else if(fixtureB->IsSensor())
	{
		if(fixtureA->IsSensor()) return;
		if(fixtureA->GetBody()->GetType() == b2_staticBody)	return;
		
		void* userData = fixtureB->GetUserData();
		if (userData != NULL)
		{
			//gravWell* gw = (gravWell*)userData;
			//b2Body* gravObj = fixtureA->GetBody();
			//gw->trappedObjects.insert(gravObj);
		}
	}
}

void physicsContact::EndContact(b2Contact *contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	if(fixtureA->IsSensor())
	{
		if(fixtureB->IsSensor()) return;							//Both are sensors; oops
		if(fixtureB->GetBody()->GetType() == b2_staticBody)	return;	//Can't gravitate static objects
		
		void* userData = fixtureA->GetUserData();
		if (userData != NULL)
		{
			//gravWell* gw = (gravWell*)userData;
			//b2Body* gravObj = fixtureB->GetBody();
			//gw->trappedObjects.erase(gw->trappedObjects.find(gravObj));	//Take this item out of the list
		}
	}

	else if(fixtureB->IsSensor())
	{
		if(fixtureA->IsSensor()) return;
		if(fixtureA->GetBody()->GetType() == b2_staticBody)	return;
		
		void* userData = fixtureB->GetUserData();
		if (userData != NULL)
		{
			//gravWell* gw = (gravWell*)userData;
			//b2Body* gravObj = fixtureA->GetBody();
			//gw->trappedObjects.erase(gw->trappedObjects.find(gravObj));
		}
	}
}


obj* magichexagonEngine::objFromXML(string sXMLFilename, Point ptOffset, Point ptVel)
{
	//Load in the XML document
    XMLDocument* doc = new XMLDocument();
    int iErr = doc->LoadFile(sXMLFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		errlog << "Error parsing XML file " << sXMLFilename << ": Error " << iErr << endl;
		delete doc;
		return NULL;
	}

	//Grab root element
    XMLElement* root = doc->FirstChildElement("object");
    if(root == NULL)
	{
		errlog << "Error: No toplevel \"object\" item in XML file " << sXMLFilename << endl;
		delete doc;
		return NULL;
	}
    
	obj* o = new obj;
	//anim* lastani = NULL;
	map<string, physSegment*> mSegNames;	//For keeping track of bodies, so we can hook up joints correctly
	//Start on segments
	for(XMLElement* segment = root->FirstChildElement("segment"); segment != NULL; segment = segment->NextSiblingElement("segment"))
	{
		//Create new segment
		physSegment* seg = new physSegment;
		
		//Get segment name (if any)
		const char* cSegName = segment->Attribute("name");
		if(cSegName != NULL)
			mSegNames[cSegName] = seg;	//Keep track of this segment
		
		//Get image path (if any)
		const char* cImgPath = segment->Attribute("image");
		if(cImgPath != NULL)
			seg->img = getImage(cImgPath);    //Grab the name
			
		//Get image offset
		const char* cOffset = segment->Attribute("imgoffset");
		if(cOffset != NULL)
			seg->pos = pointFromString(cOffset);
			
		//Get image center of rotation
		const char* cRotOffset = segment->Attribute("rotcenter");
		if(cRotOffset != NULL)
			seg->center = pointFromString(cRotOffset);
		
		//Get rotation
		segment->QueryFloatAttribute("rot", &seg->rot);
		
		//Get texel size
		const char* cSize = segment->Attribute("imgsize");
		if(cSize != NULL)
			seg->size = pointFromString(cSize);
			
		//Get texel shear
		const char* cShear = segment->Attribute("shear");
		if(cShear != NULL)
			seg->shear = pointFromString(cShear);
		
		//Get color
		const char* cColor = segment->Attribute("colorize");
		if(cColor != NULL)
			seg->col = colorFromString(cColor);
			
		//Get physics offset
		b2BodyDef bodyDef;
		bodyDef.position.SetZero();
		const char* cPos = segment->Attribute("pos");
		if(cPos != NULL)
			bodyDef.position = pointFromString(cPos);
		bodyDef.position += ptOffset;
		bodyDef.linearVelocity = ptVel;
		
		//If the object is dynamic
		bool isstatic = true;
		segment->QueryBoolAttribute("static", &isstatic);
		if(!isstatic)
			bodyDef.type = b2_dynamicBody;
			
		//Create physicsy stuff, if we should
		if(!segment->NoChildren())
		{
			b2Body* body = getWorld()->CreateBody(&bodyDef);
			seg->body = body;
			body->SetUserData(seg);
			for(XMLElement* elem = segment->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
			{
				const char* cName = elem->Name();
				if(cName == NULL) continue;
				string sName(cName);
				if(sName == "circle")
				{
					b2CircleShape shape;
					
					//Get radius
					shape.m_radius = 1.0f;
					elem->QueryFloatAttribute("radius", &shape.m_radius);
					
					//Get position
					shape.m_p.SetZero();
					const char* cPos = elem->Attribute("pos");
					if(cPos != NULL)
						shape.m_p = pointFromString(cPos);
					
					//See if this should collide with other physical objects (aka a sensor)
					bool collide = true;
					elem->QueryBoolAttribute("collide", &collide);
					
					//Check and see if gravity well
					bool gravity = false;
					elem->QueryBoolAttribute("gravity", &gravity);
					
					//Create fixture
					b2FixtureDef fd;
					fd.shape = &shape;
					fd.isSensor = !collide;
					fd.density = 1.0f;
					elem->QueryFloatAttribute("density", &fd.density);
					fd.friction = 0.1f;
					elem->QueryFloatAttribute("friction", &fd.friction);
					fd.restitution = 0.1f;
					elem->QueryFloatAttribute("restitution", &fd.restitution);
					b2Fixture* fix = body->CreateFixture(&fd);
					
					//Create gravity well if we should
					if(gravity)
					{
						gravWell* well = new gravWell;
						well->sensor = fix;
						well->sensor->SetUserData(well);
						
						//Get gravity mass
						well->wellMass = 5.972;	//Fairly high default mass
						elem->QueryFloatAttribute("gravmass", &well->wellMass);
						
						//m_Wells.push_back(well);
					}
				}
				else if(sName == "box")
				{
					b2PolygonShape box;
					
					//Get position
					Point pos(0,0);
					const char* cPos = elem->Attribute("pos");
					if(cPos != NULL)
						pos = pointFromString(cPos);
					
					//Get size
					Point size(1,1);
					const char* cSize = elem->Attribute("size");
					if(cSize != NULL)
						size = pointFromString(cSize);
						
					//Get rotation angle
					float32 angle = 0.0f;
					elem->QueryFloatAttribute("rot", &angle);
					
					//Create box
					box.SetAsBox(size.x/2.0, size.y/2.0, pos, angle);
					
					//Create fixture
					b2FixtureDef fd;
					fd.shape = &box;
					fd.density = 1.0f;
					elem->QueryFloatAttribute("density", &fd.density);
					fd.friction = 0.1f;
					elem->QueryFloatAttribute("friction", &fd.friction);
					fd.restitution = 0.1f;
					elem->QueryFloatAttribute("restitution", &fd.restitution);
					body->CreateFixture(&fd);
				}
				else if(sName == "polygon")
				{
					//Start with an array of vertices
					Point vertices[b2_maxPolygonVertices];	//Up to 8 vertices
					int count = 0;
					
					//Loop through, finding vertices
					for(XMLElement* vertex = elem->FirstChildElement("point"); vertex != NULL; vertex = vertex->NextSiblingElement("point"))
					{
						Point pos(0,0);
						const char* cPos = vertex->Attribute("pos");
						if(cPos != NULL)
						{
							pos = pointFromString(cPos);
							vertices[count] = pos;
							count++;
							if(count >= b2_maxPolygonVertices) break;	//Ignore vertices past 8
						}
					}
					if(count < 3) continue;	//Sanity check
					
					//Create polygon out of vertices
					b2PolygonShape polygon;
					polygon.Set(vertices, count);
					
					//Create fixture & add to body
					b2FixtureDef fd;
					fd.shape = &polygon;
					fd.density = 1.0f;
					elem->QueryFloatAttribute("density", &fd.density);
					fd.friction = 0.1f;
					elem->QueryFloatAttribute("friction", &fd.friction);
					fd.restitution = 0.1f;
					elem->QueryFloatAttribute("restitution", &fd.restitution);
					body->CreateFixture(&fd);
				}
			}
			
			//Get physics stuff
			//If the object can sleep
			bool sleep = true;
			segment->QueryBoolAttribute("sleep", &sleep);
			body->SetSleepingAllowed(sleep);
			
			//If the object can rotate normally
			bool rotate = true;
			segment->QueryBoolAttribute("rotate", &rotate);
			body->SetFixedRotation(!rotate);
			
			//If the object should follow this one body
			bool parent = false;
			segment->QueryBoolAttribute("parent", &parent);
			if(parent)
				o->body = body;
		}
		
		//Animation for this segment
		const char* cAnimFilename = segment->Attribute("anim");
		if(cAnimFilename != NULL)
		{
			anim* ani = new anim;
			if(!ani->fromXML(cAnimFilename)) 
				delete ani;
			else
			{
				ani->segments.push_back(seg);
				ani->seg = seg;
				if(ani->rotadd)
					ani->origvelfac = seg->rot;
				o->animations.push_back(ani);
			}
		}
			
		o->addSegment(seg);
	}
	
	addObject(o);
	
	//Hook up joints between segments
	for(XMLElement* joint = root->FirstChildElement("joint"); joint != NULL; joint = joint->NextSiblingElement("joint"))
	{
		b2Body* bodyA, *bodyB;	//The two bodies that we'll hook the joint up to
		
		//Get first object
		const char* cSeg1Name = joint->Attribute("seg1");
		if(cSeg1Name != NULL && mSegNames.count(cSeg1Name))
			bodyA = mSegNames[cSeg1Name]->body;
		else continue;
		
		//Get second object
		const char* cSeg2Name = joint->Attribute("seg2");
		if(cSeg2Name != NULL && mSegNames.count(cSeg2Name))
			bodyB = mSegNames[cSeg2Name]->body;
		else continue;
		
		//Get joint type
		const char* cJointType = joint->Attribute("type");
		if(cJointType == NULL) continue;
		string sType = cJointType;
		
		if(sType == "revolute")	//Revolute joint
		{
			//Get joint position
			Point pos;
			const char* cJointPos = joint->Attribute("pos");
			if(cJointPos != NULL)
				pos = pointFromString(cJointPos);
			
			b2RevoluteJointDef jointDef;
			jointDef.Initialize(bodyA, bodyB, bodyA->GetWorldCenter()+pos);
			jointDef.collideConnected = false;
			getWorld()->CreateJoint(&jointDef);
		}
	}
	
	//Read frames from the XML
	for(XMLElement* frameElem = root->FirstChildElement("frame"); frameElem != NULL; frameElem = frameElem->NextSiblingElement("frame"))
	{
		const char* cFrameName = frameElem->Attribute("name");
		if(cFrameName == NULL) 
			continue;
		objframe* f = new objframe;
		const char* cSegments = frameElem->Attribute("segments");
		if(cSegments != NULL)
		{
			istringstream iss(stripCommas(cSegments));
			while(!iss.eof() && !iss.fail())	//Strip individual segment names from this string
			{
				string s;
				if(!(iss >> s)) break;
				if(!mSegNames.count(s)) continue;
				physSegment* seg = mSegNames[s];
				f->segments.push_back(seg);
			}
		}
		else delete f;
		
		//Read decay variables
		const char* cDecayFrame = frameElem->Attribute("decayframe");
		if(cDecayFrame != NULL)
			f->nextframe = cDecayFrame;
		
		frameElem->QueryFloatAttribute("decaytime", &f->decaytime);
		frameElem->QueryFloatAttribute("decayvar", &f->decayvar);
		frameElem->QueryBoolAttribute("flip", &f->velflip);
		
		//Read spawn variables
		const char* cSpawn = frameElem->Attribute("spawn");
		if(cSpawn != NULL)
		{
			f->spawn = cSpawn;
			const char* cSpawnPos = frameElem->Attribute("spawnpos");
			if(cSpawnPos != NULL)
				f->spawnpos = pointFromString(cSpawnPos);
			const char* cSpawnVel = frameElem->Attribute("spawnvel");
			if(cSpawnVel != NULL)
				f->spawnvel = pointFromString(cSpawnVel);
			frameElem->QueryBoolAttribute("spawnaddvelx", &f->spawnaddvelx);
			frameElem->QueryBoolAttribute("spawnaddvely", &f->spawnaddvely);
		}
		
		o->addFrame(f, cFrameName);
	}
	//Set default frame, if there is one
	const char* cDefaultFrame = root->Attribute("defaultframe");
	if(cDefaultFrame != NULL)
		o->setFrame(cDefaultFrame);
	
	//Create animations for these objects
	XMLElement* animation = root->FirstChildElement("anim");
	if(animation != NULL)
	{
		for(XMLElement* elem = animation->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
		{
			bool bSeparate = false;	//If we should separate the animations here
			elem->QueryBoolAttribute("separate", &bSeparate);
			bool bFirst = true;
			
			anim* ani = new anim;
			if(!ani->fromXMLElement(elem)) continue;
			const char* cSegments = elem->Attribute("segment");
			if(cSegments != NULL)
			{
				istringstream iss(stripCommas(cSegments));
				while(!iss.eof() && !iss.fail())	//Strip individual segment names from this string
				{
					string s;
					if(!(iss >> s)) break;
					if(!mSegNames.count(s)) continue;
					physSegment* seg = mSegNames[s];
					
					if(bSeparate)
					{
						if(bFirst)
							bFirst = false;
						else
						{
							ani = new anim;
							ani->fromXMLElement(elem);
						}
						o->animations.push_back(ani);
					}
					
					if(ani->rotadd)
						ani->origvelfac = seg->rot;
					ani->segments.push_back(seg);
					ani->seg = seg;
					
					//HACK: Save original size, center, shear, or position
					if(ani->type & ANIM_SIZE)
						ani->orig = seg->size;
					else if(ani->type & ANIM_CENTER)
						ani->orig = seg->center;
					else if(ani->type & ANIM_SHEAR)
						ani->orig = seg->shear;
					else if(ani->type & ANIM_POS)
						ani->orig = seg->pos;
					
					//SORT OF HACK: Determine if velfac and set type accordingly
					const char* cName = elem->Name();
					if(cName == NULL) continue;
					string sName = cName;
					if(sName == "velfac")
					{
						const char* cType = elem->Attribute("type");
						if(cType == NULL) continue;
						string sType = cType;
						if(sType == "rot")
							ani->dest = &seg->rot;
						else if(sType == "shearx")
							ani->dest = &seg->shear.x;
						else if(sType == "sheary")
							ani->dest = &seg->shear.y;
						else if(sType == "sizex")
							ani->dest = &seg->size.x;
						else if(sType == "sizey")
							ani->dest = &seg->size.y;
						else if(sType == "posx")
							ani->dest = &seg->center.x;
						else if(sType == "posy")
							ani->dest = &seg->center.y;
					}
				}
				if(!bSeparate)
					o->animations.push_back(ani);	
			}
		}
	}
	
	delete doc;
	return o;
}

void magichexagonEngine::handleKeys()
{
	//CameraPos.x = m_placingWell->GetWorldCenter().x;
	//CameraPos.y = m_placingWell->GetWorldCenter().y;
	//cameraBounds();
	
}




