/*
    CutsceneEditor source - Engine.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "Engine.h"
#include <SDL2/SDL_syswm.h>
ofstream errlog("err.log");


GLfloat LightAmbient[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
// Diffuse Light Values
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
// Light Position 
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

void PrintEvent(const SDL_Event * event)
{
    if (event->type == SDL_WINDOWEVENT) {
        switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            printf("Window %d shown\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            printf("Window %d hidden\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            printf("Window %d exposed\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MOVED:
            printf("Window %d moved to %d,%d\n",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            printf("Window %d resized to %dx%d\n",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            printf("Window %d minimized\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            printf("Window %d maximized\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            printf("Window %d restored\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            printf("Mouse entered window %d\n",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            printf("Mouse left window %d\n", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            printf("Window %d gained keyboard focus\n",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            printf("Window %d lost keyboard focus\n",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            printf("Window %d closed\n", event->window.windowID);
            break;
        default:
            printf("Window %d got unknown event %d\n",
                    event->window.windowID, event->window.event);
            break;
        }
    }
}

bool Engine::_frame()
{
	tyrsound_update();
    //Handle input events from SDL
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
		//PrintEvent(&event);
        //Update internal cursor position if cursor has moved
        if(event.type == SDL_MOUSEMOTION)
        {
            m_ptCursorPos.x = event.motion.x;
            m_ptCursorPos.y = event.motion.y;
        }
		if(event.type == SDL_WINDOWEVENT)
		{
			if(event.window.event == SDL_WINDOWEVENT_FOCUS_LOST && m_bPauseOnKeyboardFocus)
			{
				m_bPaused = true;
				pauseMusic();
			}
			else if(event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED && m_bPauseOnKeyboardFocus)
			{
				m_bPaused = false;
				resumeMusic();
			}
			else if(event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				if(m_bResizable)
					changeScreenResolution(event.window.data1, event.window.data2);
				else
					errlog << "Error! Resize event generated, but resizable flag not set." << endl;
			}
		}
        if(event.type == SDL_QUIT)
            return true;
		if(!m_bPaused)
			handleEvent(event);
    }
	if(m_bPaused)
	{
		SDL_Delay(100);	//Wait 100 ms
		return m_bQuitting;	//Break out here
	}

    float32 fCurTime = ((float32)SDL_GetTicks())/1000.0;
    if(m_fAccumulatedTime <= fCurTime)
    {
        m_fAccumulatedTime += m_fTargetTime;
		m_iKeystates = SDL_GetKeyboardState(NULL);	//Get current key state
        frame(m_fTargetTime);	//Box2D wants fixed timestep, so we use target framerate here instead of actual elapsed time
        _render();
    }

    if(m_fAccumulatedTime + m_fTargetTime * 3.0 < fCurTime)    //We've gotten far too behind; we could have a huge FPS jump if the load lessens
        m_fAccumulatedTime = fCurTime;   //Drop any frames past this
    return m_bQuitting;
}

void Engine::_render()
{
    // Begin rendering by clearing the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Game-specific drawing
    draw();
	
	//Draw gamma/brightness overlay on top of everything else
	glEnable(GL_BLEND);
	if(m_fGamma > 1.0f)
	{
		glBlendFunc(GL_DST_COLOR, GL_ONE);
		glColor3f(m_fGamma - 1.0, m_fGamma - 1.0, m_fGamma - 1.0);
	}
	else
	{
		glBlendFunc( GL_ZERO, GL_SRC_COLOR );
		glColor3f(m_fGamma, m_fGamma, m_fGamma);
	}
	//Fill whole screen with rect ( Example taken from http://yuhasapoint.blogspot.com/2012/07/draw-quad-that-fills-entire-opengl.html on 11/20/13)
	glBindTexture(GL_TEXTURE_2D, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
	glVertex3i(-1, -1, -1);
	glVertex3i(1, -1, -1);
	glVertex3i(1, 1, -1);
	glVertex3i(-1, 1, -1);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	//Reset blend func & color
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor3f(1.0, 1.0, 1.0);
	
	//Draw cursor at absolute location
	//glPushMatrix();
	//glLoadIdentity();
	//glTranslatef( 0.0f, 0.0f, MAGIC_ZOOM_NUMBER);
	//glClear(GL_DEPTH_BUFFER_BIT); //TODO Draw cursor over everything
	//glPopMatrix();
	
    // End rendering and update the screen
     SDL_GL_SwapWindow(m_Window);
}

Engine::Engine(uint16_t iWidth, uint16_t iHeight, string sTitle, string sIcon, bool bResizable)
{
	m_sTitle = sTitle;
	m_sIcon = sIcon;
	m_bResizable = bResizable;
    b2Vec2 gravity(0.0, -9.8);  //Vector for our world's gravity
    m_physicsWorld = new b2World(gravity);
    m_ptCursorPos.SetZero();
    m_physicsWorld->SetAllowSleeping(true);
    m_iWidth = iWidth;
    m_iHeight = iHeight;
    m_iKeystates = NULL;
    m_bShowCursor = true;
    setFramerate(60);   //60 fps default
    m_bFullscreen = false;
    setup_sdl();
    setup_opengl();
	m_fGamma = 1.0f;
	m_bPaused = false;
	m_bPauseOnKeyboardFocus = true;

    //Initialize engine stuff
    m_fAccumulatedTime = 0.0;
    //m_bFirstMusic = true;
    m_bQuitting = false;
    srand(SDL_GetTicks());  //Not as random as it could be... narf
	m_fTimeScale = 1.0f;

	if(tyrsound_init(NULL, NULL) != TYRSOUND_ERR_OK)
        errlog << "Failed to init tyrsound." << std::endl;
}

Engine::~Engine()
{
	SDL_DestroyWindow(m_Window);

    //Clean up our image map
	errlog << "Clearing images" << endl;
    clearImages();

    //Clean up our sound effects
	for(map<string, tyrsound_Handle>::iterator i = m_sounds.begin(); i != m_sounds.end(); i++)
		tyrsound_unload(i->second);
	
	//Clean up tyrsound
	tyrsound_shutdown();

    // Clean up and shutdown
	errlog << "Deleting phys world" << endl;
	delete m_physicsWorld;
	errlog << "Quit SDL" << endl;
	SDL_Quit();
}

void Engine::start()
{
    // Load all that we need to
    init(lCommandLine);
    // Let's rock now!
    while(!_frame());
}

/*void Engine::fillRect(Point p1, Point p2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    fillRect(p1.x, p1.y, p2.x, p2.y, red, green, blue, alpha);
}

void Engine::fillRect(float32 x1, float32 y1, float32 x2, float32 y2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    Color col;
    col.from256(red, green, blue, alpha);
	fillRect(x1, y1, x2, y2, col);
}

void Engine::fillRect(Rect rc, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    fillRect(rc.left, rc.top, rc.right, rc.bottom, red, green, blue, alpha);
}

void Engine::fillRect(float32 x1, float32 y1, float32 x2, float32 y2, Color col)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glBegin(GL_QUADS);
    glColor4f(col.r,col.g,col.b,col.a);	//Colorize
    //Draw (No, I have no idea how these formulas work, either)
    glVertex3f((2.0*(float32)m_iWidth/(float32)m_iHeight)*((GLfloat)x1/(GLfloat)m_iWidth-0.5), -2.0*(GLfloat)y1/(GLfloat)m_iHeight + 1.0, 0.0);
    glVertex3f((2.0*(float32)m_iWidth/(float32)m_iHeight)*((GLfloat)x1/(GLfloat)m_iWidth-0.5), -2.0*(GLfloat)y2/(GLfloat)m_iHeight+1.0, 0.0);
    glVertex3f((2.0*(float32)m_iWidth/(float32)m_iHeight)*((GLfloat)x2/(GLfloat)m_iWidth-0.5), -2.0*(GLfloat)y2/(GLfloat)m_iHeight+1.0, 0.0);
    glVertex3f((2.0*(float32)m_iWidth/(float32)m_iHeight)*((GLfloat)x2/(GLfloat)m_iWidth-0.5), -2.0*(GLfloat)y1/(GLfloat)m_iHeight+1.0, 0.0);
    glEnd();
    glColor4f(1.0,1.0,1.0,1.0);	//Back to normal
}*/

