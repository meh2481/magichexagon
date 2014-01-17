/*
 CutsceneEditor source - Image.cpp
 Copyright (c) 2014 Mark Hutcheson
*/

#include "Image.h"
#include <set>

Image::Image(string sFilename)
{
  //m_ptHotSpot.SetZero();
  m_sFilename = sFilename;
  _load(sFilename);
  _addImgReload(this);
}

int power_of_two(unsigned int val);

void Image::_load(string sFilename)
{
	errlog << "Load " << sFilename << endl;
	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	unsigned int width(0), height(0);
	
	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(sFilename.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(sFilename.c_str());
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN)
	{
		errlog << "Unknown image type for file " << sFilename << endl;
		return;
	}
  
	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, sFilename.c_str());
	else
		errlog << "File " << sFilename << " doesn't support reading." << endl;
	//if the image failed to load, return failure
	if(!dib)
	{
		errlog << "Error loading image " << sFilename.c_str() << endl;
		return;
	}  
	//retrieve the image data
  
	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
 
	int mode, modeflip;
	if(FreeImage_GetBPP(dib) == 24) // RGB 24bit
	{
		mode = GL_RGB;
		modeflip = GL_BGR;
	}
	else if(FreeImage_GetBPP(dib) == 32)  // RGBA 32bit
	{
		mode = GL_RGBA;
		modeflip = GL_BGRA;
	}
  
	bits = FreeImage_GetBits(dib);	//if this somehow one of these failed (they shouldn't), return failure
	if((bits == 0) || (width == 0) || (height == 0))
	{
		errlog << "Something went terribly horribly wrong with getting image bits; just sit and wait for the singularity" << endl;
		return;
	}
  
	//generate an OpenGL texture ID for this texture
	m_iWidth = width;
	m_iHeight = height;
	glGenTextures(1, &m_hTex);
	//bind to the new texture ID
	glBindTexture(GL_TEXTURE_2D, m_hTex);
	//store the texture data for OpenGL use
	glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, modeflip, GL_UNSIGNED_BYTE, bits);
  
	//Free FreeImage's copy of the data
	FreeImage_Unload(dib);
}

Image::~Image()
{
    //image cleanup
    errlog << "Freeing image \"" << m_sFilename << "\"" << endl;
	if(m_hTex)
		glDeleteTextures(1, &m_hTex);	//Free OpenGL graphics memory
    _removeImgReload(this);
}

  /* TODO: Intelligent drawing
  
  <fgenesis> i recommend using glViewport and related functions so you don't have to scale stuff into [-1 .. 1] anymore
<fgenesis> let your gfx card do the heavy lifting, not the CPU
<fgenesis> also have a look at glOrtho() and glMatrixMode(), you'll need those
*/

void Image::render(Point size)
{
	render(size, Point(0,0));
}

// Shear is straightforward. To shear left/right, simply subtract from the x texel coordinates for the top part of the image,
// and add to the x texel coordinates for the bottom part of the image. Similarly, to shear up/down, add to the left side
// (move left side up) and subtract from the right side (move right side down) by the same amount. 
void Image::render(Point size, Point shear)
{
	// tell opengl to use the generated texture
	glBindTexture(GL_TEXTURE_2D, m_hTex);
  
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	//if(you want things blurry)
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	//else //If you want things pixellated
	//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  
	// make a rectangle
	glBegin(GL_QUADS);
	// top left
	glTexCoord2f(0.0, 1.0);	//Our real image is in the upper-left corner of memory, flipped vertically. Compensate.
	glVertex3f(-size.x/2.0 - shear.x, size.y/2.0 + shear.y, 0.0);
	// bottom left
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-size.x/2.0 + shear.x, -size.y/2.0 + shear.y, 0.0);
	// bottom right
	glTexCoord2f(1.0, 0.0);
	glVertex3f(size.x/2.0 + shear.x, -size.y/2.0 - shear.y, 0.0);
	// top right
	glTexCoord2f(1.0, 1.0);
	glVertex3f(size.x/2.0 - shear.x, size.y/2.0 - shear.y, 0.0);
	
	
	
	//glTexCoord2f(0.0f, 0.0f);
	//glTexCoord2f(0.0f, (float32)(m_iHeight)/(float32)(m_iRealHeight));	//If the full width of the image isn't a power of 2, compensate
	//glTexCoord2f((float32)(m_iWidth)/(float32)(m_iRealWidth), (float32)(m_iHeight)/(float32)(m_iRealHeight));
	//glTexCoord2f((float32)(m_iWidth)/(float32)(m_iRealWidth), 0.0f);

	glEnd();
  
}

void Image::_reload()
{
  _load(m_sFilename);
}

static set<Image*> sg_images;

void reloadImages()
{
  for(set<Image*>::iterator i = sg_images.begin(); i != sg_images.end(); i++)
  {
    (*i)->_reload();
  }
}

void _addImgReload(Image* img)
{
  sg_images.insert(img);
}

void _removeImgReload(Image* img)
{
  sg_images.erase(img);
}

static map<string, Image*> g_mImages;  //Image handler
Image* getImage(string sFilename)
{
	if(sFilename == "image_none") return NULL;
	
    map<string, Image*>::iterator i = g_mImages.find(sFilename);
    if(i == g_mImages.end())   //This image isn't here; load it
    {
        Image* img = new Image(sFilename);   //Create this image
        g_mImages[sFilename] = img; //Add to the map
        return img;
    }
    return i->second; //Return this image
}

void clearImages()
{
    for(map<string, Image*>::iterator i = g_mImages.begin(); i != g_mImages.end(); i++)
        delete (i->second);    //Delete each image
    g_mImages.clear();
}



