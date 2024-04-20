/*! ========================================================================

      @file    DXFont.h
      @author  jmc,jmp
      @brief   Implementation of DXFont.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _DX_FONT_H_
#define _DX_FONT_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"
#include "GraphicsPrimitive.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class DXFont : public Graphics::Primitive
/*! A text rendering object.
*/
{
  public:
    // enums
    enum Flags
    //! Constants for font properties.
    {
      kFlagBold   = 0x0001,  //!< Font is bold.
      kFlagItalic = 0x0002   //!< Font is italic.
    };
    
    // structs
    struct Vertex
    //! Represents a font vertex.
    {
      // format
      enum { Format = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 };
      
      // data
      float  x,y,z,rhw;  //!< Coordinates (pretransformed).
      DWORD  c;          //!< Color.
      float  u,v;        //!< Texture coordinates.
    };

    // ct and dt
    DXFont(Graphics::Renderer *renderer,const std::string &name,int height,DWORD flags);
    ~DXFont(void);

    // device objects
    void RestoreDeviceObjects(void);
    void ReleaseDeviceObjects(void);
    
    // rendering
	  void DrawText(int x,int y,int w,int h,DWORD color,int flags,const std::string &text);
    
    // lengths
    SIZE GetStringWidth(const std::string &str);
	  SIZE GetCharWidth(char c);
	  SIZE GetCharPosition(const std::string &str,int width,unsigned int index);
	  int  GetCharIndex(const std::string &str,int width,int x,int y);
	  int  GetLineHeight(void) { return (mFontHeight); }
	
	private:
	  // utility
	  void nFillTexture(void);
    void nFillVertex(float x,float y,float z,float rhw,DWORD c,float u,float v,Vertex* vtx);  
    
    // data members
    std::string  mFontName;    //!< Name of the font.
    int          mFontHeight;  //!< Font height.
    DWORD        mFontFlags;   //!< Font flags.

    LPD3DXFONT  mFont;      //!< Font renderer.
    RECT        mFontRect;  //!< Font rectangle.
    
    std::vector< SIZE >  mCharSizes;  //!< Sizes of each printable character (ASCII order).
};


#endif  /* _DX_FONT_H_ */

