/* =========================================================================
   
    @file    Game_States.h
    @author  jmc
    @brief   Game playloop function.
    
    (c) 2004 DigiPen (USA) Corporation, all rights reserved.
   
   ========================================================================= */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Game.h"

#include "DXFont.h"
#include "DXRect.h"
#include "DXLine.h"
#include "DXSphere.h"

#include "Camera.h"
#include "Input.h"
#include "ball.h"

#include "UIElement.h"
#include "UIScreen.h"
#include "UIElement.h"
#include "DXCircle.h"

#include "Profiler.h"

#include "Clock.h"
#include "log.h"


/*                                                                 functions
---------------------------------------------------------------------------- */

//void Game::DoCubemapRender(void)
//{
//LPDIRECT3DDEVICE9   dev = mWindow->GetRenderer()->GetDevice();
//LPDIRECT3DSURFACE9  oldRT;
//LPDIRECT3DSURFACE9  curRT;
//D3DXVECTOR3         eyeV(0.0f,0.0f,0.0f);
//D3DXVECTOR3         atV;
//D3DXVECTOR3         upV;
//D3DXMATRIX          mat;
//  
//  // Render to the cube map.
//  dev->GetRenderTarget(0,&oldRT);
//  
//  D3DXMatrixIdentity(&mat);
//  mWindow->GetRenderer()->SetWorld(mat);
//  D3DXMatrixPerspectiveFovLH(&mat,D3DX_PI / 4.0f,1.0f,1.0f,2000.0f);
//  mWindow->GetRenderer()->SetProjection(mat);
//  
//  // +X
//  mWindow->GetRenderer()->Clear();
//  mWindow->GetRenderer()->BeginRender();
//  Skybox::cubeTexture->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_X,0,&curRT);
//  dev->SetRenderTarget(0,curRT);
//  atV = D3DXVECTOR3(1,0,0);
//  upV = D3DXVECTOR3(0,1,0);
//  D3DXMatrixLookAtLH(&mat,&eyeV,&atV,&upV);
//  mWindow->GetRenderer()->SetView(mat);
//  mSkybox->Render(eyeV,atV);  
//  mPlayfield->Render(eyeV,atV);
//  mWindow->GetRenderer()->EndRender();
//  
//  // -X
//  mWindow->GetRenderer()->Clear();
//  mWindow->GetRenderer()->BeginRender();
//  Skybox::cubeTexture->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_X,0,&curRT);
//  dev->SetRenderTarget(0,curRT);
//  atV = D3DXVECTOR3(-1,0,0);
//  upV = D3DXVECTOR3(0,1,0);
//  D3DXMatrixLookAtLH(&mat,&eyeV,&atV,&upV);
//  mWindow->GetRenderer()->SetView(mat);
//  mSkybox->Render(eyeV,atV);  
//  mPlayfield->Render(eyeV,atV);
//  mWindow->GetRenderer()->EndRender();
//  
//  // +Y
//  mWindow->GetRenderer()->Clear();
//  mWindow->GetRenderer()->BeginRender();
//  Skybox::cubeTexture->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_Y,0,&curRT);
//  dev->SetRenderTarget(0,curRT);
//  atV = D3DXVECTOR3(0,1,0);
//  upV = D3DXVECTOR3(0,0,1);
//  D3DXMatrixLookAtLH(&mat,&eyeV,&atV,&upV);
//  mWindow->GetRenderer()->SetView(mat);
//  mSkybox->Render(eyeV,atV);  
//  mPlayfield->Render(eyeV,atV);
//  mWindow->GetRenderer()->EndRender();
//  
//  // -Y
//  mWindow->GetRenderer()->Clear();
//  mWindow->GetRenderer()->BeginRender();
//  Skybox::cubeTexture->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_Y,0,&curRT);
//  dev->SetRenderTarget(0,curRT);
//  atV = D3DXVECTOR3(0,-1,0);
//  upV = D3DXVECTOR3(0,0,1);
//  D3DXMatrixLookAtLH(&mat,&eyeV,&atV,&upV);
//  mWindow->GetRenderer()->SetView(mat);
//  mSkybox->Render(eyeV,atV);  
//  mPlayfield->Render(eyeV,atV);
//  mWindow->GetRenderer()->EndRender();
//  
//  // +Z
//  mWindow->GetRenderer()->Clear();
//  mWindow->GetRenderer()->BeginRender();
//  Skybox::cubeTexture->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_Z,0,&curRT);
//  dev->SetRenderTarget(0,curRT);
//  atV = D3DXVECTOR3(0,0,1);
//  upV = D3DXVECTOR3(0,1,0);
//  D3DXMatrixLookAtLH(&mat,&eyeV,&atV,&upV);
//  mWindow->GetRenderer()->SetView(mat);
//  mSkybox->Render(eyeV,atV);  
//  mPlayfield->Render(eyeV,atV);
//  mWindow->GetRenderer()->EndRender();
//  
//  // -Z
//  mWindow->GetRenderer()->Clear();
//  mWindow->GetRenderer()->BeginRender();
//  Skybox::cubeTexture->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_Z,0,&curRT);
//  dev->SetRenderTarget(0,curRT);
//  atV = D3DXVECTOR3(0,0,-1);
//  upV = D3DXVECTOR3(0,1,0);
//  D3DXMatrixLookAtLH(&mat,&eyeV,&atV,&upV);
//  mWindow->GetRenderer()->SetView(mat);
//  mSkybox->Render(eyeV,atV);  
//  mPlayfield->Render(eyeV,atV);
//  mWindow->GetRenderer()->EndRender();
//
//  // Restore old RT.
//  dev->SetRenderTarget(0,oldRT);
//}

