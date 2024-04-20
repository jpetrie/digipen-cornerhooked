
#include "main.h"
#include "game.h"
#include "RuleSystem.h"
#include "CollisionEngine.h"
#include "PhysicsAux.h"
#include "Log.h"
#include "playfieldbase.h"

Rules_EighteenBall::Rules_EighteenBall()
{
    int sz = Game::Get()->GetSession()->GetPlayersMax();
    Group.resize(sz, OPEN);
    LegalBalls.resize(sz);
    Score.resize(sz);
    // for each player
    for(int i = 0; i < sz; ++i)
    {
        // add each ball to the 'shootable' list
        for(int j = 1; j < 36; ++j)
        {
            if(j == 18)
                continue;
            LegalBalls[i].push_back(j);
        }
        Score[i] = 0;
    }
    Game::Get()->GetSession()->ballInHandPlaneDistance = -24;
}



// test a shot for legallity
bool  Rules_EighteenBall::Test()
{
    //LogS->Post(__FUNCTION__);
    //int max = Game::Get()->GetSession()->GetPlayersCur();
    std::stringstream msg;
    
    if(Group[Game::Get()->GetSession()->CurrentTurn()] != EIGHTEEN && LegalBalls[Game::Get()->GetSession()->CurrentTurn()].size() == 0)
    {
        Group[Game::Get()->GetSession()->CurrentTurn()] = EIGHTEEN;
        LegalBalls[Game::Get()->GetSession()->CurrentTurn()].push_back(18);
    }

    EighteenBallGroups group = Group[Game::Get()->GetSession()->CurrentTurn()];
    std::stringstream ss;
    ss << "Player " << Game::Get()->GetSession()->CurrentTurn() << " shooting at ";
    if(group == OPEN)
        ss << "OPEN";
    if(group == SOLIDS)
        ss << "SOLIDS";
    if(group == STRIPES)
        ss << "STRIPES";
    if(group == EIGHTEEN)
        ss << "EIGHTEEN";

    ss << "- First ball sunk: " << FirstBallSunk();
    //Game::Get()->WriteMessage(ss.str());

    /*std::stringstream lbz;
    for(unsigned int i = 0; i < LegalBalls[Game::Get()->GetSession()->CurrentTurn()].size(); ++i)
    {
        lbz << LegalBalls[Game::Get()->GetSession()->CurrentTurn()][i] << " ";
    }
    Game::Get()->WriteMessage(lbz.str());*/

    LastScratch = NO_SCRATCH;
	bool keep_shooting = false;
   
    // check for the special case that this is the break
	if( Break() )
    {
        // if no balls hit
        if(Ball_Ball.size() == 0)
        {
			SetScratchType(ILLEGAL_BREAK);
            where = __LINE__;
        }
        
        else if( BallPocketed(0) )
        {
            SetScratchType(SUNK_CUE);
            where = __LINE__;
        }

        // else if less than four walls were hit
        else if( !BallsToRail(4) )
        {
            if(Ball_Pocket.size() == 0)
            {
                SetScratchType(ILLEGAL_BREAK);
                where = __LINE__;
            }
            else
            {
                SetScratchType(NO_SCRATCH);
                where = __LINE__;
                keep_shooting = true;
            }
        }
        	
        // victory condition on break       
        else if(BallPocketed(18))
        {
            winner = Game::Get()->GetSession()->CurrentTurn();
            where = __LINE__;
            game_over = true;
        }
        else
        {
            SetScratchType(NO_SCRATCH);
            where = __LINE__;
            keep_shooting = false;
            BreakShot = false;
        }

        if(keep_shooting)
        {        
            BreakShot = false;
        }
		// if the break is not successful, the incoming player has the option of breaking, or accepting the table
		// if the cueball is pocketed, all other balls are spotted
    }
    // the table is still open after the break	
    // if it is not the break
    else
    {
        // if no balls hit
        std::stringstream ss;
        ss << "Balls hit: " << Ball_Ball.size();
        //Game::Get()->WriteMessage(ss.str());

        if(Ball_Ball.size() < 0) ///@todo CHANGE THIS NOW - why aren't collisions getting detected
        {
			// SCRATCH - stop shooting
            SetScratchType(FAIL_TO_STRIKE_BALL);
            where = __LINE__;
            goto done_shooting_18;
            //return false;
        }
        
        // check to see if the eighteen ball has been pocketed
        if(BallPocketed(18) && Group[Game::Get()->GetSession()->CurrentTurn()] != EIGHTEEN)
        {
            winner = (Game::Get()->GetSession()->CurrentTurn() + 1) % 2;
            where = __LINE__;
            game_over = true;
            goto done_shooting_18;
        }
        else if(BallPocketed(18) && Group[Game::Get()->GetSession()->CurrentTurn()] == EIGHTEEN)
        {
            // test for ball sunk in called pocket
            winner = Game::Get()->GetSession()->CurrentTurn();
            game_over = true;
            goto done_shooting_18;
        }
        
        if(!PlayerBallHitFirst())
        {
            SetScratchType(WRONG_BALL_STRUCK);
            where = __LINE__;
            goto done_shooting_18;
        }
                
        // else if the cueball is pocketed
        if(BallPocketed(0))
        {
            SetScratchType(SUNK_CUE);
            where = __LINE__;
            goto done_shooting_18;
        }

        // else if a rail was not hit
        if(Ball_Wall.size() == 0 && Ball_Pocket.size() == 0)
        {
			// SCRATCH - stop shooting
            SetScratchType(TABLE_SCRATCH);
            where = __LINE__;
            goto done_shooting_18;
        }
        

        // if the player has no legal group to shoot at, make a determination, and assign groups
        if(Group[Game::Get()->GetSession()->CurrentTurn()] == OPEN)
        {
            int curr = Game::Get()->GetSession()->CurrentTurn(), next;
            //int max = Game::Get()->GetSession()->GetPlayersMax();
           
            int  turn = (Game::Get()->GetSession()->CurrentTurn() + 1) % Game::Get()->GetSession()->GetPlayersMax();
            while(Game::Get()->GetSession()->GetPlayer(turn) == 0)
                turn = (turn + 1) % Game::Get()->GetSession()->GetPlayersMax();
            next = turn;
        
            std::stringstream fmt;
            // check that a ball has been pocketed and that the pocketed ball was not the cueball
            if(Ball_Pocket.size() > 0 && !BallPocketed(0) && FirstBallSunk() > 0)
            {
                LegalBalls[curr].clear();
                LegalBalls[next].clear();
                // if we are less than the eighteen ball, we are 'solids'
                if(FirstBallSunk() < 18)
                {
                    // set the groups
                    Group[curr] = SOLIDS;
                    
                    // add the balls
                    for(unsigned int i = 1; i < Game::Get()->GetPlayfield()->mBalls.size(); ++ i)
                    {
                        // don't add the ball we just pocketed to the list, we would just need to remove it later
                        if(Game::Get()->GetPlayfield()->mBalls[i]->Number() < 18)
                        {
                            LegalBalls[curr].push_back(Game::Get()->GetPlayfield()->mBalls[i]->Number());
                        }
                    }

                    // set the groups
                    Group[next] = STRIPES;
                    // add the balls
                    for(unsigned int i = 0; i < Game::Get()->GetPlayfield()->mBalls.size(); ++ i)
                    {
                        // don't add the ball we just pocketed to the list, we would just need to remove it later
                        if(Game::Get()->GetPlayfield()->mBalls[i]->Number() > 18)
                        {
                            LegalBalls[next].push_back(Game::Get()->GetPlayfield()->mBalls[i]->Number());
                        }
                    }
                    
                    fmt << "Solids: ";
                    
                }
                // we are 'stripes'
                else if(FirstBallSunk() > 18)
                {
                    // set the group
                    Group[curr] = STRIPES;
                    // add the balls
                    for(unsigned int i = 0; i < Game::Get()->GetPlayfield()->mBalls.size(); ++ i)
                    {
                        // don't add the ball we just pocketed to the list, we would just need to remove it later
                        if(Game::Get()->GetPlayfield()->mBalls[i]->Number() > 18)
                        {
                            LegalBalls[curr].push_back(Game::Get()->GetPlayfield()->mBalls[i]->Number());
                        }
                    }

                    // set the groups
                    Group[ next ] = SOLIDS;
                    // add the balls
                    for(unsigned int i = 1; i < Game::Get()->GetPlayfield()->mBalls.size(); ++ i)
                    {
                        // don't add the ball we just pocketed to the list, we would just need to remove it later
                        if(Game::Get()->GetPlayfield()->mBalls[i]->Number() < 18)
                        {
                            LegalBalls[next].push_back(Game::Get()->GetPlayfield()->mBalls[i]->Number());
                        }
                    }
                    fmt << "Stripes";
                }

            }
        }

        // no 
//balls pocketed - not a scratch, but don't keep shooting
        if(Ball_Pocket.size() > 0)
        {
            int turn = Game::Get()->GetSession()->CurrentTurn();
            if(PocketedBalls.size() > 0 && LegalBallSunk(PocketedBalls[0]))
            {
                SetScratchType(NO_SCRATCH);
                where = __LINE__;
                keep_shooting = true;
                std::vector<int>::iterator it;
                it = std::find(LegalBalls[turn].begin(), LegalBalls[turn].end(), PocketedBalls[0]);
                // don't erase the cueball
                if(it != LegalBalls[turn].end() && *it != 0)
                {
                    LegalBalls[turn].erase(it);
                    Score[turn]++;
                }
            }
            else
            {
                SetScratchType(TABLE_SCRATCH);
                where = __LINE__;
                keep_shooting = false;
            }

            TakeShot = false;
            std::stringstream fmt;
            fmt << "Pocketed: " << Ball_Pocket.size() << " balls.";
            return keep_shooting;
            //Game::Get()->WriteMessage(fmt.str());
        }
        

        keep_shooting = false;
    }
// I know.
done_shooting_18:

    TakeShot = false;
    if(keep_shooting)
    {
        SetScratchType(NO_SCRATCH);
    }

    

    return keep_shooting;

}