void Engine::createSound(string sPath, string sName)
{
	tyrsound_Stream strm;
    if(tyrsound_createFileNameStream(&strm, sPath.c_str(), "rb") != TYRSOUND_ERR_OK)
    {
        errlog << "File not found: " << sPath << endl;
        return;
    }

    tyrsound_Handle handle = tyrsound_load(strm);

    if(handle == TYRSOUND_NULLHANDLE)
    {
        errlog << "Invalid handle for song " << sPath << endl;
        return;
    }
	m_sounds[sName] = handle;
	tyrsound_play(handle);
	tyrsound_pause(handle);
}

void Engine::playSound(string sName, int volume, int pan, float32 pitch)
{
	tyrsound_Handle handle = m_sounds[sName];
    tyrsound_setVolume(handle, (float)(volume)/100.0);
	tyrsound_setSpeed(handle, pitch);
	tyrsound_setPosition(handle, pan, 0, 0);
	//TODO: Play sounds more than once at a time
	tyrsound_stop(handle);
	tyrsound_seek(handle, 0);
    tyrsound_play(handle);
}

void Engine::pauseMusic()
{
    tyrsound_pause(m_sounds["music"]);
}

void Engine::restartMusic()
{
	tyrsound_Handle handle = m_sounds["music"];
	tyrsound_stop(handle);
	tyrsound_seek(handle, 0);
    tyrsound_play(handle);
}

