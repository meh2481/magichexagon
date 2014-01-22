/*
    magichexagon source - level.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "magichexagon.h"
#include "tinyxml2.h"
#include <float.h>
#include <sstream>

const float s_fPlayerPos = 1.42;
const float s_fCenterWallW = 0.14;

void magichexagonEngine::renderLevel()
{
	if(m_iCurMenu == MENU_NONE)
	{
		HUDItem* time = m_hud->getChild("curtime");
		if(time != NULL)
			bestTime((HUDTextbox*)time, "", m_fTotalSpinTime);
		
		time = m_hud->getChild("bestrtime");
		if(time != NULL)
		{
			if(m_fBestTime[m_iStartLevel] >= 60.0)	//Already beaten this level
			{
				if(m_fTotalSpinTime > m_fBestTime[m_iStartLevel])
					((HUDTextbox*)(time))->setText("new record");
				else
					bestTime((HUDTextbox*)time, "best: ", m_fBestTime[m_iStartLevel]);
			}
			else
			{
				float fNext = 10.0f * (1+floor(m_fTotalSpinTime/10.0));
				if(fNext > 60.0f)
					fNext = 60.0f * (1+floor(m_fTotalSpinTime/60.0));
				bestTime((HUDTextbox*)time, "next: ", fNext);
			}
		}
	}
	
	glPushMatrix();
	//Rotate by how much we're spinning
	glRotatef(m_fRotateAngle, 0, 0, 1);
	
	//Get how large our screenspace is
	Point ptWorldSize(getWidth(), getHeight());
	ptWorldSize = worldMovement(ptWorldSize);	//Get the actual world movement in texels
	float fDrawSize = ptWorldSize.Length() * 1.75;	//Actual radius we _need_ to draw is 0.5*this, add on extra so we can tilt and such
	
	//Draw center hex
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f(m_colors[0].r, m_colors[0].g, m_colors[0].b, m_colors[0].a);
	glTexCoord2f(0.0, 0.0);
	glBegin(GL_POLYGON);
	glVertex3f(-1.0, 0.0, 0.0);
	glVertex3f(-0.5, 0.866, 0.0);
	glVertex3f(0.5, 0.866, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	glVertex3f(0.5, -0.866, 0.0);
	glVertex3f(-0.5, -0.866, 0.0);
	glEnd();
	
	//Draw hollow hex around center one
	glColor4f(m_colors[1].r, m_colors[1].g, m_colors[1].b, m_colors[1].a);
	glPushMatrix();
	glTexCoord2f(0.0, 0.0);
	for(int i = 0; i < 6; i++)
	{
		glBegin(GL_QUADS);
		//left center
		glVertex3f(-1.0, 0.0, 0.0);
		//left
		glVertex3f(-1.0-s_fCenterWallW, 0.0, 0.0);
		//Top left
		glVertex3f(-0.5*(s_fCenterWallW+1.0), 0.866*(s_fCenterWallW+1.0), 0.0);
		//Top left inside
		glVertex3f(-0.5, 0.866, 0.0);
		
		glEnd();
		glRotatef(60, 0, 0, 1);
	}
	glPopMatrix();
	
	//Draw radial arms going outwards at the full size of the screen
	glPushMatrix();
	glTexCoord2f(0.0, 0.0);
	for(int i = 0; i < 6; i++)
	{
		glColor4f(m_colors[2+i].r, m_colors[2+i].g, m_colors[2+i].b, m_colors[2+i].a);
		glBegin(GL_QUADS);
		//left center
		glVertex3f(-1.0-s_fCenterWallW, 0.0, 0.0);
		//left
		glVertex3f(-fDrawSize, -0.01, 0.0);	//A touch down in order to cover up any gaps from rounding errors
		//Top left
		glVertex3f(-0.5*fDrawSize, 0.866*fDrawSize, 0.0);
		//Top left inside
		glVertex3f(-0.5*(s_fCenterWallW+1.0), 0.866*(s_fCenterWallW+1.0), 0.0);
		glEnd();
		
		//Draw walls
		glColor4f(m_colors[1].r, m_colors[1].g, m_colors[1].b, m_colors[1].a);
		for(list<Wall>::iterator j = m_walls[i].begin(); j != m_walls[i].end(); j++)
		{
			glBegin(GL_QUADS);
			//left inside
			glVertex3f(min(-1.0 - j->height, -1.0 - s_fCenterWallW/2.0), -0.01, 0.01);	//Again, a touch down to cover rounding errors
			//left outside
			glVertex3f(min(-1.0 - j->height - j->length, -1.0 - s_fCenterWallW/2.0), -0.01, 0.01);
			//Top left outside
			glVertex3f(0.5*min(-1.0 - j->height - j->length, -1.0 - s_fCenterWallW/2.0), -0.866*min(-1.0 - j->height - j->length, -1.0 - s_fCenterWallW/2.0), 0.01);
			//Top left inside
			glVertex3f(0.5*(min(-1.0 - j->height, -1.0 - s_fCenterWallW/2.0)), -0.866*(min(-1.0 - j->height, -1.0 - s_fCenterWallW/2.0)), 0.01);
			glEnd();
		}
		
		glRotatef(60, 0, 0, 1);
	}
	glPopMatrix();
	
	//Draw triangle for player
	glColor4f(m_colors[1].r, m_colors[1].g, m_colors[1].b, m_colors[1].a);
	glPushMatrix();
	glRotatef(m_fPlayerAngle+90.0, 0, 0, 1);
	glBegin(GL_TRIANGLES);
	//top
	glVertex3f(0, s_fPlayerPos, 0.011);
	//left bottom
	glVertex3f(-0.1462, 1.22, 0.011);
	//right bottom
	glVertex3f(0.1462, 1.22, 0.011);
	glEnd();
	glPopMatrix();
	
	//Draw central cutie mark image thingy
	glPopMatrix();
	if(centerCutie != NULL)
	{
		glColor4f(1.0, 1.0, 1.0, 1.0);
		Point pt(1.5, 1.5);
		centerCutie->render(pt);
	}
}

void magichexagonEngine::addWall(float32 height, float32 speed, float32 length, int32_t hex)
{
	Wall w;
	w.speed = speed;
	w.height = height;
	w.length = length;
	m_walls[hex].push_back(w);
}

int magichexagonEngine::calcPlayerHex(float32* relAngle)
{
	float fAbsPlayerAngle = m_fPlayerAngle+60.0;
	while(fAbsPlayerAngle < 0.0)
		fAbsPlayerAngle += 360.0;
	while(fAbsPlayerAngle >= 360.0)
		fAbsPlayerAngle -= 360.0;
	
	int playerHex = (fAbsPlayerAngle) / 60.0;
	
	//Calculate the player's angle according to their current hex
	float32 plAngle = 60.0 - (fAbsPlayerAngle - playerHex*60.0);
	if(plAngle == 60.0)
	{
		plAngle = 0;
		playerHex--;
		if(playerHex < 0)
			playerHex = 5;
	}
	if(relAngle != NULL)
		*relAngle = plAngle;
	return playerHex;
}

void magichexagonEngine::die()
{
	m_iCurMenu = MENU_GAMEOVER;
	CameraPos.z = m_fDefCameraZ;
	m_hud->setScene("gameover");
	pauseMusic();
	playSound("gameover");
	playSound("die");
	m_iCurLevel = m_iStartLevel;
	//Update our best time for this level
	if(m_fTotalSpinTime > m_fBestTime[m_iStartLevel])
	{
		m_fBestTime[m_iStartLevel] = m_fTotalSpinTime;
		saveConfig(getSaveLocation() + "config.xml");	//Save new best time, so it isn't lost
	}
}

void magichexagonEngine::updateWalls(float32 dt)
{
	float32 plAngle;
	int playerHex = calcPlayerHex(&plAngle);
	
	float32 playerDist = cos(DEG2RAD * plAngle) * s_fPlayerPos;
	float32 playerHeight = sin(DEG2RAD* plAngle) * s_fPlayerPos;
	
	for(int j = 0; j < 6; j++)
	{
		for(list<Wall>::iterator i = m_walls[j].begin(); i != m_walls[j].end(); i++)
		{
			if(m_wTop == NULL)	//Wait until top wall of last level is gone before speeding up
				i->height -= i->speed * dt;
			else
				i->height -= m_wTop->speed * dt;
			if(playerHex == j && i->height + i->length + 1.0 > s_fPlayerPos)	//Player is in this hex, and not above this wall
			{
				float32 wallAngle = atan(playerHeight / (1.0 + i->height - playerDist)) * RAD2DEG;
				if(wallAngle > 60.0f || //If the angle here is greater than 60 degrees, we have a collision
				  //If we're at the very edge of this hex, we can test the height directly.
				  (plAngle <= 5.0 && i->height + 1.0 < s_fPlayerPos && i->height + i->length + 1.0 > s_fPlayerPos) ||
				  (plAngle >= 55.0 && i->height + 1.0 < s_fPlayerPos && i->height + i->length + 1.0 > s_fPlayerPos))		
				{
					die();
					break;
				}
			}
			if(i->height + i->length <= 0)
			{	
				if(&(*i) == m_wTop)
					m_wTop = NULL;
				i = m_walls[j].erase(i);
				i--;
			}
		}
	}
}

void magichexagonEngine::checkSides(float32 fOldAngle, int prevHex, int curHex)
{
	if(calcPlayerHex() != prevHex)
	{
		for(list<Wall>::iterator i = m_walls[curHex].begin(); i != m_walls[curHex].end(); i++)
		{
			//If between top and bottom of wall
			if(i->height + 1.0 < s_fPlayerPos && i->height + 1.0 + i->length > s_fPlayerPos)
			{
				m_fPlayerAngle = fOldAngle;
				break;
			}
		}
	}
}

void magichexagonEngine::updateLevel(float32 dt)
{
	updateWalls(dt);
	
	//Get the maximum height of the tallest wall to see if we need to generate a new pattern
	float32 maxHeight = 0;
	for(int j = 0; j < 6; j++)
	{
		list<Wall>::iterator it = m_walls[j].end();
		if(it != m_walls[j].begin())
		{
			it--;
			if(maxHeight < it->height + it->length)
				maxHeight = it->height + it->length;
			
		}
	}
	
	if(maxHeight + m_gap < 11.0)	//Give brief gap of 4.0 texels between patterns, and more if specified
		nextPattern();

	//Spin!
	m_fTotalSpinTime += dt;
	if(m_fTotalSpinTime > m_fTargetMadSpin)	//MAAD SPIIIN
	{
		if(m_fTotalSpinTime > m_fTargetMadSpin + m_fMadSpinLength)
		{
			m_fTargetMadSpin += m_fMadSpinLength + (m_iCurLevel == 6)?(randFloat(5, 8)):(randFloat(10, 12));
			
			//For "magic" level, we want the screen to be perfectly even when not rotating like mad
			if(m_iCurLevel == 6)
			{
				if(m_fRotateAngle/60.0 - floorf(m_fRotateAngle/60.0) > 0.5)
					m_fRotateAngle = ceilf(m_fRotateAngle/60.0)*60;
				else
					m_fRotateAngle = floorf(m_fRotateAngle/60.0)*60;
			}
		}
		else
		{
			if(m_iCurLevel == LEVEL_MAGIC)	//Spin with our already potentially mad spinning in magic level
				m_fRotateAngle += (m_fRotateAdd > 0)?((-m_fRotateAdd+300)*dt):(-(m_fRotateAdd+300)*dt);
			else							//Spin against our current spin direction in kindness level
				m_fRotateAngle += (m_fRotateAdd > 0)?(-(m_fRotateAdd+300)*dt):((-m_fRotateAdd+300)*dt);
		}
	}
	else if(m_fTotalSpinTime > m_fTargetSpinReverse)
	{
		m_fTargetSpinReverse += randFloat(4, 7);
		m_fRotateAdd = -m_fRotateAdd;
	}
	else if(m_fTotalSpinTime > m_fTargetSpinIncrease && m_fTargetSpinIncrease > 0)
	{
		if(m_fRotateAdd < 0)
			m_fRotateAdd -= 50;
		else
			m_fRotateAdd += 50;
		m_fTargetSpinIncrease = 0;
	}
	m_fRotateAngle += m_fRotateAdd * dt;
	
	//check and see if level should be changed
	checkLevel();
}

void magichexagonEngine::checkLevel()
{
	//Play "excellent" vox if passing previous record
	if(m_fTotalSpinTime > m_fBestTime[m_iStartLevel] && m_fBestTime[m_iStartLevel] > 0 && !m_bPlayedExcellent)
	{
		playSound("excellent");
		m_bPlayedExcellent = true;
	}
	
	//Within a level, play vox for "honesty" - "laughter"
	for(int i = 1; i < 6; i++)
	{
		float time = 10.0 * i;
		if(m_fTotalSpinTime > time && m_fLastChecked < time)
		{
			switch(i)
			{
				case LEVEL_HONESTY:
					playSound("honesty");
					break;
					
				case LEVEL_KINDNESS:
					playSound("kindness");
					break;
					
				case LEVEL_LOYALTY:
					playSound("loyalty");
					break;
										
				case LEVEL_GENEROSITY:
					playSound("generosity");
					break;
										
				case LEVEL_LAUGHTER:
					playSound("laughter");
					break;
			}
			break;
		}
	}
	
	//Every time another level is cleared, play "magic" - "awesome"
	for(int i = 1; ; i++)
	{
		float time = 60.0 * i;
		if(m_fTotalSpinTime < time)
			break;
		if(m_fTotalSpinTime > time && m_fLastChecked < time)
		{
			//Play the proper congratulatory vox
			switch(i)
			{
				case 1:
					playSound("magic");
					break;
					
				case 2:
					playSound("nice");
					break;
					
				case 3:
					playSound("wonderful");
					break;
					
				default:
					playSound("awesome");
					break;
					
			}
			
			//Load next level
			switch(m_iCurLevel)
			{
				case LEVEL_FRIENDSHIP:
					changeLevel(LEVEL_LOYALTY);
					break;
				
				case LEVEL_HONESTY:
					changeLevel(LEVEL_GENEROSITY);
					break;
				
				case LEVEL_KINDNESS:
					changeLevel(LEVEL_LAUGHTER);
					break;
				
				case LEVEL_LOYALTY:
					if(m_fWallSpeed < 8.1)	//Go into nuts mode
					{
						m_fRotateAdd = 200;
						m_fWallSpeed = 8.25;
						m_gap = 5.0;
						//Reverse color
						phaseColor(&m_colors[0], Color(98,38,8), 0.5);
						phaseColor(&m_colors[1], Color(255,255,255), 0.5);
						phaseColor(&m_colors[2], Color(16,191,202), 0.5);
						phaseColor(&m_colors[3], Color(12,136,201), 0.5);
						phaseColor(&m_colors[4], Color(0,8,104), 0.5);
						phaseColor(&m_colors[5], Color(133,62,189), 0.5);
						phaseColor(&m_colors[6], Color(255,108,47), 0.5);
						phaseColor(&m_colors[7], Color(146,224,129), 0.5);
					}
					else
						changeLevel(LEVEL_LAUGHTER);
					break;
				
				case LEVEL_GENEROSITY:
					if(m_fWallSpeed < 8.6)	//Go into nuts mode
					{
						m_fRotateAdd = 225;
						m_fWallSpeed = 9;
						m_gap = 5.0;
						//Nightmare Rarity mode
						phaseColor(&m_colors[0], Color(72,181,214), 0.5);
						phaseColor(&m_colors[1], Color(58,49,57), 0.5);
						phaseColor(&m_colors[2], Color(120,106,168), 0.5);
						phaseColor(&m_colors[3], Color(72,181,214), 0.5);
						phaseColor(&m_colors[4], Color(120,106,168), 0.5);
						phaseColor(&m_colors[5], Color(72,181,214), 0.5);
						phaseColor(&m_colors[6], Color(120,106,168), 0.5);
						phaseColor(&m_colors[7], Color(72,181,214), 0.5);
					}
					else
						changeLevel(LEVEL_LAUGHTER);
					break;
				
				case LEVEL_LAUGHTER:
					changeLevel(LEVEL_MAGIC);
					break;
				
				case LEVEL_MAGIC:	//Go absolutely nuts if they survive here for 60+ seconds
					//It's sort of entirely broken with the mad spinning, but you're basically dead anyway, so you probably won't notice
					if(m_fRotateAdd > 0) 
						m_fRotateAdd += 300;
					else
						m_fRotateAdd -= 300;
					break;
					
			}
		}
	}
		
		
	m_fLastChecked = m_fTotalSpinTime;
}

void magichexagonEngine::nextPattern()
{
	m_gap = 0;
	int iCurLevel = m_iCurLevel;
	
	if(m_Patterns.size() <= m_iCurLevel || !m_Patterns[iCurLevel].size())	//Check for empty level
		iCurLevel = 0;
	
	int iPattern = rand() % m_Patterns[iCurLevel].size();	//Pick a random pattern out of those available
	int startHex = rand() % 6;	//Start pattern at a random hex value (so it'll point in a random direction)
	
	for(list<pattern>::iterator i = m_Patterns[iCurLevel][iPattern].begin(); i != m_Patterns[iCurLevel][iPattern].end(); i++)
	{
		int hex = i->hex;
		if(hex < 0 || hex > 5)
			hex = rand() % 6;	//Set to random hex if out of range (can use this to set random patterns)
		if(hex + startHex > 5)
			hex -= 6;
		addWall(i->height + m_fWallStartHeight, m_fWallSpeed, i->length, hex + startHex);	//Add this wall to our list
	}
}

bool magichexagonEngine::loadPatterns(string sFilename)
{
	errlog << "Parsing pattern file " << sFilename << endl;
	//Open file
	XMLDocument* doc = new XMLDocument;
	int iErr = doc->LoadFile(sFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		errlog << "Error parsing pattern file: Error " << iErr << endl;
		delete doc;
		return false;
	}
	
	//Grab root element
	XMLElement* root = doc->RootElement();
	if(root == NULL)
	{
		errlog << "Error: Root element NULL in XML file " << sFilename << endl;
		delete doc;
		return false;
	}
	
	for(XMLElement* level = root->FirstChildElement("level"); level != NULL; level = level->NextSiblingElement("level"))
	{
		vector<list<pattern> > vl_Level;
		
		for(XMLElement* pat = level->FirstChildElement("pattern"); pat != NULL; pat = pat->NextSiblingElement("pattern"))
		{
			list<pattern> lWalls;
			
			for(XMLElement* wall = pat->FirstChildElement("wall"); wall != NULL; wall = wall->NextSiblingElement("wall"))
			{
				pattern p;
				p.hex = 0;
				p.height = p.length = 0.0;
				wall->QueryIntAttribute("hex", &p.hex);
				wall->QueryFloatAttribute("startpos", &p.height);
				wall->QueryFloatAttribute("height", &p.length);
				lWalls.push_back(p);
			}
			
			vl_Level.push_back(lWalls);
		}
		
		m_Patterns.push_back(vl_Level);
	}
	
	delete doc;
	return true;
}

Wall* magichexagonEngine::top()
{
	Wall* wret = NULL;
	float top = 0;
	for(int i = 0; i < 6; i++)
	{
		list<Wall>::iterator it = m_walls[i].begin();
		if(it != m_walls[i].end())
		{
			Wall* w = &(*it);
			if(w != NULL)
			{
				if(w->height + w->length > top)
				{
					top = w->height + w->length;
					wret = w;
				}
			}
		}
	}
	return wret;
}

void magichexagonEngine::resetLevel()
{
	m_ColorsChanging.clear();
	m_fRotateAngle = 0.0;
	m_fPlayerAngle = -92.5f;
	m_fTotalSpinTime = 0.0f;
	m_fTargetSpinTime = LEVELTIME;
	m_fWallStartHeight = 15.0;
	m_wTop = NULL;
	m_gap = 0;
	m_iTargetSpinLevel = LEVEL_HONESTY;
	m_fLastChecked = 0;
	m_bPlayedExcellent = false;
	for(int i = 0; i < 6; i++)
	{
		if(m_walls[i].size())
			m_walls[i].clear();
	}
	
	changeLevel(m_iCurLevel);
}

void magichexagonEngine::changeLevel(int iNewLevel)
{	
	//Mark topmost wall
	if(iNewLevel != m_iCurLevel)
		m_wTop = top();
	m_gap = 5.0;	//Gap of 5 texels until next pattern generates
	m_fMadSpinLength = 0;
	m_fTargetMadSpin = FLT_MAX;
	CameraPos.z = m_fDefCameraZ;
	m_iCurLevel = iNewLevel;
	if(iNewLevel == LEVEL_MAGIC)
	{
		phaseColor(&m_colors[0], Twilight, 0.5);
		phaseColor(&m_colors[1], Twilight, 0.5);
		phaseColor(&m_colors[2], TwilightMane, 0.5);
		phaseColor(&m_colors[3], TwilightMane, 0.5);
		phaseColor(&m_colors[4], TwilightMane, 0.5);
		phaseColor(&m_colors[5], TwilightMane, 0.5);
		phaseColor(&m_colors[6], TwilightMane, 0.5);
		phaseColor(&m_colors[7], TwilightMane, 0.5);
		centerCutie = getImage("res/gfx/twilimark.png");
		m_fRotateAdd = m_fRotateAngle = 0;
		m_fTargetSpinReverse = FLT_MAX;
		m_fTargetSpinIncrease = FLT_MAX;
		m_fWallSpeed = 10;
		m_fPlayerMove = 10;
		CameraPos.z -= 2;
		m_fMadSpinLength = 2.0;
		m_fTargetMadSpin = m_fTotalSpinTime + randFloat(5, 8);
	}
	else if(iNewLevel == LEVEL_LAUGHTER)
	{
		phaseColor(&m_colors[0], Pinkie, 0.5);
		phaseColor(&m_colors[1], PinkieMane, 0.5);
		phaseColor(&m_colors[2], PinkieEyes, 0.5);
		phaseColor(&m_colors[3], Pinkie, 0.5);
		phaseColor(&m_colors[4], PinkieEyes, 0.5);
		phaseColor(&m_colors[5], Pinkie, 0.5);
		phaseColor(&m_colors[6], PinkieEyes, 0.5);
		phaseColor(&m_colors[7], Pinkie, 0.5);
		centerCutie = getImage("res/gfx/pinkiemark.png");
		m_fRotateAdd = 175;
		m_fWallSpeed = 9.5;
		m_fPlayerMove = 9.5;
		m_fTargetSpinReverse = m_fTotalSpinTime + randFloat(4,7);
		m_fTargetSpinIncrease = m_fTotalSpinTime + randFloat(12, 15);
	}
	else if(iNewLevel == LEVEL_GENEROSITY)
	{
		phaseColor(&m_colors[0], RarityEyes, 0.5);
		phaseColor(&m_colors[1], Rarity, 0.5);
		phaseColor(&m_colors[2], RarityMane, 0.5);
		phaseColor(&m_colors[3], RarityEyes, 0.5);
		phaseColor(&m_colors[4], RarityMane, 0.5);
		phaseColor(&m_colors[5], RarityEyes, 0.5);
		phaseColor(&m_colors[6], RarityMane, 0.5);
		phaseColor(&m_colors[7], RarityEyes, 0.5);
		centerCutie = getImage("res/gfx/rarimark.png");
		m_fRotateAdd = 150;
		m_fWallSpeed = 8.5;
		m_fPlayerMove = 8.5;
		m_fTargetSpinReverse = m_fTotalSpinTime + randFloat(4,7);
		m_fTargetSpinIncrease = m_fTotalSpinTime + randFloat(12, 15);
	}
	else if(iNewLevel == LEVEL_LOYALTY)
	{
		phaseColor(&m_colors[0], Dash, 0.5);
		phaseColor(&m_colors[1], Color(0,0,0), 0.5);
		phaseColor(&m_colors[2], DashManeR, 0.5);
		phaseColor(&m_colors[3], DashManeO, 0.5);
		phaseColor(&m_colors[4], DashManeY, 0.5);
		phaseColor(&m_colors[5], DashManeG, 0.5);
		phaseColor(&m_colors[6], DashManeB, 0.5);
		phaseColor(&m_colors[7], DashManeV, 0.5);
		centerCutie = getImage("res/gfx/dashmark.png");
		m_fRotateAdd = 125;
		m_fWallSpeed = 8;
		m_fPlayerMove = 8.0;
		m_fTargetSpinReverse = m_fTotalSpinTime + randFloat(4,7);
		m_fTargetSpinIncrease = m_fTotalSpinTime + randFloat(12, 15);
	}
	else if(iNewLevel == LEVEL_KINDNESS)
	{
		phaseColor(&m_colors[0], Fluttershy, 0.5);
		phaseColor(&m_colors[1], FluttershyEyes, 0.5);
		phaseColor(&m_colors[2], FluttershyMane, 0.5);
		phaseColor(&m_colors[3], Fluttershy, 0.5);
		phaseColor(&m_colors[4], FluttershyMane, 0.5);
		phaseColor(&m_colors[5], Fluttershy, 0.5);
		phaseColor(&m_colors[6], FluttershyMane, 0.5);
		phaseColor(&m_colors[7], Fluttershy, 0.5);
		centerCutie = getImage("res/gfx/fluttermark.png");
		m_fRotateAdd = 100;
		m_fWallSpeed = 7;
		m_fPlayerMove = 8.0;
		m_fTargetSpinReverse = m_fTotalSpinTime + randFloat(4,7);
		m_fTargetSpinIncrease = m_fTotalSpinTime + randFloat(12, 15);
		m_fMadSpinLength = 1.0;
		m_fTargetMadSpin = m_fTotalSpinTime + randFloat(9, 11);
	}
	else if(iNewLevel == LEVEL_HONESTY)
	{
		phaseColor(&m_colors[0], AJ, 0.5);
		phaseColor(&m_colors[1], AJEyes, 0.5);
		phaseColor(&m_colors[2], AJMane, 0.5);
		phaseColor(&m_colors[3], AJ, 0.5);
		phaseColor(&m_colors[4], AJMane, 0.5);
		phaseColor(&m_colors[5], AJ, 0.5);
		phaseColor(&m_colors[6], AJMane, 0.5);
		phaseColor(&m_colors[7], AJ, 0.5);
		centerCutie = getImage("res/gfx/ajmark.png");
		m_fRotateAdd = 75;
		m_fWallSpeed = 5.0;
		m_fPlayerMove = 7.0;
		m_fTargetSpinReverse = m_fTotalSpinTime + randFloat(4,7);
		m_fTargetSpinIncrease = m_fTotalSpinTime + randFloat(12, 15);
	}
	else if(iNewLevel == LEVEL_FRIENDSHIP)
	{
		m_colors[0] = Color(255,255,255);	//Center part
		m_colors[1] = Color(0,0,0);			//Center ring and triangle
		m_colors[2] = Dash;					//Radial arm 1
		m_colors[3] = Fluttershy;			//Radial arm 2
		m_colors[4] = Twilight;				//Radial arm 3
		m_colors[5] = Rarity;				//Radial arm 4
		m_colors[6] = Pinkie;				//Radial arm 5
		m_colors[7] = AJ;					//Radial arm 6
		centerCutie = NULL;
		m_fRotateAdd = 25;
		m_fWallSpeed = 3.5;
		m_fPlayerMove = 5.0;
		m_fTargetSpinReverse = m_fTotalSpinTime + randFloat(4,7);
		m_fTargetSpinIncrease = m_fTotalSpinTime + randFloat(12, 15);
	}
	else
		errlog << "Unknown level: " << iNewLevel << endl;
	
	if(rand() % 2 == 0)	//Start rotating in a random direction
		m_fRotateAdd = -m_fRotateAdd;
}