// bool handle scratch
void Rules_EighteenBall::HandleScratch(const eScratchType scratch)
{
	switch(scratch)
	{
	case TABLE_SCRATCH:
		{
		}
		break;
	case NO_SCRATCH:
		{
		}
		break;
	case FAIL_TO_STRIKE_BALL:
		{
		}
		break;
	case ILLEGAL_BREAK:
		{
			// spot the cueball
			Game::Get()->GetPhysics()->RigidBodyVector3D(GetBallByNumber(0)->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0,0,-25));
		}
		break;
	default:
		{
		}
	}

  if(GetBallByNumber(0)->Pocketed())
  {
      GetBallByNumber(0)->Pocketed(false);
      Game::Get()->needToSpot = true;
      
     // Game::Get()->GetPhysics()->RigidBodyVector3D(GetBallByNumber(0)->ID(), Physics::Engine::eRigidBodyVector::propPosition, Vector3D(0,0,-25));
  }
                
	//Collisions.clear();
	Ball_Ball.clear();
	Ball_Wall.clear();
	Ball_Pocket.clear();
    
  ResolvePocketedBalls();
	FirstStruck = 0;
  PocketedBalls.clear();
}

void Rules_EighteenBall::ResolvePocketedBalls(void)
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
                LegalBalls[p].erase(it);
            }
            if(Group[p] != EIGHTEEN && LegalBalls[p].size() == 0)
            {
                Group[Game::Get()->GetSession()->CurrentTurn()] = EIGHTEEN;
                LegalBalls[Game::Get()->GetSession()->CurrentTurn()].push_back(18);
            }
        }
        
    }
}

