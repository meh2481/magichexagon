/*
    magichexagon source - color.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "magichexagon.h"
#include "tinyxml2.h"
#include <float.h>
#include <sstream>

float myAbs(float v)
{
	if(v < 0.0)
		return -v;
	return v;
}

void magichexagonEngine::updateColors(float32 dt)
{
	//Update our colors that are phasing into one another
	for(list<ColorPhase>::iterator i = m_ColorsChanging.begin(); i != m_ColorsChanging.end(); i++)
	{
		if(i->dir)
		{
			if(myAbs(i->colorToChange->r - i->destr) >= myAbs(i->amtr*dt))
				i->colorToChange->r += i->amtr * dt;
			if(myAbs(i->colorToChange->g - i->destg) >= myAbs(i->amtg*dt))
				i->colorToChange->g += i->amtg * dt;
			if(myAbs(i->colorToChange->b - i->destb) >= myAbs(i->amtb*dt))
				i->colorToChange->b += i->amtb * dt;
			if(myAbs(i->colorToChange->r - i->destr) <= myAbs(i->amtr*dt) &&
			   myAbs(i->colorToChange->g - i->destg) <= myAbs(i->amtg*dt) &&
			   myAbs(i->colorToChange->b - i->destb) <= myAbs(i->amtb*dt))
			{
				i->colorToChange->r = i->destr;
				i->colorToChange->g = i->destg;
				i->colorToChange->b = i->destb;
				if(!i->pingpong)
				{
					i = m_ColorsChanging.erase(i);
					i--;
				}
				else
					i->dir = false;
			}
		}
		else
		{
			if(myAbs(i->colorToChange->r - i->srcr) >= myAbs(i->amtr*dt))
				i->colorToChange->r -= i->amtr * dt;
			if(myAbs(i->colorToChange->g - i->srcg) >= myAbs(i->amtg*dt))
				i->colorToChange->g -= i->amtg * dt;
			if(myAbs(i->colorToChange->b - i->srcb) >= myAbs(i->amtb*dt))
				i->colorToChange->b -= i->amtb * dt;
			if(myAbs(i->colorToChange->r - i->srcr) <= myAbs(i->amtr*dt) &&
			   myAbs(i->colorToChange->g - i->srcg) <= myAbs(i->amtg*dt) &&
			   myAbs(i->colorToChange->b - i->srcb) <= myAbs(i->amtb*dt))
			{
				i->colorToChange->r = i->srcr;
				i->colorToChange->g = i->srcg;
				i->colorToChange->b = i->srcb;
				if(!i->pingpong)
				{
					i = m_ColorsChanging.erase(i);
					i--;
				}
				else
					i->dir = true;
			}
		}
	}
	
	if(m_iCurMenu != MENU_LEVELSELECT)
	{
		//Update our background pinwheel colors; flip colors about every second
		static float fElapsedTime = 0.0f;
		static float fTargetTime = 1.0f;
		fElapsedTime += dt;
		if(fElapsedTime > fTargetTime)
		{
			fTargetTime += 1.0f;
			colorFlip();
		}
	}
}
	
void magichexagonEngine::phaseColor(Color* src, Color dest, float time, bool bPingPong)
{
	ColorPhase cp;
	cp.pingpong = bPingPong;
	cp.srcr = src->r;
	cp.srcg = src->g;
	cp.srcb = src->b;
	cp.dir = true;
	cp.colorToChange = src;
	cp.destr = dest.r;
	cp.destg = dest.g;
	cp.destb = dest.b;
	cp.amtr = (dest.r - src->r) / time;
	cp.amtg = (dest.g - src->g) / time;
	cp.amtb = (dest.b - src->b) / time;
	bool bSet = false;
	for(list<ColorPhase>::iterator i = m_ColorsChanging.begin(); i != m_ColorsChanging.end(); i++)
	{
		if(i->colorToChange == src)
		{
			bSet = true;
			*i = cp;
			break;
		}
	}
	if(!bSet)
		m_ColorsChanging.push_back(cp);
}

void magichexagonEngine::colorFlip()
{
	if(m_ColorsChanging.size()) return;	//Don't alternate colors if some of the colors are already changing
	//Flip across
	phaseColor(&m_colors[2], m_colors[5], 0.2);
	phaseColor(&m_colors[3], m_colors[6], 0.2);
	phaseColor(&m_colors[4], m_colors[7], 0.2);
	phaseColor(&m_colors[5], m_colors[2], 0.2);
	phaseColor(&m_colors[6], m_colors[3], 0.2);
	phaseColor(&m_colors[7], m_colors[4], 0.2);
}

void magichexagonEngine::clearColors()
{
	for(list<ColorPhase>::iterator i = m_ColorsChanging.begin(); i != m_ColorsChanging.end(); i++)
	{
		i->colorToChange->r = i->srcr;
		i->colorToChange->g = i->srcg;
		i->colorToChange->b = i->srcb;
	}
	m_ColorsChanging.clear();
}

void magichexagonEngine::highlightLevel()
{
	clearColors();
	switch(m_iCurLevel)
	{
		case 0:
			phaseColor(&m_colors[7], Color(189,102,215), 0.25, true); //Twilight
			phaseColor(&m_colors[0], Twilight, 0.1);
			centerCutie = getImage("res/gfx/twilimark.png");
			break;
		
		case 1:
			phaseColor(&m_colors[6], Color(252,150,0), 0.25, true);	//AJ
			phaseColor(&m_colors[0], AJ, 0.1);
			centerCutie = getImage("res/gfx/ajmark.png");
			break;
		
		case 2:
			phaseColor(&m_colors[5], Color(225,215,110), 0.25, true); //Fluttershy
			phaseColor(&m_colors[0], Fluttershy, 0.1);
			centerCutie = getImage("res/gfx/fluttermark.png");
			break;
			
		case 3:
			if(m_fBestTime[0] >= 60.0)
			{
				phaseColor(&m_colors[4], Color(0,165,247), 0.25, true);	//Dash
				phaseColor(&m_colors[0], Dash, 0.1);
				centerCutie = getImage("res/gfx/dashmark.png");
			}
			else
			{
				phaseColor(&m_colors[0], Color(255,255,255), 0.1);
				centerCutie = NULL;
			}
			break;
		
		case 4:
			if(m_fBestTime[1] >= 60.0)
			{
				phaseColor(&m_colors[3], Color(176,209,231), 0.25, true);	//Rarity
				phaseColor(&m_colors[0], Rarity, 0.1);
				centerCutie = getImage("res/gfx/rarimark.png");
			}
			else
			{
				phaseColor(&m_colors[0], Color(255,255,255), 0.1);
				centerCutie = NULL;
			}
			break;
			
		default:
			if(m_fBestTime[2] >= 60.0)
			{
				phaseColor(&m_colors[2], Color(248,111,159), 0.25, true); //Pinkie
				phaseColor(&m_colors[0], Pinkie, 0.1);
				centerCutie = getImage("res/gfx/pinkiemark.png");
			}
			else
			{
				phaseColor(&m_colors[0], Color(255,255,255), 0.1);
				centerCutie = NULL;
			}
			break;
		
	}
	
}

void magichexagonEngine::setMenuColors()
{
	m_colors[0] = Color(255,255,255);	//Center part
	m_colors[1] = Color(0,0,0);			//Center ring and triangle
	
	if(m_fBestTime[2] >= 60)
		m_colors[2] = Pinkie;				//Radial arm 1
	else
		m_colors[2].set(0.7,0.7,0.7);
	
	if(m_fBestTime[1] >= 60)
		m_colors[3] = Rarity;				//Radial arm 2
	else
		m_colors[3].set(0.9,0.9,0.9);
		
	if(m_fBestTime[0] >= 60)
		m_colors[4] = Dash;					//Radial arm 3
	else
		m_colors[4].set(0.7,0.7,0.7);
	
	m_colors[5] = Fluttershy;			//Radial arm 4
	m_colors[6] = AJ;					//Radial arm 5
	m_colors[7] = Twilight;				//Radial arm 6
	
	highlightLevel();
}








