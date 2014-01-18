/*
    magichexagon source - level.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "magichexagon.h"
#include "tinyxml2.h"
#include <float.h>
#include <sstream>

void magichexagonEngine::renderLevel()
{
	//Rotate by how much we're spinning
	glRotatef(m_fRotateAngle, 0, 0, 1);
	
	//Get how large our screenspace is
	Point ptWorldSize(getWidth(), getHeight());
	ptWorldSize = worldMovement(ptWorldSize);	//Get the actual world movement in texels
	float fDrawSize = ptWorldSize.Length() * 1.75;	//Actual radius we _need_ to draw is 0.5*this, add on extra so we can tilt and such
	
	//Draw center hex	//TODO: Draw as polygon, so no rounding errors
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
	
	static float s_fWallW = 0.14;
	
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
		glVertex3f(-1.0-s_fWallW, 0.0, 0.0);
		//Top left
		glVertex3f(-0.5*(s_fWallW+1.0), 0.866*(s_fWallW+1.0), 0.0);
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
		glVertex3f(-1.0-s_fWallW, 0.0, 0.0);
		//left
		glVertex3f(-fDrawSize, 0.0, 0.0);
		//Top left
		glVertex3f(-0.5*fDrawSize, 0.866*fDrawSize, 0.0);
		//Top left inside
		glVertex3f(-0.5*(s_fWallW+1.0), 0.866*(s_fWallW+1.0), 0.0);
		
		glEnd();
		
		//Draw walls
		glColor4f(m_colors[1].r, m_colors[1].g, m_colors[1].b, m_colors[1].a);
		for(list<Wall>::iterator j = m_walls[i].begin(); j != m_walls[i].end(); j++)
		{
			glBegin(GL_QUADS);
			//left inside
			glVertex3f(min(-1.0 - j->height, -1.0 - s_fWallW/2.0), 0.0, 0.0);
			//left outside
			glVertex3f(min(-1.0 - j->height - j->length, -1.0 - s_fWallW/2.0), 0.0, 0.0);
			//Top left outside
			glVertex3f(0.5*min(-1.0 - j->height - j->length, -1.0 - s_fWallW/2.0), -0.866*min(-1.0 - j->height - j->length, -1.0 - s_fWallW/2.0), 0.0);
			//Top left inside
			glVertex3f(0.5*(min(-1.0 - j->height, -1.0 - s_fWallW/2.0)), -0.866*(min(-1.0 - j->height, -1.0 - s_fWallW/2.0)), 0.0);
			glEnd();
		}
		
		glRotatef(60, 0, 0, 1);
	}
	glPopMatrix();
	
	//Draw triangle for player	//TODO: Rotate as player moves
	static float s_fPlayerPos = 1.42;
	glColor4f(m_colors[1].r, m_colors[1].g, m_colors[1].b, m_colors[1].a);
	glPushMatrix();
	glRotatef(m_fPlayerAngle, 0, 0, 1);
	glBegin(GL_TRIANGLES);
	//top
	glVertex3f(0, s_fPlayerPos, 0.01);
	//left bottom
	glVertex3f(-0.1462, 1.22, 0.01);
	//right bottom
	glVertex3f(0.1462, 1.22, 0.01);
	glEnd();
	glPopMatrix();
	
	//TODO: Draw walls
	
}

void magichexagonEngine::addWall(float32 height, float32 speed, float32 length, int32_t hex)
{
	Wall w;
	w.speed = speed;
	w.height = height;
	w.length = length;
	m_walls[hex].push_back(w);
}

void magichexagonEngine::updateWalls(float32 dt)
{
	for(int j = 0; j < 6; j++)
	{
		for(list<Wall>::iterator i = m_walls[j].begin(); i != m_walls[j].end(); i++)
		{
			i->height -= i->speed * dt;
			if(i->height + i->length <= 0)
			{	
				i = m_walls[j].erase(i);
				i--;
			}
		}
	}
}













