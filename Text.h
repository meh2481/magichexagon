/*
    CutsceneEditor source - Text.h
    Class for ease of drawing bitmapped fonts
    Copyright (c) 2014 Mark Hutcheson
*/
#ifndef TEXT_H
#define TEXT_H

#include "globaldefs.h"
#include "Image.h"
#include <map>

#define ALIGN_LEFT      1
#define ALIGN_RIGHT     2
#define ALIGN_CENTER    4
#define ALIGN_TOP       8
#define ALIGN_MIDDLE    16
#define ALIGN_BOTTOM    32

class Text
{
private:
    Text(){};   //Default constructor cannot be called
    Image* m_imgFont;   //Image for this bitmap font
    map<unsigned char, Rect> m_mRectangles;  //Rectangles for drawing each character
    string m_sName;
    uint8_t m_iAlign;

public:
	Color col;

    Text(string sXMLFilename);  //Create the font from this XML file
    ~Text();

    //Render this text to the screen
    void render(string sText, Point pt);
    void render(string sText, float32 x, float32 y);

    //Find the size of a given string of text
    Point sizeString(string sText);
    void setAlign(uint8_t iAlign);  //Set alignment of the text (align left = to the left of pt, align right = to the right of pt)
    string getName()    {return m_sName;};
    void   setName(string sName)    {m_sName = sName;};

};





















#endif
