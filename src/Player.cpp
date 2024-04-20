/*!
	@file    Player.cpp
	@author  Scott
	@date    09-23-2004

	@brief   Implementation of player class.

	(c) 2004 DigiPen (USA) Corporation, all rights reserved.
*//*__________________________________________________________________________*/

#include "main.h"

#include "Game.h"
#include "Camera.h"

#include "Player.h"

/*! Constructor.
*//*__________________________________________________________________________*/
Player::Player(void)
{
}

/*! Destructor.
*//*__________________________________________________________________________*/
Player::~Player(void)
{
}

Shot Player::SelectShot(void) 
{ 
	//::D3DXVECTOR3 v; 
	//Game::Get()->GetCamera()->GetViewVector(&v); 
	Shot s;
	//Geometry::Vector3D aim(v.x, v.y, v.z);
	//s.v = aim;
    Game::Get()->GetCamera()->GetViewVector(&(s.v)); 
	return s;
}

Shot Player::GetBestShot(void)
{
    return bestShot;
}

void Player::SetBestShot(const Shot& shot)
{
    bestShot = shot;
}