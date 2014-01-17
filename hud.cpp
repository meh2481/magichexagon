/*
 CutsceneEditor source - hud.cpp
 Copyright (c) 2014 Mark Hutcheson
*/

#include "hud.h"
#include <sstream>

extern int screenDrawWidth;
extern int screenDrawHeight;

//-------------------------------------------------------------------------------------
// HUDItem class functions
//-------------------------------------------------------------------------------------
HUDItem::HUDItem(string sName)
{
    m_ptPos.SetZero();
    m_sName = sName;
    m_signalHandler = stubSignal;
    m_iSCALE_FAC = 1;
}

HUDItem::~HUDItem()
{
    for(list<HUDItem*>::iterator i = m_lChildren.begin(); i != m_lChildren.end(); i++)
        delete (*i);    //Clean up all children also
}

void HUDItem::event(SDL_Event event)
{
    //Base class does nothing with this, except pass on
    for(list<HUDItem*>::iterator i = m_lChildren.begin(); i != m_lChildren.end(); i++)
        (*i)->event(event);
}

void HUDItem::draw(float32 fCurTime)
{
    //Base class does nothing with this, except pass on
    for(list<HUDItem*>::iterator i = m_lChildren.begin(); i != m_lChildren.end(); i++)
        (*i)->draw(fCurTime);
}

void HUDItem::setSignalHandler(void (*signalHandler)(string))
{
    m_signalHandler = signalHandler;

    //Make all children have this signal handler as well
    for(list<HUDItem*>::iterator i = m_lChildren.begin(); i != m_lChildren.end(); i++)
        (*i)->setSignalHandler(signalHandler);
}

void HUDItem::addChild(HUDItem* hiChild)
{
    if(hiChild == NULL)
        return;
    //hiChild->setSignalHandler(m_signalHandler);	//TODO: Why not?
    m_lChildren.push_back(hiChild);
}

HUDItem* HUDItem::getChild(string sName)
{
    if(m_sName == sName)    //Base case 1: We're this child they're looking for
        return this;

    for(list<HUDItem*>::iterator i = m_lChildren.begin(); i != m_lChildren.end(); i++)
    {
        HUDItem* hi = (*i)->getChild(sName);   //Recursive call to children
        if(hi != NULL)
            return hi;
    }
    return NULL;    //Base case 2: No child of this with that name
}

//-------------------------------------------------------------------------------------
// HUDImage class functions
//-------------------------------------------------------------------------------------
HUDImage::HUDImage(string sName) : HUDItem(sName)
{
    m_img = NULL;
}

HUDImage::~HUDImage()
{

}

void HUDImage::draw(float32 fCurTime)
{
    HUDItem::draw(fCurTime);
    if(m_img != NULL)
    {
		//TODO
		//glColor4f(col.r,col.g,col.b,col.a);
        //m_img->draw(m_ptPos.x * m_iSCALE_FAC, m_ptPos.y * m_iSCALE_FAC);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);
    }
}

void HUDImage::setImage(Image* img)
{
    m_img = img;
}

//-------------------------------------------------------------------------------------
// HUDTextbox class functions
//-------------------------------------------------------------------------------------
HUDTextbox::HUDTextbox(string sName) : HUDItem(sName)
{
    m_iAlign = ALIGN_RIGHT | ALIGN_BOTTOM;
    m_txtFont = NULL;
}

HUDTextbox::~HUDTextbox()
{
}

void HUDTextbox::draw(float32 fCurTime)
{
    HUDItem::draw(fCurTime);
    if(m_txtFont == NULL) return;

    m_txtFont->setAlign(m_iAlign);
    m_txtFont->col = col;

    //Render a box around where this text will be
    Point ptSize = m_txtFont->sizeString(m_sValue);

    Rect rcText = {m_ptPos.x*m_iSCALE_FAC, m_ptPos.y*m_iSCALE_FAC, (m_ptPos.x+1)*m_iSCALE_FAC + ptSize.x, (m_ptPos.y+1)*m_iSCALE_FAC + ptSize.y};

    //Deal with alignment issues
    if(m_iAlign & ALIGN_CENTER)
        rcText.offset(-rcText.width()/2,0);
    if(m_iAlign & ALIGN_LEFT)
        rcText.offset(-rcText.width(),0);
    if(m_iAlign & ALIGN_MIDDLE)
        rcText.offset(0,-rcText.height()/2);
    if(m_iAlign & ALIGN_TOP)
        rcText.offset(0,-rcText.height());

    //Fill in bg
    //fillRect(rcText, fill.r, fill.g, fill.b, fill.a);

    //Render the text
    m_txtFont->render(m_sValue, m_ptPos.x*m_iSCALE_FAC, m_ptPos.y*m_iSCALE_FAC);
}

