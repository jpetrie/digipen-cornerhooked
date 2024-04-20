/*!
	@file		RuleSystem.h
	@author		Scott
	@date		09-13-2004
	@todo		
	@brief		Provides a rules management system, designed to be generic 
				and extensible.
	
	(c) 2004 DigiPen (USA) Corporation, all rights reserved.	
*//*__________________________________________________________________________*/

#include "main.h"
#include "game.h"
#include "RuleSystem.h"
#include "CollisionEngine.h"
#include "PhysicsAux.h"
#include "Log.h"
#include "playfieldbase.h"

using Physics::kC_BoundedPlane;
using Physics::kC_Plane;
using Physics::kC_Sphere;

Rules::Rules():BreakShot(true), LastScratch(NO_SCRATCH), TakeShot(false), winner(-1), game_over(false),FirstStruck(0) 
{
    Game::Get()->GetSession()->SetCurrentTurn(0);
}
Rules::~Rules()	{}

//
//	This function evaluates a collision type and adds the
//	appropriate collisions to their respective vectors
//
void RuleCollisionCB(Collision::Contact* c, Physics::RigidBody*, Physics::RigidBody*)
{
	if(c == 0)
		return;
	else if(c->mBody1 == 0 || c->mBody2 == 0)
		return;
	else if(c->mBody1->mCollideGeom == 0 || c->mBody2->mCollideGeom == 0)
		return;

	// sphere/plane
	if(c->mBody1->mCollideGeom->Kind() == kC_Plane && c->mBody2->mCollideGeom->Kind() == kC_Sphere)
	{
		Game::Get()->GetSession()->GetRules()->Ball_Wall.push_back(c);
		return;
	}
	else if(c->mBody1->mCollideGeom->Kind() == kC_Sphere && c->mBody2->mCollideGeom->Kind() == kC_Plane)
	{
		Game::Get()->GetSession()->GetRules()->Ball_Wall.push_back(c);
		return;
	}
	// sphere/sphere
	else if(c->mBody1->mCollideGeom->Kind() == kC_Sphere && c->mBody2->mCollideGeom->Kind() == kC_Sphere)
	{
		Game::Get()->GetSession()->GetRules()->Ball_Ball.push_back(c);
		return;
	}
	// Sphere/Pocket
	else if(c->mBody1->mCollideGeom->Kind() == kC_BoundedPlane && c->mBody2->mCollideGeom->Kind() == kC_Sphere)
	{
		Game::Get()->GetSession()->GetRules()->Ball_Pocket.push_back(c);
		return;
	}
	else if(c->mBody1->mCollideGeom->Kind() == kC_Sphere && c->mBody2->mCollideGeom->Kind() == kC_BoundedPlane)
	{
		Game::Get()->GetSession()->GetRules()->Ball_Pocket.push_back(c);
		return;
	}
}

void RuleCollisionSS_CB(Collision::Contact* c, Physics::RigidBody*, Physics::RigidBody*)
{
	Game::Get()->GetSession()->GetRules()->Ball_Ball.push_back(c);
	if (Game::Get()->GetSession()->GetRules()->FirstStruck == 0)
	{
		if(GetBallNumber(c->mID1) > 0)
		{
			Game::Get()->GetSession()->GetRules()->FirstStruck = GetBallNumber(c->mID1);
		}
		else if(GetBallNumber(c->mID2) > 0)
		{
			Game::Get()->GetSession()->GetRules()->FirstStruck = GetBallNumber(c->mID2);
		}
	}
	std::stringstream ss;
	ss << Game::Get()->GetSession()->GetRules()->Ball_Ball.size();
	//Game::Get()->WriteMessage(ss.str());
		
}
void RuleCollisionSP_CB(Collision::Contact* c, Physics::RigidBody*, Physics::RigidBody*)
{
	Game::Get()->GetSession()->GetRules()->Ball_Wall.push_back(c);
}
void RuleCollisionSBP_CB(Collision::Contact* c, Physics::RigidBody*, Physics::RigidBody*)
{
	Game::Get()->GetSession()->GetRules()->Ball_Pocket.push_back(c);
	if(c->mBody1->mCollideGeom->Kind() == Physics::kC_Sphere)
	{
		Game::Get()->GetSession()->GetRules()->PocketedBalls.push_back(GetBallNumber(c->mID1));
	}
	else
	{
		Game::Get()->GetSession()->GetRules()->PocketedBalls.push_back(GetBallNumber(c->mID2));
	}
}
bool Rules::BallsToRail(int req)
{
    if(static_cast<int>(Ball_Wall.size()) < req)
        return false;

    int cue_hit = 0;
    for(unsigned int i = 0; i < Ball_Wall.size(); ++i)
    {
        if( static_cast<int>(Ball_Wall[i]->mID1) == Game::Get()->GetPlayfield()->mBalls[0]->ID() || 
            static_cast<int>(Ball_Wall[i]->mID2) == Game::Get()->GetPlayfield()->mBalls[0]->ID())
        {
            ++cue_hit;
        }
    }
    return !!static_cast< int >(Ball_Wall.size() - cue_hit);
}