void Engine::resumeMusic()
{
	if(m_sounds.count("music"))
		tyrsound_play(m_sounds["music"]);
}

void Engine::seekMusic(float32 fTime)
{
	tyrsound_seek(m_sounds["music"], fTime);
}

void Engine::playMusic(string sName, int volume, int pan, float32 pitch)
{
	if(!m_sounds.count("music"))
		createSound(sName, "music");
	tyrsound_setLoop(m_sounds["music"], 0.0f, -1);
	playSound("music", volume, pan, pitch);
}

bool Engine::keyDown(int32_t keyCode)
{
	if(m_iKeystates == NULL) return false;	//On first cycle, this can be NULL and cause segfaults otherwise
	
	//HACK: See if one of our combined keycodes
	if(keyCode == SDL_SCANCODE_CTRL) return (keyDown(SDL_SCANCODE_LCTRL)|keyDown(SDL_SCANCODE_RCTRL));
	if(keyCode == SDL_SCANCODE_SHIFT) return (keyDown(SDL_SCANCODE_LSHIFT)|keyDown(SDL_SCANCODE_RSHIFT));
	if(keyCode == SDL_SCANCODE_ALT) return (keyDown(SDL_SCANCODE_LALT)|keyDown(SDL_SCANCODE_RALT));
	if(keyCode == SDL_SCANCODE_GUI) return (keyDown(SDL_SCANCODE_LGUI)|keyDown(SDL_SCANCODE_RGUI));
	
	//Otherwise, just use our pre-polled list we got from SDL
    return(m_iKeystates[keyCode]);
}

void Engine::setFramerate(float32 fFramerate)
{
    if(m_fFramerate == 0.0)
        m_fAccumulatedTime = (float32)SDL_GetTicks()/1000.0;   //If we're stuck at 0fps for a while, this number could be huge, which would cause unlimited fps for a bit
    m_fFramerate = fFramerate;
    if(m_fFramerate == 0.0)
        m_fTargetTime = FLT_MAX;    //Avoid division by 0
    else
        m_fTargetTime = 1.0 / m_fFramerate;
}

#include "opengl-api.h"