void HUDTextbox::setText(uint32_t iNum)
{
    ostringstream oss;
    oss << iNum;
    setText(oss.str());
}

//-------------------------------------------------------------------------------------
// HUD class functions
//-------------------------------------------------------------------------------------
HUDToggle::HUDToggle(string sName) : HUDItem(sName)
{
    m_iKey = SDLK_UNKNOWN;
    m_imgEnabled = NULL;
    m_imgDisabled = NULL;
    m_bValue = false;   //Default is disabled
}

HUDToggle::~HUDToggle()
{

}

void HUDToggle::event(SDL_Event event)
{
    HUDItem::event(event);
    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == m_iKey)
    {
        m_signalHandler(m_sSignal); //Generate signal
        m_bValue = !m_bValue;   //Toggle
    }
}

void HUDToggle::draw(float32 fCurTime)
{
    HUDItem::draw(fCurTime);

    if(m_bValue)    //Draw enabled image
    {
        if(m_imgEnabled != NULL)
        {
            glColor4f(col.r,col.g,col.b,col.a);
            //TODO m_imgEnabled->draw(m_ptPos.x*m_iSCALE_FAC, m_ptPos.y*m_iSCALE_FAC);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
        }
    }
    else    //Draw disabled image
    {
        if(m_imgDisabled != NULL)
        {
            glColor4f(col.r,col.g,col.b,col.a);
            //TODO m_imgDisabled->draw(m_ptPos.x*m_iSCALE_FAC, m_ptPos.y*m_iSCALE_FAC);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
        }
    }
}

void HUDToggle::setEnabledImage(Image* img)
{
    m_imgEnabled = img;
}

void HUDToggle::setDisabledImage(Image* img)
{
    m_imgDisabled = img;
}

//-------------------------------------------------------------------------------------
// HUDGroup class functions
//-------------------------------------------------------------------------------------
HUDGroup::HUDGroup(string sName) : HUDItem(sName)
{
    m_fFadeDelay = FLT_MAX;
    m_fFadeTime = FLT_MAX;
    m_fStartTime = FLT_MIN;
}

HUDGroup::~HUDGroup()
{

}

void HUDGroup::draw(float32 fCurTime)
{
    if(m_fStartTime == FLT_MIN)
        m_fStartTime = fCurTime;

    if(m_fFadeDelay != FLT_MAX && m_fFadeTime != FLT_MAX && fCurTime > m_fFadeDelay+m_fStartTime)
    {
        //We're multiplying by col.a again here so that we can nest groups safely. Why would we want to? I have no idea
        col.a = (m_fFadeTime - ((fCurTime - (m_fFadeDelay+m_fStartTime)))/(m_fFadeTime))*col.a;
    }

    //Draw all the children with this alpha, if we aren't at alpha = 0
    if(fCurTime < m_fFadeDelay+m_fStartTime+m_fFadeTime)
        HUDItem::draw(fCurTime);
}

void HUDGroup::event(SDL_Event event)
{
    HUDItem::event(event);

    if(event.type == SDL_KEYDOWN && m_mKeys.find(event.key.keysym.sym) != m_mKeys.end())
    {
        m_fStartTime = FLT_MIN; //Cause this to reset
    }
}

//-------------------------------------------------------------------------------------
// HUD class functions
//-------------------------------------------------------------------------------------
HUD::HUD(string sName) : HUDItem(sName)
{
}

HUD::~HUD()
{
    errlog << "Destroying HUD \"" << m_sName << "\"" << endl;

    destroy();
}

