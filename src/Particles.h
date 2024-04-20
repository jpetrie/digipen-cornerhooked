/*!
	@file	Particles.h
	@author	Scott Smith
	@date	September 21, 2004

	@brief	Interface to a generalized particle engine.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#ifndef __PARTICLES_H__
#define __PARTICLES_H__

#include "main.h"
#include "quaternion.h"
#include "nsl_random.h"
#include "graphicsrenderer.h"

const int kParticleCount = 100;

struct particle_rule
{
	void operator()(D3DXVECTOR3& pos, D3DXVECTOR3& vel, D3DXVECTOR3& /*orn*/, int frames, int max_frames)
	{
        //vel = D3DXVECTOR3(r.rand_float(), r.rand_float(), r.rand_float());
        D3DXVec3Scale(&vel, &vel, 1.f - static_cast<FLOAT>((float)frames / (float)max_frames));
		pos.x = pos.x + vel.x;
        pos.y = pos.y + vel.y;
        pos.z = pos.z + vel.z;
	}
	//random r;
};


template < class _P, class _BF, int _M >
class Particles
{
public:
    Particles(Graphics::Renderer*);
	virtual ~Particles();
	void	Update(const D3DXVECTOR3&pos, const D3DXVECTOR3& view);			// heavy lifting
	void	Origin(const D3DXVECTOR3& org);
	void	Init(void);

protected:
	struct Particle
	{
        Particle():render_type(0), alive(true), frame(0), max_frames(300), lifetime(max_frames) {}

		void Render(const D3DXVECTOR3& cam_pos, const D3DXVECTOR3& cam_look);

		D3DXVECTOR3          pos;			///< the position vector of the particle
		D3DXVECTOR3          vel;			///< the velocity vector of the particle
		Quaternion			 orient;			///< the orientation of the object if applicable
		_P*					 render_type;	///< the drawable type of the particle
		int					 max_frames;		///< the frame based lifetime of the particle
        int                  frame;
        int                  alive;
        int                 lifetime;
	};

	//std::list< Particle* >   ParticleList;	///< the list of particles to be maintained
    Particle*                ParticleList[_M];
	_BF	                     BehaviorType;	///< a functor that determines the behavior of the system
	const int	             max_count;		///< the maximum number of active particles
   	D3DXVECTOR3              origin;			///< where new particles emit from

private:
    Particles&  operator=(const Particles&);
    Particles(const Particles&);
    Graphics::Renderer      *renderer;
};

#include "particles.inl"

#endif  /* __PARTICLES_H__ */
