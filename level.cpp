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
	float fDrawSize = ptWorldSize.Length() * 1.75;	//Actual radius we need to draw is 0.5*this, add on extra to be safe
	
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
		glRotatef(60, 0, 0, 1);
	}
	glPopMatrix();
	
	//Draw triangle for player
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