HUDItem* HUD::_getItem(XMLElement* elem)
{
    if(elem == NULL)
        return NULL;

    const char* cName = elem->Name();
    if(cName == NULL) return NULL;
    string sName(cName);
    if(sName == "images")    //Image list
    {
        for(XMLElement* elemImage = elem->FirstChildElement("image"); elemImage != NULL; elemImage = elemImage->NextSiblingElement())
        {
            if(elemImage == NULL) break;    //Done
            const char* cImgName = elemImage->Attribute("name");
            if(cImgName == NULL) continue;
            string sImgName(cImgName);
            const char* cImgPath = elemImage->Attribute("path");
            if(cImgPath == NULL) continue;
            Image* img = new Image(cImgPath);   //Load image
            m_mImages[sImgName] = img;          //Stick it into our list
        }
    }
    else if(sName == "fonts")    //Font list
    {
        //Load all fonts
        for(XMLElement* elemFont = elem->FirstChildElement("font"); elemFont != NULL; elemFont = elemFont->NextSiblingElement())
        {
            if(elemFont == NULL) break; //Done
            const char* cFontName = elemFont->Attribute("name");
            if(cFontName == NULL) continue;
            string sFontName(cFontName);
            const char* cFontPath = elemFont->Attribute("path");
            if(cFontPath == NULL) continue;
            Text* fon = new Text(cFontPath);   //Load font
            m_mFonts[sFontName] = fon;          //Stick it into our list
        }
    }
    else if(sName == "group")
    {
        //Loop through children, and add recursively
        const char* cGroupName = elem->Attribute("name");
        if(cGroupName == NULL) return NULL;
        HUDGroup* hGroup = new HUDGroup(cGroupName);
        bool bDefault = false;
        elem->QueryBoolAttribute("defaultenabled", &bDefault);
        if(!bDefault)
            hGroup->hide(); //Hide initially if we should
        float32 fDelay = FLT_MAX;
        elem->QueryFloatAttribute("fadedelay", &fDelay);
        hGroup->setFadeDelay(fDelay);
        float32 fTime = FLT_MAX;
        elem->QueryFloatAttribute("fadetime", &fTime);
        hGroup->setFadeTime(fTime);

        //-----------Parse keys and key nums
        int32_t iNumKeys = 0;
        elem->QueryIntAttribute("keynum", &iNumKeys);
        const char* cKeys = elem->Attribute("keys");
        if(cKeys != NULL && iNumKeys)
        {
            string sKeys = stripCommas(cKeys);
            istringstream iss(sKeys);
            for(int i = 0; i < iNumKeys; i++)
            {
                int32_t iKey = 0;
                iss >> iKey;
                hGroup->addKey(iKey);
            }
        }

        //Load all children of this group
        for(XMLElement* elemGroup = elem->FirstChildElement(); elemGroup != NULL; elemGroup = elemGroup->NextSiblingElement())
        {
            HUDItem* it = _getItem(elemGroup);  //Recursive call for group items
            hGroup->addChild(it);
        }
        return hGroup;
    }
    else if(sName == "toggleitem")
    {
        const char* cToggleName = elem->Attribute("name");
        if(cToggleName == NULL) return NULL;
        HUDToggle* tog = new HUDToggle(cToggleName);
        const char* cToggleImgOn = elem->Attribute("img_on");
        if(cToggleImgOn != NULL && cToggleImgOn[0] != '\0')
            tog->setEnabledImage(m_mImages[cToggleImgOn]);
        const char* cToggleImgOff = elem->Attribute("img_off");
        if(cToggleImgOff != NULL && cToggleImgOff[0] != '\0')
            tog->setDisabledImage(m_mImages[cToggleImgOff]);
        const char* cPosi = elem->Attribute("pos");
        if(cPosi != NULL)
        {
            Point ptPos = pointFromString(cPosi);
            tog->setPos(ptPos);
        }
        const char* cSig = elem->Attribute("signal");
        if(cSig != NULL)
            tog->setSignal(cSig);
        int32_t iKey = 0;
        elem->QueryIntAttribute("key", &iKey);
        tog->setKey(iKey);
        bool bEnabled = false;
        elem->QueryBoolAttribute("default", &bEnabled);
        tog->setEnabled(bEnabled);
        return (tog);
    }
    else if(sName == "bgimage")
    {
        const char* cHudImageName = elem->Attribute("name");
        if(cHudImageName == NULL) return NULL;
        const char* cImg = elem->Attribute("img");
        if(cImg == NULL) return NULL;
        //Create HUDImage
        HUDImage* hImg = new HUDImage(cHudImageName);
        hImg->setImage(m_mImages[cImg]);
        const char* cPos = elem->Attribute("pos");
        if(cPos != NULL)
        {
            Point ptPos = pointFromString(cPos);
            hImg->setPos(ptPos);
        }
        return (hImg); //Add this as a child of our HUD
    }
    else if(sName == "textbox")
    {
        const char* cTextName = elem->Attribute("name");
        if(cTextName == NULL) return NULL;
        const char* cTextFont = elem->Attribute("font");
        if(cTextFont == NULL) return NULL;
        HUDTextbox* tb = new HUDTextbox(cTextName);
        tb->setFont(m_mFonts[cTextFont]);
        const char* cDefaultText = elem->Attribute("default");
        if(cDefaultText != NULL)
            tb->setText(cDefaultText);
        const char* cPos = elem->Attribute("pos");
        if(cPos != NULL)
        {
            Point ptTextPos = pointFromString(cPos);
            tb->setPos(ptTextPos);
        }
        const char* cAlign = elem->Attribute("align");
        if(cAlign != NULL)
        {
            //Set font alignment
            string sAlign(cAlign);
            uint8_t iFontAlign = 0;
            if(sAlign.find("right") != sAlign.npos)
                iFontAlign |= ALIGN_RIGHT;
            if(sAlign.find("left") != sAlign.npos)
                iFontAlign |= ALIGN_LEFT;
            if(sAlign.find("center") != sAlign.npos)
                iFontAlign |= ALIGN_CENTER;
            if(sAlign.find("top") != sAlign.npos)
                iFontAlign |= ALIGN_TOP;
            if(sAlign.find("middle") != sAlign.npos)
                iFontAlign |= ALIGN_MIDDLE;
            if(sAlign.find("bottom") != sAlign.npos)
                iFontAlign |= ALIGN_BOTTOM;
            tb->setAlign(iFontAlign);
        }
        const char* cFill = elem->Attribute("fill");
        if(cFill != NULL)
        {
            //Get color to fill in background of textbox
            Color fillCol = colorFromString(cFill);
            tb->fill = fillCol;
        }
        return(tb);
    }
    
	else
        errlog << "Unknown HUD item \"" << sName << "\". Ignoring..." << endl;
    return NULL;
}

