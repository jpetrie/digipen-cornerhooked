/*! ========================================================================

      @file    DXFont.h
      @author  jmc,jmp
      @brief   Implementation of DXFont.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "DXFont.h"

#include "Game.h"
#include "Window.h"


/*                                                                 variables
---------------------------------------------------------------------------- */

namespace
{
  LPD3DXSPRITE  gFontSprite;
};


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
DXFont::DXFont(Graphics::Renderer *renderer,const std::string &name,int height,DWORD flags)
/*! Constructor.
    
    @param renderer  Renderer to attach to.
    @param name      Name of font to use.
    @param height    Height of font.
    @param flags     Font creation flags.
*/
: Graphics::Primitive(renderer),
  mFontName(name),mFontHeight(height),mFontFlags(flags),
  mFont(0)
{
	RestoreDeviceObjects();
}

/*  ________________________________________________________________________ */
DXFont::~DXFont(void)
/*! Destructor.
*/
{
  ReleaseDeviceObjects();
}

/*  ________________________________________________________________________ */
void DXFont::RestoreDeviceObjects(void)
/*! Restore D3D device objects.
*/
{	
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();

  D3DXCreateSprite(dev,&gFontSprite);

  D3DXCreateFont(dev,mFontHeight,0,0,1,false,0,0,0,0,mFontName.c_str(),&mFont);
  mFontRect.top    = 0;
  mFontRect.left   = 0;
  mFontRect.right  = Game::Get()->GetWindow()->Width();
  mFontRect.bottom = Game::Get()->GetWindow()->Height();

  // Compute character sizes.
HDC  dc = mFont->GetDC();

  // 32 is ASCII for space, and everything up to 127 is printable.
  mCharSizes.clear();
  for(unsigned char c = 32; c < 127; ++c)
  {
  SIZE  sz = { 0,0 };
  
    ::GetTextExtentPoint32(dc,reinterpret_cast< LPCSTR >(&c),1,&sz);
    mCharSizes.push_back(sz);
  }
  
  mFont->OnResetDevice();
  gFontSprite->OnResetDevice();
}

/*  ________________________________________________________________________ */
void DXFont::ReleaseDeviceObjects(void)
/*! Release D3D device objects.
*/
{
  mFont->OnLostDevice();
  if(gFontSprite != 0)
    gFontSprite->OnLostDevice();
  SAFE_RELEASE(gFontSprite);
  SAFE_RELEASE(mFont);
}

/*  ________________________________________________________________________ */
void DXFont::DrawText(int x,int y,int w,int h,DWORD color,int flags,const std::string &text)
/*! Render text to the screen.

    The coordinates specify the top-left of the text area rendered.

    @param x      X coordinate of the text.
    @param y      Y coordinate of the text.
    @param w      Width of the text rectangle.
    @param h      Height of the text rectangle.
    @param color  Color of the text.
    @param flags  Flags.
    @param text   Text to render.
*/
{
  // Early out if empty.
  if(text.empty())
    return;
 
  mFontRect.left   = x;
  mFontRect.top    = y;
  mFontRect.right  = x + w;
  mFontRect.bottom = y + h;
  gFontSprite->Begin(0);
  mFont->DrawText(gFontSprite,text.c_str(),-1,&mFontRect,flags,color);
  gFontSprite->End();
}

/*  ________________________________________________________________________ */
SIZE DXFont::GetStringWidth(const std::string &str)
/*! Compute the pixel width and height of a string of text.

    @param str  The string to calculate.
    
    @return A SIZE object containing the width and height of the string,
            in pixels. 
*/
{
SIZE  sz = {0,0};

  for(unsigned int i = 0; i < str.size(); ++i)
  {
    if(str[i] >= 32 && str[i] < 127)
      sz.cx += mCharSizes[str[i] - 32].cx;
    if(mCharSizes[str[i] - 32].cy > sz.cy)
      sz.cy = mCharSizes[str[i] - 32].cy;
  }
  
  return (sz);
}

/*  ________________________________________________________________________ */
SIZE DXFont::GetCharWidth(char c)
/*! Compute the pixel width and height of a single character.

    @param c  The character.
    
    @return A SIZE object containing the width and height of the character.
*/
{
  if (c >= 32)
  {
    return (mCharSizes[c - 32]);
  }
  else
  {
    SIZE zero = {0,0};
    return zero;
  }
}

/*  ________________________________________________________________________ */
SIZE DXFont::GetCharPosition(const std::string &str,int width,unsigned int index)
/*! Recover the pixel coordinates of a character.

    @param str    The text string.
    @param width  Pixel width the string may occupy.
    @param index  Index of the character to query.
    
    @return
    The coordinate, in pixels, of the desired character, or (-1,-1) if the
    index is out of range. Coordinates of (0,0) are the top-left of the text
    area (the character at index 0 has those coordinates).
*/
{
SIZE  result = { -1,-1 };
  
  if(index > str.length())
    return (result);
  
  result.cx = result.cy = 0;    
  for(unsigned int i = 0; i < index; ++i)
  {
    result.cx += mCharSizes[str[i] - 32].cx;
    if(result.cx > width)
    {
      result.cx = 0;
      result.cy += mFontHeight;
    }
  }
  
  return (result);
}

/*  ________________________________________________________________________ */
int DXFont::GetCharIndex(const std::string &str,int width,int x,int y)
/*! Gets the index of a character, given pixel coordinates.

    The input coordinates should be relative to the top-left of the text.

    @param str    The text string.
    @param width  Pixel width the string may occupy.
    @param x      X coordinate.
    @param y      Y coordinate.
    
    @return
    The index of the clicked character.
*/
{
int  idx = 0;
int  ly  = 0;
int  lx  = 0;

  while(lx < x && ly < y && idx < static_cast< int >(str.size()))
  {
  SIZE  cz = mCharSizes[str[idx] - 32];
    
    lx += cz.cx;
    if(lx >= width)
    {
      ly += mFontHeight;
      lx = 0;
    }
    ++idx;
  }

  if(idx == 0)
    idx = 1;

  return (idx - 1);
}
