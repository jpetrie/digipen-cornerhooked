
#include "main.h"

#include "AIPlayer.h"
#include "nsl_random.h"

float convert_distance(float max, float ideal, float d)
{
	if(d > 0.f && d <= ideal)
		return (d / ideal);
	else if(d > ideal && d <= max)
		return (1.f - (d - ideal)/(max - ideal)); 
	else
		return 0.f;
}

static D3DXVECTOR3 Perturb(const D3DXVECTOR3& u, float score)
{
	D3DXVECTOR3 v = u;
	static random r;
	long n = r.rand_int(1, 10);
    float f = r.rand_float();

    if(f > score)
    {    
	    if(n == 2)
	    {
		    v.x *= 1.1f;
	    }
	    if(n == 3)
	    {
		    v.y *= 1.1f;
	    }
	    if(n == 4 || n == 6)
	    {
		    v.z *= 1.1f;
	    }
    }
	return v;
}

D3DXVECTOR3 GhostBall(const Pocket *p, int ball, GHOST_BALL /*type*/)	// choose the ghost ball to render as an aiming aid.
{
	Geometry::Vector3D B;
    D3DXVECTOR3 B2;
	//if(type == REAL)
	{
        int id = GetBallByNumber(ball)->ID();//Game::Get()->GetPlayfield()->mBalls[ball]->ID();
		B = Geometry::Vector3D(Game::Get()->GetPhysics()->RigidBodyVector3D(id, Physics::Engine::eRigidBodyVector::propPosition));
	}
	//else
	//{
	//	int id = Game::Get()->GetPlayfield()->mAIBalls[ball];
	//	B = Geometry::Vector3D(Game::Get()->GetAIPhysics()->RigidBodyVector3D(id, Physics::Engine::eRigidBodyVector::propPosition));
	//}
    B2.x = B[0];
    B2.y = B[1];
    B2.z = B[2];

    D3DXVECTOR3 P = p->CornerPoint();
    D3DXVECTOR3 U(B2 - P);
	//Geometry::Vector3D U(B - P);
    D3DXVECTOR3 u;
    ::D3DXVec3Normalize(&u, &U);
	//Geometry::Vector3D u = U.normal();
	//Geometry::Vector3D ret = (p.mCornerPoint + U + (2.f * u)) - Geometry::Point3D();
	D3DXVECTOR3 ret = (P + U + (2.f * u));

	return ret;
}
Shot AIPlayer::SelectShot(void)		// choose the best shot for the AI player to shoot.
{
	if(FirstShot)
	{
		FirstShot = false;
		Shot s;
		s.v = D3DXVECTOR3(0,0,23);//Geometry::Vector3D(0, 0, 23);
		//s.p = Geometry::Vector3D(0, 0, 1);
		return s;
	}
	std::vector< Shot > shots;
	std::vector< Ball* >::iterator it = Game::Get()->GetPlayfield()->mBalls.begin();
	std::vector< int >::iterator it2 = Game::Get()->GetPlayfield()->mAIBalls.begin();
    std::vector<int> pballs = Game::Get()->GetSession()->GetRules()->GetLegalBalls(Game::Get()->GetSession()->CurrentTurn());
    std::vector< Ball * > lballs;
    for(unsigned int i = 0; i < pballs.size(); ++i)
    {
        lballs.push_back(GetBallByNumber(pballs[i]));
    }
	
	Geometry::Vector3D c(Game::Get()->GetPhysics()->RigidBodyVector3D(GetBallByNumber(0)->ID(), Physics::Engine::eRigidBodyVector::propPosition));
	Geometry::Point3D cue(c[0], c[1], c[2]);
	
    // for each ball in the list of available balls
	for(unsigned int i = 0; i < lballs.size(); ++i)
	{
		Geometry::Vector3D pos = Game::Get()->GetPhysics()->RigidBodyVector3D(lballs[i]->ID(), Physics::Engine::eRigidBodyVector::propPosition);
		Geometry::Sphere3D obj(Geometry::Point3D(pos[0], pos[1], pos[2]), 2.0);
        
        // test that the ball is in the player's group of legal balls.
        //Ball* pBall = lballs[i];
		
        //if(pBall->Pocketed())
        //    continue;
        
        // test the ball against every pocket
		for(unsigned int p = 1; p < Game::Get()->GetPlayfield()->mPockets.size(); ++p)
		{
			// the corner of the pocket
            D3DXVECTOR3 Corner(Game::Get()->GetPlayfield()->mPockets[p]->CornerPoint());
            // a ray from the player's ball to the pocket
            Geometry::Ray3D los(obj.center, Geometry::Vector3D(Corner.x, Corner.y, Corner.z) - Geometry::Vector3D(obj.center[0], obj.center[1], obj.center[2]));

            // modify the shot score
			float shot_mod = 0;
			for(unsigned int j = 0; j < Game::Get()->GetPlayfield()->mBalls.size(); ++j)
			{
                if(Game::Get()->GetPlayfield()->mBalls[j]->Number() == 0)
                    continue;

				Geometry::Vector3D b = Game::Get()->GetPhysics()->RigidBodyVector3D(Game::Get()->GetPlayfield()->mBalls[j]->ID(), Physics::Engine::eRigidBodyVector::propPosition);
				Geometry::Sphere3D obstruct(Geometry::Point3D(b[0], b[1], b[2]), 2.0);
		        
				// find the aim point
                D3DXVECTOR3 ghost_temp = GhostBall(Game::Get()->GetPlayfield()->mPockets[p], lballs[i]->Number(), AIBALL);
                Geometry::Vector3D g(ghost_temp.x, ghost_temp.y, ghost_temp.z);
				Geometry::Vector3D ghost(g);
				Geometry::Point3D ghostb(ghost[0], ghost[1], ghost[2]);
				Geometry::Ray3D aim(cue, (ghostb - cue));
				
                // don't test a ball against itself
				//if(lballs[i]->Number() == Game::Get()->GetPlayfield()->mBalls[j]->Number())
				//{
                //    continue;
				//}
                //else
				{
					// there is something between the object ball and the pocket
					if(Geometry::Intersects(los, obstruct, 0))
						shot_mod--;//continue;

					Geometry::LineSeg3D seg(cue, ghostb);
					std::pair<Geometry::Point3D, Geometry::Point3D> int_pt; 
					int hits = 0;
					// test if there is an object ball between the cueball and the ghost ball
					if((hits = Geometry::Intersects(aim, obstruct, &int_pt)) != 0)
					{
						if(hits == 1)
							if(seg.contains(int_pt.first))
								shot_mod--;//continue;
						if(seg.contains(int_pt.first) || seg.contains(int_pt.second))
							shot_mod--;//continue;
					}
					
				}
				// we passed all of the tests - we can shoot this ball in straight
				float d1 = convert_distance(max_len, ideal_bp, los.direction.length());
				float d2 = convert_distance(max_len, ideal_bb, aim.direction.length());
				float angle = los.direction.normal() * aim.direction.normal();
				std::vector<float> input(3);
				std::vector<float> output(1);
				input[0] = d1;
				input[1] = d2;
				input[2] = angle;
				if(angle < 0.f)
					shot_mod -= 2;//continue;
				Shot sh;
                output = ANN.Run(input);
				sh.score = output[0];
                Geometry::Vector3D temp_aim(aim.direction.normal());
                D3DXVECTOR3 new_aim(temp_aim[0], temp_aim[1], temp_aim[2]);
				sh.v = new_aim;
				sh.p = ghost_temp;
				shots.push_back(sh);
			}
		}
	}

	std::sort(shots.begin(), shots.end());
	int s = shots.size();

    if(s == 0)
	{
		Shot sht;
		sht.v = D3DXVECTOR3(0,1,0);//Geometry::Vector3D(0,1,0);
		return sht;
	}
	else
	{
        // don't perturb the shot if it not the AI's turn 
        if(Game::Get()->GetSession()->GetPlayer(Game::Get()->GetSession()->CurrentTurn())->IsAI())
		    shots[shots.size() - 1].v = Perturb(shots[shots.size() - 1].v, shots[shots.size() - 1].score);
 
		return shots[shots.size() - 1];//.v;//Geometry::Vector3D();	
	}
}


