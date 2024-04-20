/*!
	@file    Player.h
	@author  Scott
	@date    07-05-2004
	@brief   Interface to player class.

	(c) 2004 DigiPen (USA) Corporation, all rights reserved.
*//*__________________________________________________________________________*/

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "main.h"

struct Shot
//! 
{
	D3DXVECTOR3  v;
	D3DXVECTOR3  p;
	float        score;
	
	bool operator<(const Shot&rhs) { return (this->score < rhs.score); }
};

class Player
{
public:
  Player(void);
  virtual ~Player(void);
  
  virtual bool IsAI(void) const { return (false); }
  
	virtual Shot SelectShot(void);
    Shot GetBestShot(void);
    void SetBestShot(const Shot& shot);
	
	std::string GetName(void) const           { return (mName); }
	void        SetName(const std::string &n) { mName = n; }
  
private:
    std::string  mName;  ///< Player's name.
    Shot bestShot;
	bool tookShot;
	std::vector< uint32_t > LegalBalls;
};

#endif  /* _PLAYER_H_ */