// test to see if a ball was pocketed on the current turn
 bool Rules::BallPocketed(int ball)
 {
     for(unsigned int i = 0; i < Ball_Pocket.size(); ++i)
     {
        if( GetBallNumber(Ball_Pocket[i]->mID1) == ball || 
            GetBallNumber(Ball_Pocket[i]->mID2) == ball)
        {
            return true;
        }
     }
     return false;
 }
bool Rules::PlayerBallHitFirst(void)
{
	int player = Game::Get()->GetSession()->CurrentTurn();
    // if the first collision is not with a ball from the players group
	bool good = (std::find(LegalBalls[player].begin(), LegalBalls[player].end(), FirstStruck) != LegalBalls[player].end());
	
//	std::stringstream fmt;
//	fmt << "Player ball hit:" << ((good) ? "true" : "false") << "						";
//	Game::Get()->WriteMessage(fmt.str());
	return good;
	//return true;
}

// returns a ball number
int  Rules::FirstBallSunk(void)
{
    if(PocketedBalls.size() > 0)
     return PocketedBalls[0];
    return -1;
    
}
// true if the ball was in the player's group
bool Rules::LegalBallSunk(int ball)
{
    int player = Game::Get()->GetSession()->CurrentTurn();
    std::vector< int >::iterator it = LegalBalls[player].begin();
    it = std::find(LegalBalls[player].begin(), LegalBalls[player].end(), ball);
    if(it != LegalBalls[player].end())
    {
        return true;
    }
    return false;
}
EighteenBallGroups Rules_EighteenBall::GetGroup(void)
{ 
    return Group[Game::Get()->GetSession()->CurrentTurn()]; 
}

struct lowball_pred
{
    bool operator()(const std::vector<Ball*>::iterator it1, const std::vector<Ball*>::iterator it2)
    {
        return ((*it1)->Number() < (*it2)->Number());
    }
};
#undef min

int Rules_NineteenBall::GetLowestBall(void)
{
    //return (*(std::min(Game::Get()->GetPlayfield()->mBalls.begin() + 1, Game::Get()->GetPlayfield()->mBalls.end(), lowball_pred())))->Number();
	int minball = 19;
	for (unsigned int i = 0; i < Game::Get()->GetPlayfield()->mBalls.size(); ++i)
	{
		if(!Game::Get()->GetPlayfield()->mBalls[i]->Pocketed() && 
			Game::Get()->GetPlayfield()->mBalls[i]->Number() != 0 && 
			/*std::find(PocketedBalls.begin(), PocketedBalls.end(), Game::Get()->GetPlayfield()->mBalls[i]->Number()) != PocketedBalls.end() &&*/
			Game::Get()->GetPlayfield()->mBalls[i]->Number() < minball)
		{
			minball = Game::Get()->GetPlayfield()->mBalls[i]->Number();
		}
	}
	return minball;
}