void Engine::setup_sdl()
{

	if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
		errlog << "SDL_InitSubSystem Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    errlog << "Loading OpenGL..." << std::endl;

    if (SDL_GL_LoadLibrary(NULL) == -1)
    {
		errlog << "SDL_GL_LoadLibrary Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

  // Quit SDL properly on exit
  atexit(SDL_Quit);
  
  // Create SDL window
  Uint32 flags = SDL_WINDOW_OPENGL;
  if(m_bResizable)
	flags |= SDL_WINDOW_RESIZABLE;
  
  m_Window = SDL_CreateWindow(m_sTitle.c_str(),
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             m_iWidth, 
							 m_iHeight,
                             flags);

  if(m_Window == NULL)
  {
  	errlog << "Couldn't set video mode: " << SDL_GetError() << endl;
    exit(1);
  }
  SDL_GL_CreateContext(m_Window);

  
  // Set the minimum requirements for the OpenGL window
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  
  //Vsync and stuff	//TODO: Toggle? Figure out what it's actually doing? My pathetic gfx card doesn't do anything with any of these values
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);	//Apparently double-buffering or something
  SDL_GL_SetSwapInterval(1);					//Apparently Vsync or something
  //Apparently MSAA or something
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

  SDL_DisplayMode mode;
  SDL_GetDisplayMode(0, 0, &mode);
  errlog << "Default monitor refresh rate: " << mode.refresh_rate << " Hz" << endl;
  setFramerate(mode.refresh_rate);
  
  
  //Hide system cursor for SDL, so we can use our own
  SDL_ShowCursor(0);
  
  OpenGLAPI::LoadSymbols();	//Load our OpenGL symbols to use
  
  _loadicon();	//Load our window icon
  
}

//Set up OpenGL
void Engine::setup_opengl()
{
	// Make the viewport
    glViewport(0, 0, m_iWidth, m_iHeight);

    // set the clear color to black
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    glEnable(GL_TEXTURE_2D);

    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
  
    //Enable image transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set the camera projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective( 45.0f, (GLfloat)m_iWidth/(GLfloat)m_iHeight, 0.1f, 500.0f );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glPushMatrix();
	
	//Set up lighting
    glShadeModel(GL_SMOOTH);
    glEnable( GL_LIGHT0 );
    glEnable( GL_LIGHT1 );
    glEnable( GL_COLOR_MATERIAL );

    // Setup The Ambient Light
    glLightfv( GL_LIGHT1, GL_AMBIENT, LightAmbient );

    // Setup The Diffuse Light
    glLightfv( GL_LIGHT1, GL_DIFFUSE, LightDiffuse );

    // Position The Light
    glLightfv( GL_LIGHT1, GL_POSITION, LightPosition );

    // Enable Light One
    glEnable( GL_LIGHT1 ); 
    
}

void Engine::setMSAA(int iMSAA)
{
	if(iMSAA)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, iMSAA);
		
		// Enable OpenGL antialiasing stuff
		glEnable(GL_MULTISAMPLE);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		
		// Disable OpenGL antialiasing stuff
		glDisable(GL_MULTISAMPLE);
		glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POLYGON_SMOOTH);
	}
}

void Engine::_loadicon()	//Load icon into SDL window
{
	errlog << "Load icon " << m_sIcon << endl;
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP *dib(0);
	BYTE* bits(0);
	unsigned int width(0), height(0);
	
	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(m_sIcon.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(m_sIcon.c_str());
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN)
	{
		errlog << "Unknown image type for file " << m_sIcon << endl;
		return;
	}
  
	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, m_sIcon.c_str());
	else
		errlog << "File " << m_sIcon << " doesn't support reading." << endl;
	//if the image failed to load, return failure
	if(!dib)
	{
		errlog << "Error loading image " << m_sIcon.c_str() << endl;
		return;
	}  
	//retrieve the image data
  
	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	
	FreeImage_FlipVertical(dib);
  
	bits = FreeImage_GetBits(dib);	//if this somehow one of these failed (they shouldn't), return failure
	if((bits == 0) || (width == 0) || (height == 0))
	{
		errlog << "Something went terribly horribly wrong with getting image bits; just sit and wait for the singularity" << endl;
		return;
	}
	
	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(bits, width, height, FreeImage_GetBPP(dib), FreeImage_GetPitch(dib), 
													0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	if(surface == NULL)
		errlog << "NULL surface for icon " << m_sIcon << endl;
	else
	{
		SDL_SetWindowIcon(m_Window, surface);
		SDL_FreeSurface(surface);
	}
  
	FreeImage_Unload(dib);
}

