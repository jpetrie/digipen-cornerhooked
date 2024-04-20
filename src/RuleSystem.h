/*!
	@file		RuleSystem.h
	@author		Scott
	@date		09-13-2004
	@todo		
	@brief		Provides a rules management system, designed to be generic 
				and extensible.
	
	(c) 2004 DigiPen (USA) Corporation, all rights reserved.	
*//*__________________________________________________________________________*/

#pragma once

#include "main.h"
#include "player.h"

namespace Collision
{
	class Contact;
};

namespace Physics
{
	class RigidBody;
};

const int kCallbackRuleLog = 4;
const int kCallbackRuleSS  = 5;
const int kCallbackRuleSP  = 6;
const int kCallbackRuleSBP = 7;


void RuleCollisionCB(Collision::Contact *, Physics::RigidBody *, Physics::RigidBody *);

void RuleCollisionSS_CB(Collision::Contact* c, Physics::RigidBody*, Physics::RigidBody*);
void RuleCollisionSP_CB(Collision::Contact* c, Physics::RigidBody*, Physics::RigidBody*);
void RuleCollisionSBP_CB(Collision::Contact* c, Physics::RigidBody*, Physics::RigidBody*);

enum eScratchType
{
	TABLE_SCRATCH = 0,
	SUNK_CUE,
	ILLEGAL_BREAK,
	FAIL_TO_STRIKE_BALL,
	WRONG_BALL_STRUCK,
	NO_SCRATCH,
	TYPE_COUNT
};

// game types
enum eGameType
{
	EIGHTEEN_BALL = 0,
	NINETEEN_BALL,
//	STRAIGHT_POOL,
//	TWO_ELEVEN,
	GAME_TYPE_COUNT
};

const std::string Scratches[TYPE_COUNT] = 
{
	"Table Scratch",
	"Sunk Cue Ball",
	"Illegal Break",
	"Failed to strike ball",
	"Incorrect ball struck",
	"No scratch"
};

const std::string GameTypes[ GAME_TYPE_COUNT + 1 ] =
{
	"Eighteen Ball",
	"Nineteen Ball",
//	"Straight Pool",
//	"Two-Eleven",
	"Game type has not yet been selected."
};

const unsigned int GameMaxPlayers[ GAME_TYPE_COUNT ] =
{
	2 ,
	8 ,
//	8 ,
//	8
} ;

class Rules
{
public:
	// ct & dt
	Rules();
	~Rules();

	// Test a shot for legallity.  Returns true if the shot is legal.
	virtual bool Test() = 0;

    // resolve scratches
	virtual void HandleScratch(const eScratchType) = 0;

    virtual eGameType GetGameType(void) = 0;
	
    // scratch state functions
    eScratchType GetLastScratch(void)           { return LastScratch; }
    void         SetScratchType(eScratchType t) { LastScratch = t; }

    // state functions
	bool TookShot(void)		                                        { return TakeShot; }
	void TookShot(bool b)	                                        { TakeShot = b; }
    bool Break(void)		                                        { return BreakShot; }
    bool GameOver(void)                                             { return game_over; }
    int  Winner(void)                                               { return GameOver() ? winner : -1; }
    std::vector< int >   GetLegalBalls(int player)                  { return LegalBalls[player]; }
    int                  GetScore(int player)                       { return Score[player];      }
    
    // utility functions
    bool BallsToRail(int);
    bool BallPocketed(int);
    bool PlayerBallHitFirst(void);
    int  FirstBallSunk(void);
    bool LegalBallSunk(int ball);
    virtual void ResolvePocketedBalls(void) = 0;

    bool BreakShot;
	std::vector< Collision::Contact *>  Ball_Ball;
	std::vector< Collision::Contact *>  Ball_Wall;
	std::vector< Collision::Contact * > Ball_Pocket;
    std::vector< int >                  PocketedBalls;

	//std::list< Collision::Contact * > Collisions;
	int where;
	int	FirstStruck;
private:
	
protected:
	eScratchType LastScratch;
	//void SortCollisions(Player *);
	bool TakeShot;
    bool game_over;
    int  winner;
	std::vector< std::vector< int > > LegalBalls;
    std::vector< int > Score;
};

enum EighteenBallGroups 
{
    OPEN = 0,
    SOLIDS,
    STRIPES,
    EIGHTEEN
};

class Rules_EighteenBall : public Rules 
{
public:
    Rules_EighteenBall();
    virtual ~Rules_EighteenBall() {}
	// Test a shot for legallity.  Returns true if the shot is legal.
	virtual bool Test();

	// bool handle scratch
	virtual void HandleScratch(const eScratchType);

    virtual eGameType GetGameType(void)  { return EIGHTEEN_BALL; }
    EighteenBallGroups GetGroup(void);

private:
	bool TestBreak(Player *);
    virtual void ResolvePocketedBalls(void);
    std::vector< EighteenBallGroups > Group;

};

class Rules_NineteenBall : public Rules 
{
public:
    Rules_NineteenBall();
    virtual ~Rules_NineteenBall() {}

	// Test a shot for legality.  Returns true if the shot is legal.
	virtual bool Test();

	// bool handle scratch
	virtual void HandleScratch(const eScratchType);
    
    virtual eGameType GetGameType(void)  { return NINETEEN_BALL; }
    int Rules_NineteenBall::GetLowestBall(void);

	void ResolvePocketedBalls(void);





};