void HUD::create(string sXMLFilename)
{
    //Load in the XML document
    XMLDocument* doc = new XMLDocument();
    int iErr = doc->LoadFile(sXMLFilename.c_str());
	if(iErr != XML_NO_ERROR)
	{
		errlog << "Error parsing XML file " << sXMLFilename << ": Error " << iErr << endl;
		delete doc;
		return;
	}

    XMLElement* elem = doc->FirstChildElement("hud");
    if(elem == NULL)
	{
		errlog << "Error: No toplevel \"hud\" item in XML file " << sXMLFilename << endl;
		return;
	}
    const char* cName = elem->Attribute("name");
    if(cName != NULL)
        m_sName = cName;    //Grab the name
    errlog << "Creating HUD \"" << m_sName << "\"" << endl;
    //Load all elements
    for(elem = elem->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {
        if(elem == NULL) break;
        HUDItem* it = _getItem(elem);
        if(it != NULL)
            addChild(it);

    }
    delete doc;
}

void HUD::destroy()
{
  //Delete all images
  for(map<string, Image*>::iterator i = m_mImages.begin(); i != m_mImages.end(); i++)
    delete i->second;
  
  //And all fonts
  for(map<string, Text*>::iterator i = m_mFonts.begin(); i != m_mFonts.end(); i++)
    delete i->second;
  
  for(list<HUDItem*>::iterator i = m_lChildren.begin(); i != m_lChildren.end(); i++)
    delete (*i);
  m_lChildren.clear();
}



















