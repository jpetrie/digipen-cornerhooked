/*! ========================================================================

      @file    PlayfieldCue.cpp
      @author  jmp
      @brief   Implementation of input manager.

      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "PlayfieldCue.h"

#include "Game.h"
#include "Camera.h"

#include "nsl_random.h"


/*                                                                 variables
---------------------------------------------------------------------------- */

namespace
{
  random  gRNG(::clock());
};


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
Cue::Cue(Graphics::Renderer *renderer)
/*! Constructor.
*/
: mRenderer(renderer)
{
  // Build lines.
  mLine = new DXLine(mRenderer,0xFFAB8200,0x7FAB8200);
  for(int i = 0; i < kCueLineCount; ++i)
  {
    mLineTime[i] = gRNG.rand_float(0.0f,5.0f);
    mLineDir[i]  = gRNG.rand_float(-0.02f,0.02f);
  }
}

/*  ________________________________________________________________________ */
Cue::~Cue(void)
/*! Destructor.
*/
{
  SAFE_DELETE(mLine);
}

/*  ________________________________________________________________________ */
void Cue::Update(void)
/*! Update the cue.
*/
{
  for(int i = 0; i < kCueLineCount; ++i)
    mLineTime[i] += mLineDir[i];
}

/*  ________________________________________________________________________ */
void Cue::Render(const D3DXVECTOR3 &vec,const D3DXVECTOR3 &target,float dist)
/*! Render the cue.

    @param vec     The vector along which the cue is oriented.
    @param target  Position of the cue target.
    @param dist    Distance from target to cue tip.
*/
{
D3DXVECTOR3 v,up;

  // get the up vector
  Game::Get()->GetCamera()->GetUpVector(&up);

  // if the up vector and the aim vector are the same, use a new up
  if(vec == up)
  {
    up.x = 1.f;
    up.y = 0.f;
    up.z = 0.f;
  }

  // aim vector crossed with the up vector gives a vector that is perp to aim
  D3DXVec3Cross(&v, &vec, &up);

  // normalize
  D3DXVec3Normalize(&v, &v);

  float resolution = 20.f;
  float step = (kPI * 2.f) / resolution;

  // normalize the direction vector
  D3DXVECTOR3 dir;
  D3DXVec3Normalize(&dir, &vec);

  // displace from the target to the 'butt' of the cue
  D3DXVECTOR3 pos = target + (2.f * dist * (-dir));
  float butt_r = .2f;
  float tip_r = .2f;

  for(float ang = 0.f; ang <= (kPI * 2.f); ang += step)
  {
    D3DXVECTOR3 butt;
    D3DXVECTOR3 tip;
    D3DXMATRIX r;
    D3DXMatrixRotationAxis(&r, &dir, ang);
    D3DXVec3TransformCoord(&butt, &butt, &r);
    D3DXVec3TransformCoord(&tip, &tip, &r);

    D3DXVec3Normalize(&butt, &butt);
    D3DXVec3Normalize(&tip, &tip);
    D3DXVECTOR3 res1 = (pos + 2.f * -dir) + butt_r * butt;
    D3DXVECTOR3 res2 = (pos + 30.f * -dir) + tip_r * tip;

    mLine->DrawLine(res1.x, res1.y, res1.z, res2.x, res2.y, res2.z);
  }
}
