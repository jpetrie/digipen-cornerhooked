/*!
	@file		AIPlayer.h
	@author		Scott
	@date		07-14-2004
	@brief		AI player functions. 
*//*__________________________________________________________________________*/

#ifndef __AIPLAYER_H__
#define __AIPLAYER_H__

#include "main.h"

#include "game.h"
#include "ann.h"
#include "player.h"

/*!
	@namespace	AI
	@author		Scott
	@date		07-15-2004
	@ingroup	ch
	@brief		Contains data structures and functions related to AI.
*//*__________________________________________________________________________*/

	enum GHOST_BALL{REAL = 0, AIBALL = 1};

	const float ideal_bb = 36.f / 2.25;
	const float ideal_bp = 24.f / 2.25f;
	const float max_len  = sqrt(float((100*100) + (50*50) + (25*25)));
	
	class AIPlayer : public Player
	{
	public:
		AIPlayer(const std::string BPN_file, const int turn):ANN(BPN_file), TurnID(turn), FirstShot(true) {}
		
		virtual bool IsAI(void) const { return (true); }
		virtual Shot SelectShot(void);
		
		int					TurnID;
		AI::NeuralNet		ANN;
		bool				FirstShot;
	};

	float convert_distance(float max, float ideal, float d);
    D3DXVECTOR3 GhostBall(const Pocket p, int ball, GHOST_BALL type);
	
#endif