/*  ________________________________________________________________________ */
void Game::Playloop(void)
/*! Playloop heartbeat function. 
*/
{
ProfileFn;
int           time;
static int    fps      = 0;
static int    frame    = 0;
static int    timebase = 0;
static Clock  clock;
static Clock  prefMon;
static float  prefMonPerc[10] = { 0 };

  // Trivial updates.
  clock.Update();
  mInput->Update();
  
  // Invoke the update function for the active game state.
  // Update() must be called before mScreen->Update() to ensure that a 
  // valid screen exists.
  prefMon.Update();
  Update(clock.Elapsed());
  mScreen->Update(mInput->MouseX(),mInput->MouseY(),clock.Elapsed());
  prefMon.Update();
  prefMonPerc[0] = prefMon.Elapsed();

  // Turn indicator update
  UpdateTurnIndicator() ;

  // loop jukebox music if the current one playing has ended
  if( GetCurrentJBIndex() >= 0 && mSound && !mSound->IsPlaying( GetCurrentJBSong() ) )
	UI_GPJBNextClick();

  // FPS calculation.
	frame++;
	time = ::GetTickCount();
	if (time - timebase > 1000)
	{
		fps = int(frame * 1000.0 / (time-timebase));
		timebase = time;		
		frame = 0;
	}
	else
		fps = int(frame * 1000.0 / (time-timebase));

  D3DXMATRIX  matWorld;
  D3DXMATRIX  matView;
  D3DXMATRIX  matProj;
  D3DXVECTOR3  vecView;
  D3DXVECTOR3  vecPos;
  

  // Rendering pass.
  // CanRender() will, if needed, reset the render device
  // and its attached objects.
  if(mWindow->GetRenderer()->CanRender())
  {	 
  float  aspect = static_cast< float >(mWindow->Width()) / static_cast< float >(mWindow->Height());

    // Clear the screen and begin object rendering.
    mWindow->GetRenderer()->Clear();
    mWindow->GetRenderer()->BeginRender();
    
	  // Set world, view and projection matrices for 3D rendering.
    mCamera->GetViewVector(&vecView);
	  mCamera->GetPosVector(&vecPos);
    D3DXMatrixIdentity(&matWorld);
    D3DXMatrixPerspectiveFovLH(&matProj,D3DX_PI / 4.0f,aspect,1.0f,2000.0f);
    mCamera->GetViewTransform(&matView);

    mWindow->GetRenderer()->SetWorld(matWorld);
    mWindow->GetRenderer()->SetView(matView);
    mWindow->GetRenderer()->SetProjection(matProj);

    if(Game::Get()->CurrentState() != Game::Get()->splash1SID &&
       Game::Get()->CurrentState() != Game::Get()->splash2SID)
    {
    std::stringstream  fmt;
    
	    mSkybox->Render(vecPos,vecView);  
      mPlayfield->Render(vecPos,vecView);

		  SetParticleSphere();
      if(mParticleSystem)
	      mParticleSystem->Process( static_cast<unsigned int>( clock.Elapsed()*1000.0 ) );
       
      if(CurrentState() == GetGameplayStateID())
        fmt << "Press F1 for help.";// FPS: " << fps;
      //else
      //  fmt << "FPS: " << fps;
      
      mFPSFont->DrawText(kUI_DistFromEdge+1,2 * kUI_DistFromEdge + kUI_GPStatusBarH+1,256,256,0xFF000000,DT_LEFT,fmt.str().c_str());
      mFPSFont->DrawText(kUI_DistFromEdge,2 * kUI_DistFromEdge + kUI_GPStatusBarH,256,256,0xFFFFFFFF,DT_LEFT,fmt.str().c_str());

      // Update the message area.
      UpdateMessages();
      PrintMessages();
    }
  
    // Set world, view, projection matrices for 2D rendering.
    D3DXMatrixOrthoLH(&matProj,static_cast< float >(mWindow->Width()),static_cast< float >(mWindow->Height()),0.0f,2000.0f);
    D3DXMatrixTranslation(&matView,-(static_cast< float >(mWindow->Width()) / 2.0f),static_cast< float >(mWindow->Height()) / 2.0f,0.0f);
    
    mWindow->GetRenderer()->SetWorld(matWorld);
    mWindow->GetRenderer()->SetView(matView);
    mWindow->GetRenderer()->SetProjection(matProj);
    
    // Draw the UI.
	  DXRect::EnterRenderState();
    mScreen->Render();
    DXRect::LeaveRenderState();

    // Finish rendering and flip buffers.
	  mWindow->GetRenderer()->EndRender();
    mWindow->GetRenderer()->Present();
  }
}