void Engine::changeScreenResolution(float32 w, float32 h)
{
	errlog << "Changing screen resolution to " << w << ", " << h << endl;
//In Windoze, we copy the graphics memory to our new context, so we don't have to reload all of our images and stuff every time the resolution changes
#ifdef _WIN32
	SDL_SysWMinfo info;
 
	//Get window handle from SDL
	SDL_VERSION(&info.version);
	if(SDL_GetWindowWMInfo(m_Window, &info) == -1) 
	{
		errlog << "SDL_GetWMInfo #1 failed" << endl;
		return;
	}

	//Get device context handle
	HDC tempDC = GetDC(info.info.win.window);

	//Create temporary context
	HGLRC tempRC = wglCreateContext(tempDC);
	if(tempRC == NULL) 
	{
		errlog << "wglCreateContext failed" << endl;
		return;
	}
	
	//Share resources to temporary context
	SetLastError(0);
	if(!wglShareLists(wglGetCurrentContext(), tempRC))
	{
		errlog << "wglShareLists #1 failed" << endl;
		return;
	}
#endif
	
	m_iWidth = w;
	m_iHeight = h;
	
	if(m_bFullscreen)
		SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	
	SDL_SetWindowSize(m_Window, m_iWidth, m_iHeight);

	SDL_GL_CreateContext(m_Window);
	
	//Set OpenGL back up
	setup_opengl();
	
#ifdef _WIN32
	//Previously used structure may possibly be invalid, to be sure we get it again
	SDL_VERSION(&info.version);
	if(SDL_GetWindowWMInfo(m_Window, &info) == -1) 
	{
		errlog << "SDL_GetWMInfo #2 failed" << endl;
		return;
	}
 
	//Share resources to our new SDL-created context
	if(!wglShareLists(tempRC, wglGetCurrentContext()))
	{
		errlog << "wglShareLists #2 failed" << endl;
		return;
	}
 
	//We no longer need our temporary context
	if(!wglDeleteContext(tempRC))
	{
		errlog << "wglDeleteContext failed" << endl;
		return;
	}
#else
	//TODO: *nix supposedly does this for us automagically. Test.
	//Otherwise, reload images & models
	//reloadImages();
#endif
}

void Engine::toggleFullscreen()
{
  m_bFullscreen = !m_bFullscreen;
  if(m_bFullscreen)
	SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  else
	SDL_SetWindowFullscreen(m_Window, 0);
}

void Engine::setFullscreen(bool bFullscreen)
{
	if(m_bFullscreen == bFullscreen) 
		return;
	toggleFullscreen();
}

Point Engine::getWindowPos()
{
	Point p;
	int x, y;
	SDL_GetWindowPosition(m_Window, &x, &y);
	p.x = x;
	p.y = y;
	return p;
}

void Engine::setWindowPos(Point pos)
{
	SDL_SetWindowPosition(m_Window, pos.x, pos.y);
}

void Engine::maximizeWindow()
{
	SDL_MaximizeWindow(m_Window);
}

bool Engine::isMaximized()
{
#ifdef _WIN32	//Apparently SDL_GetWindowFlags() is completely broken in SDL2 for determining maximization, and nobody else seems to have this problem. Doing it the long way.
	SDL_SysWMinfo info;
 
	//Get window handle from SDL
	SDL_VERSION(&info.version);
	if(SDL_GetWindowWMInfo(m_Window, &info) == -1) 
	{
		errlog << "SDL_GetWMInfo failed" << endl;
		return false;
	}
	
	return IsZoomed(info.info.win.window);
#else
	return (SDL_GetWindowFlags(m_Window) & SDL_WINDOW_MAXIMIZED);	//TODO: Test other OS's to see if this is true for them as well
#endif
}

