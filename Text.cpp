/*
    CutsceneEditor source - Text.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "Text.h"

Text::Text(string sXMLFilename)
{
    m_imgFont = NULL;

    //  Load font image and glyphs from xml
    //  File format:
    //
    //  <?xml version="1.0" encoding="UTF-8"?>
    //  <font name="green"
    //      <image path="res/gfx/orig/text_green.png" />
    //      <char ascii="a" rect="0,0,15,9" />
    //      ...
    //  </font>

    XMLDocument* doc = new XMLDocument();
    doc->LoadFile(sXMLFilename.c_str());

    XMLElement* elem = doc->FirstChildElement("font");
    if(elem == NULL) return;
    const char* cName = elem->Attribute("name");
    if(cName != NULL)
        m_sName = cName;
    errlog << "Creating font \"" << m_sName << "\"" << endl;
    for(elem = elem->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {
        cName = elem->Name();
        if(cName == NULL) return;
        string sName(cName);
        if(sName == "image")    //Image
        {
            const char* cPath = elem->Attribute("path");
            if(cPath == NULL) return;
            m_imgFont = new Image(cPath);   //Create image
			m_imgFont->blur = false;

        }
        else if(sName == "char")    //Character
        {
            const char* cChar = elem->Attribute("ascii");
            if(cChar == NULL) return;
            const char* cRect = elem->Attribute("rect");
            if(cRect == NULL) return;
            Rect rc = rectFromString(cRect);
            m_mRectangles[cChar[0]] = rc;   //Stick this into the list
        }
    }
    delete doc;
}

Text::~Text()
{
    errlog << "Destroying font \"" << m_sName << "\"" << endl;
    if(m_imgFont != NULL)
        delete m_imgFont;
}

void Text::render(string sText, float32 x, float32 y, float pt)
{
    if(m_imgFont == NULL)
        return;

	glColor4f(col.r, col.g, col.b, col.a);
	float width = size(sText, pt);
	x += width / 2.0 - (width / sText.size()) / 2.0;
    for(string::iterator i = sText.begin(); i != sText.end(); i++)
    {
        unsigned char c = *i;
        if(c == '\0')
            break;

        map<unsigned char, Rect>::iterator iRect = m_mRectangles.find(c);
        if(iRect == m_mRectangles.end())
            continue;   //Skip over chars we can't draw

        Rect rc = iRect->second;

		glPushMatrix();
        //TODO m_imgFont->draw(x, y, rc);
		glTranslatef(-x, -y, 0.0);
		Point sz(width / sText.size(), pt);
		m_imgFont->render(sz, rc);
		glPopMatrix();
        x -= sz.x;
    }
	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

float32 Text::size(string sText, float pt)
{
	float width = m_imgFont->getWidth();
	width /= m_mRectangles.size();
	width *= pt / m_imgFont->getHeight();
	width *= sText.size();
	return width;
}





