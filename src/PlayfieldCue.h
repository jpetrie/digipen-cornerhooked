/*! ========================================================================

      @file    PlayfieldCue.h
      @author  jmp
      @brief   Interface to playfield cue.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _PLAYFIELD_CUE_H_
#define _PLAYFIELD_CUE_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"

#include "DXLine.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

// line count
const int kCueLineCount = 10;

// shape 
const float  kCueRadiusTip = 0.15f;
const float  kCueRadiusEnd = 0.20f;
const float  kCueLength    = 50.0f;


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class Cue
/*! Cue class.
*/
{
  public:
    // ct and dt
    Cue(Graphics::Renderer *renderer);
    ~Cue(void);
    
    // update
    void Update(void);
    
    // render
    void Render(const D3DXVECTOR3 &vec,const D3DXVECTOR3 &target,float dist);
    
  private:
    // data members
    Graphics::Renderer *mRenderer;                 //!< Renderer that will draw cue elements.
    DXLine             *mLine;                     //!< Cue line renderer.
    float               mLineTime[kCueLineCount];  //!<
    float               mLineDir[kCueLineCount];   //!<
};


#endif  /* _PLAYFIELD_CUE_H_ */