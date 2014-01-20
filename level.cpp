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
		glVertex3f(-fDrawSize, 0.0, 0.0);
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
			glVertex3f(min(-1.0 - j->height, -1.0 - s_fCenterWallW/2.0), 0.0, 0.01);
			//left outside
			glVertex3f(min(-1.0 - j->height - j->length, -1.0 - s_fCenterWallW/2.0), 0.0, 0.01);
			//Top left outside
			glVertex3f(0.5*min(-1.0 - j->height - j->length, -1.0 - s_fCenterWallW/2.0), -0.866*min(-1.0 - j->height - j->length, -1.0 - s_fCenterWallW/2.0), 0.01);
			//Top left inside
			glVertex3f(0.5*(min(-1.0 - j->height, -1.0 - s_fCenterWallW/2.0)), -0.866*(min(-1.0 - j->height, -1.0 - s_fCenterWallW/2.0)), 0.01);
			glEnd();
		}
		
		glRotatef(60, 0, 0, 1);
	}
	glPopMatrix();
	
	//Draw triangle for player	//TODO: Rotate as player moves
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
			i->height -= i->speed * dt;
			if(playerHex == j && i->height + i->length + 1.0 > s_fPlayerPos)	//Player is in this hex, and not above this wall
			{
				float32 wallAngle = atan(playerHeight / (1.0 + i->height - playerDist)) * RAD2DEG;
				if(wallAngle > 60.0f || //If the angle here is greater than 60 degrees, we have a collision
				  //If we're at the very edge of this hex, we can test the height directly.
				  (plAngle == 0.0 && i->height + 1.0 < s_fPlayerPos && i->height + i->length + 1.0 > s_fPlayerPos))		
				{
					m_iCurMenu = MENU_LEVELSELECT;
					pauseMusic();
					playSound("gameover");
					playSound("die");
					break;
				}
			}
			if(i->height + i->length <= 0)
			{	
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
		for(list<Wall>::iterator i = m_walls[j].begin(); i != m_walls[j].end(); i++)
		{
			if(maxHeight < i->height + i->length)
				maxHeight = i->height + i->length;
		}
	}
	
	if(maxHeight < 11.0)	//Give brief gap of 4.0 texels between patterns
		nextPattern();

	//Spin!
	m_fTotalSpinTime += dt;
	if(m_fTotalSpinTime > m_fTargetSpinReverse)
	{
		m_fTargetSpinReverse += randFloat(4, 7);
		m_fRotateAdd = -m_fRotateAdd;
	}
	if(m_fTotalSpinTime > m_fTargetSpinIncrease && m_fTargetSpinIncrease > 0)
	{
		if(m_fRotateAdd < 0)
			m_fRotateAdd -= 50;
		else
			m_fRotateAdd += 50;
		m_fTargetSpinIncrease = 0;
	}
	m_fRotateAngle += m_fRotateAdd * dt;
	
	//Update timer!
	if(m_fTotalSpinTime > m_fTargetSpinTime)
	{
		if(m_fTotalSpinTime < LEVEL_MAGIC)
			m_fTargetSpinTime += LEVELTIME;
		else
			m_fTargetSpinTime = FLT_MAX;
		changeLevel(m_fTotalSpinTime);
	}
}

void magichexagonEngine::nextPattern()
{
	int iCurLevel = m_iCurLevel;
	if(m_Patterns.size() <= m_iCurLevel)
	{
		cout << "Empty level" << endl;
		iCurLevel = 0;
	}
	
	int iPattern = rand() % m_Patterns[iCurLevel].size();	//Pick a random pattern out of those available
	int startHex = rand() % 6;	//Start pattern at a random hex value (so it'll point in a random direction)
	
	for(list<pattern>::iterator i = m_Patterns[iCurLevel][iPattern].begin(); i != m_Patterns[iCurLevel][iPattern].end(); i++)
	{
		int hex = i->hex;
		if(hex < 0 || hex > 5)
			hex = rand() % 6;	//Set to random hex if out of range (can use this to set random patterns)
		if(hex + startHex > 5)
			hex -= 6;
		addWall(i->height + WALL_START_HEIGHT, m_fWallSpeed, i->length, hex + startHex);	//Add this wall to our list
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

void magichexagonEngine::resetLevel()
{
	for(int j = 0; j < 6; j++)
		m_walls[j].clear();
	m_ColorsChanging.clear();
	m_fRotateAngle = 0.0;
	m_fRotateAdd = 25;
	if(rand() % 2 == 0)	//Start rotating in a random direction
		m_fRotateAdd = -m_fRotateAdd;
	m_colors[0] = Color(255,255,255);	//Center part
	m_colors[1] = Color(0,0,0);			//Center ring and triangle
	m_colors[2] = Dash;					//Radial arm 1
	m_colors[3] = Fluttershy;			//Radial arm 2
	m_colors[4] = Twilight;				//Radial arm 3
	m_colors[5] = Rarity;				//Radial arm 4
	m_colors[6] = Pinkie;				//Radial arm 5
	m_colors[7] = AJ;					//Radial arm 6
	centerCutie = NULL;
	m_fPlayerAngle = -92.5f;
	m_fTotalSpinTime = 0.0f;
	m_fTargetSpinReverse = randFloat(4,7);
	m_fTargetSpinTime = LEVELTIME;
	m_fTargetSpinIncrease = randFloat(12, 15);
	m_iCurLevel = 0;
	m_fWallSpeed = 3.5;
	m_fPlayerMove = 5.0;
}

void magichexagonEngine::changeLevel(float32 time)
{
	if(time >= LEVEL_MAGIC)
	{
		playSound("magic");
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
		m_iCurLevel = 6;
		for(int i = 0; i < 6; i++)
			m_walls[i].clear();
	}
	else if(time >= LEVEL_LAUGHTER)
	{
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
		m_iCurLevel = 5;
		for(int i = 0; i < 6; i++)
			m_walls[i].clear();
	}
	else if(time >= LEVEL_GENEROSITY)
	{
		playSound("generosity");
		phaseColor(&m_colors[0], RarityEyes, 0.5);
		phaseColor(&m_colors[1], Rarity, 0.5);
		phaseColor(&m_colors[2], RarityMane, 0.5);
		phaseColor(&m_colors[3], RarityEyes, 0.5);
		phaseColor(&m_colors[4], RarityMane, 0.5);
		phaseColor(&m_colors[5], RarityEyes, 0.5);
		phaseColor(&m_colors[6], RarityMane, 0.5);
		phaseColor(&m_colors[7], RarityEyes, 0.5);
		centerCutie = getImage("res/gfx/rarimark.png");
		m_iCurLevel = 4;
		for(int i = 0; i < 6; i++)
			m_walls[i].clear();
	}
	else if(time >= LEVEL_LOYALTY)
	{
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
		m_iCurLevel = 3;
		for(int i = 0; i < 6; i++)
			m_walls[i].clear();
	}
	else if(time >= LEVEL_KINDNESS)
	{
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
		m_iCurLevel = 2;
		for(int i = 0; i < 6; i++)
			m_walls[i].clear();
	}
	else if(time >= LEVEL_HONESTY)
	{
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
		m_iCurLevel = 1;
		m_fRotateAdd = 75;
		m_fWallSpeed = 5.0;
		m_fPlayerMove = 7.0;
		m_fTargetSpinReverse = randFloat(4,7);
		m_fTargetSpinIncrease = randFloat(12, 15);
		for(int i = 0; i < 6; i++)
			m_walls[i].clear();
	}
	else
		errlog << "Unknown level-change time: " << time << endl;
	
}