list<SDL_DisplayMode> Engine::getAvailableResolutions()
{
	//TODO: SDL 2.0 way
	list<SDL_DisplayMode> lResolutions;
	for(int i = 0; i < SDL_GetNumDisplayModes(0); i++)
	{
		SDL_DisplayMode mode;
		if(SDL_GetDisplayMode(0, i, &mode) == 0)
		{
			lResolutions.push_back(mode);
		}
	}
	return lResolutions;  
}

void Engine::setCursorPos(int32_t x, int32_t y)
{
    SDL_WarpMouseInWindow(m_Window, x, y);
//#ifdef __APPLE__
//    hideCursor(); //TODO: Warping the mouse shows it again in Mac, and this doesn't work. Hermph.
//#endif
}

bool Engine::getCursorDown(int iButtonCode)
{
	Uint8 ms = SDL_GetMouseState(NULL, NULL);
	switch(iButtonCode)
	{
		case LMB:
			return(ms & SDL_BUTTON(SDL_BUTTON_LEFT));
		case RMB:
			return(ms & SDL_BUTTON(SDL_BUTTON_RIGHT));
		case MMB:
			return(ms & SDL_BUTTON(SDL_BUTTON_MIDDLE));
		default:
			errlog << "Unsupported mouse code: " << iButtonCode << endl;
			break;
	}
	return false;
}

void Engine::commandline(int argc, char** argv)
{
	//Step through intelligently
	for(int i = 1; i < argc; i++)
	{
		commandlineArg cla;
		string sSwitch = argv[i];
		if(sSwitch.find('-') == 0)
		{
			if(sSwitch.find("--") == 0)
				sSwitch.erase(0,1);
			sSwitch.erase(0,1);
			
			cla.sSwitch = sSwitch;
			if(i+1 < argc)	//Switch with a value
			{
				i++;
				cla.sValue = argv[i];
			}
			
		}
		else	//No switch for this value
			cla.sValue = sSwitch;
		lCommandLine.push_back(cla);
	}
}

void Engine::addObject(obj* o)
{
	m_lObjects.push_back(o);
}

void Engine::drawObjects()
{
	multiset<physSegment*>::iterator layer;
	for(layer = m_lScenery.begin(); layer != m_lScenery.end(); layer++)	//Draw bg layers
	{
		if((*layer)->depth > 0)
			break;
		(*layer)->draw();
	}
	for(list<obj*>::iterator i = m_lObjects.begin(); i != m_lObjects.end(); i++)	//Draw objects
		(*i)->draw();
	for(; layer != m_lScenery.end(); layer++)	//Draw fg layers
		(*layer)->draw();
}

void Engine::cleanupObjects()
{
	for(list<obj*>::iterator i = m_lObjects.begin(); i != m_lObjects.end(); i++)
		delete (*i);
	for(multiset<physSegment*>::iterator i = m_lScenery.begin(); i != m_lScenery.end(); i++)
		delete (*i);
	m_lScenery.clear();
	m_lObjects.clear();
}

void Engine::updateObjects(float32 dt)
{
	list<b2BodyDef*> lAddObj;
	for(list<obj*>::iterator i = m_lObjects.begin(); i != m_lObjects.end(); i++)
	{
		b2BodyDef* def = (*i)->update(dt);
		if(def != NULL)
			lAddObj.push_back(def);
	}
	for(list<b2BodyDef*>::iterator i = lAddObj.begin(); i != lAddObj.end(); i++)
	{
		objFromXML(*((string*)(*i)->userData), (*i)->position, (*i)->linearVelocity);
		delete (*i);	//Free up memory
	}
}








