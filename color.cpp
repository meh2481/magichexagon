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
		i->colorToChange->r += i->amtr * dt;
		i->colorToChange->g += i->amtg * dt;
		i->colorToChange->b += i->amtb * dt;
		if(myAbs(i->colorToChange->r - i->destr) < COLOR_EPSILON &&
		   myAbs(i->colorToChange->g - i->destg) < COLOR_EPSILON &&
		   myAbs(i->colorToChange->b - i->destb) < COLOR_EPSILON)
		{
			i = m_ColorsChanging.erase(i);
			i--;
		}
	}
	
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
	
void magichexagonEngine::phaseColor(Color* src, Color dest, float time)
{
	ColorPhase cp;
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
	if(m_ColorsChanging.size()) return;
	//Flip across (so RD's one looks better)
	phaseColor(&m_colors[2], m_colors[5], 0.2);
	phaseColor(&m_colors[3], m_colors[6], 0.2);
	phaseColor(&m_colors[4], m_colors[7], 0.2);
	phaseColor(&m_colors[5], m_colors[2], 0.2);
	phaseColor(&m_colors[6], m_colors[3], 0.2);
	phaseColor(&m_colors[7], m_colors[4], 0.2);
}












