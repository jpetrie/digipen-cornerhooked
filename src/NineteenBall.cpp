
#include "main.h"
#include "game.h"
#include "RuleSystem.h"
#include "CollisionEngine.h"
#include "PhysicsAux.h"
#include "Log.h"
#include "playfieldbase.h"

Rules_NineteenBall::Rules_NineteenBall()
{
    int sz = Game::Get()->GetSession()->GetPlayersMax();
    LegalBalls.resize(sz);
    Score.resize(sz);
	
    for(int i = 0; i < sz; ++i)
    {
		LegalBalls[i].push_back(1);
        Score[i] = 0;
    }
    
}

struct NineteenBallSortPred
{
	bool operator()(const std::vector< Collision::Contact >::iterator lhs, const std::vector< Collision::Contact >::iterator rhs)
	{
		return ((*lhs).mTime < (*rhs).mTime);
	}
};

// test a shot for legality
bool  Rules_NineteenBall::Test()
{
//	std::stringstream ss;
//	for(unsigned int i = 0; i < PocketedBalls.size(); ++i)
//	{
//		ss << PocketedBalls[i] << "  ";
//	}
//	Game::Get()->WriteMessage(ss.str());
    

	bool ret = false;

	// this is a rotation game - so sorting makes sense
	//std::sort(Ball_Ball.begin(), Ball_Ball.end(), NineteenBallSortPred());
    Game::Get()->GetSession()->ballInHandPlaneDistance = 27;
	// ensure that the lowest numbered ball is struck first
    if(BallPocketed(0))
    {
        SetScratchType(SUNK_CUE);
        return false;
    }
	if(PlayerBallHitFirst())
	{
		SetScratchType(NO_SCRATCH);
		ret = true;
	}
	else 
	{
		SetScratchType(WRONG_BALL_STRUCK);
		return false;
	}	
//	std::stringstream fmt;
//	fmt << FirstStruck << "      ";
	//Game::Get()->WriteMessage(fmt.str());

	// make sure a ball was pocketed
//	fmt << Ball_Pocket.size() << "   ";
//	fmt << "legal ball struck												";
//	Game::Get()->WriteMessage(fmt.str());
	if (PocketedBalls.size() == 0)
	{
		SetScratchType(NO_SCRATCH);
		return false;
	}
	else if(BallPocketed(19))
	{
		winner = Game::Get()->GetSession()->CurrentTurn();
		game_over = true;
	}


	return ret;
}

// bool handle scratch
void Rules_NineteenBall::HandleScratch(const eScratchType scratch)
{
	switch(scratch)
	{
	case TABLE_SCRATCH:
		{
		}
		break;
	case FAIL_TO_STRIKE_BALL:
		{
		}
		break;
	default:
		{
		}
	}

  if(/*GetBallByNumber(0)->Pocketed()*/scratch != NO_SCRATCH)
  {
      if(GetBallByNumber(0)->Pocketed())
          GetBallByNumber(0)->Pocketed(false);

      Game::Get()->needToSpot = true;
    //  Game::Get()->GetPhysics()->RigidBodyVector3D(GetBallByNumber(0)->ID(), Physics::Engine::eRigidBodyVector::propPosition, Vector3D(0,0,-25));
      //if(Game::Get()->GetSession()->CurrentTurn() == Game::Get()->GetMyTurn())
      //    sm->TransitionTo(Game::Get()->GetSession()->BallInHandSID());
  }
    
	Ball_Ball.clear();
	Ball_Wall.clear();
	Ball_Pocket.clear();
    
    ResolvePocketedBalls();
	FirstStruck = 0;
    PocketedBalls.clear();

	for(unsigned int i = 0; i < LegalBalls.size(); ++i)
		LegalBalls[i][0] = GetLowestBall();
   
}

#undef min
void Rules_NineteenBall::ResolvePocketedBalls(void)
{
	unsigned int player_count = 0;
    for(unsigned int j = 0; j < Game::Get()->GetSession()->mPlayers.size(); ++j)
    {
        if(Game::Get()->GetSession()->mPlayers[j] != 0)
            ++player_count;
    }
    for(unsigned int i = 0; i < PocketedBalls.size(); ++i)
    {
        for(unsigned int p = 0; p < player_count; ++p)
        {
            std::vector<int>::iterator it = LegalBalls[p].begin();
            if(LegalBalls[p].end() != std::find(LegalBalls[p].begin(), LegalBalls[p].end(), PocketedBalls[i]))
            {
				LegalBalls[p][0] = GetLowestBall();
            }
		}
        
    }
	
	//int lowball = 1;
	//for(unsigned int lb = 0; lb < Game::Get()->GetPlayfield()->mBalls.size(); ++lb)
	//{
	//	if(!Game::Get()->GetPlayfield()->mBalls[lb]->Pocketed() && 
	//		Game::Get()->GetPlayfield()->mBalls[lb]->Number() != 0 &&
	//		std::find(PocketedBalls.begin(), PocketedBalls.end(), lb) != PocketedBalls.end())
	//	{
	//		if(Game::Get()->GetPlayfield()->mBalls[lb]->Number() < lowball && lowball != inball)
	//		{
	//			lowball = Game::Get()->GetPlayfield()->mBalls[lb]->Number();
	//		}
	//	}
	//}
	//for(unsigned int p = 0; p < player_count; ++p)
	//{
	//	//std::vector<int>::iterator it = LegalBalls[p].begin();
	//	//if(LegalBalls[p].end() != std::find(LegalBalls[p].begin(), LegalBalls[p].end(), lowball))
	//	//{
	//	LegalBalls[p].push_back(lowball);
	//	//}
	//}
}
