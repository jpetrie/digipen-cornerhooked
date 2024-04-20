/*!
	@file	Particles.inl
	@author	Scott Smith
	@date	September 21, 2004

	@brief	Implements a generalized particle engine.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

// ct
template< class _P, class _BF, int _M >
Particles< _P, _BF, _M >::Particles(Graphics::Renderer* r)
:renderer(r), max_count(_M), origin(0.f, 0.f, 0.f)
{
    origin.x = 0.f;
    origin.y = 0.f;
    origin.z = 0.f;
}

// dt
template< class _P, class _BF, int _M >
Particles< _P, _BF, _M >::~Particles()
{
    for(int i = 0; i < max_count; ++i)
    {
        delete ParticleList[i]->render_type;
        delete ParticleList[i];
    }
}

// render function for the particle type
template< class _P, class _BF, int _M >
void Particles< _P, _BF, _M >::Particle::Render(const D3DXVECTOR3& cam_pos, const D3DXVECTOR3& cam_look)
{
    D3DXVECTOR3 diff = (pos - cam_pos), look;
    D3DXVec3Normalize(&look, &cam_look);
    D3DXVec3Normalize(&diff, &diff);

    if( D3DXVec3Dot(&diff, &look) >= 0.f )
	    render_type->Render(cam_pos, cam_look);
}
// does the updating
template< class _P, class _BF, int _M >
void Particles< _P, _BF, _M >::Update(const D3DXVECTOR3&pos, const D3DXVECTOR3& view)
{
    random r(GetTickCount());
    for(int i = 0; i < max_count; ++i)
	{
		BehaviorType(ParticleList[i]->pos, ParticleList[i]->vel, D3DXVECTOR3(), ParticleList[i]->frame, ParticleList[i]->lifetime);
		ParticleList[i]->frame++;
        if(ParticleList[i]->frame > ParticleList[i]->lifetime)
        {
            ParticleList[i]->pos.x = origin.x;
            ParticleList[i]->pos.y = origin.y;
            ParticleList[i]->pos.z = origin.z;
            ParticleList[i]->vel = D3DXVECTOR3(r.rand_float(-3.f,3.f), r.rand_float(-3.f,3.f), r.rand_float(-3.f,3.f));
            D3DXVec3Normalize(&(ParticleList[i]->vel), &(ParticleList[i]->vel));
            D3DXVec3Scale(&(ParticleList[i]->vel), &(ParticleList[i]->vel), 5.f);
            ParticleList[i]->lifetime = ParticleList[i]->max_frames;//r.rand_int(1, ParticleList[i]->max_frames);
             ParticleList[i]->frame = 0;
        }
        if(ParticleList[i]->alive)
        {
            ParticleList[i]->render_type->SetTranslation(ParticleList[i]->pos.x, ParticleList[i]->pos.y, ParticleList[i]->pos.z);
            D3DXVECTOR3 temp = ParticleList[i]->pos;
            ParticleList[i]->render_type->SetScale(.1f, .1f, .1f);
            ParticleList[i]->render_type->SetRotation(ParticleList[i]->orient.X(), ParticleList[i]->orient.Y(),ParticleList[i]->orient.Z(), ParticleList[i]->orient.W());
		    ParticleList[i]->Render(pos, view);
        }
		
	}
	
}
// set the origin for particle release
template< class _P, class _BF, int _M >
void Particles< _P, _BF, _M >::Origin(const D3DXVECTOR3 &org)
{
	origin = org;
}
// initialize the engine
template< class _P, class _BF, int _M >
void Particles< _P, _BF, _M >::Init(void)
{
	random r;   // a random number generator

    // generate as many particles as the system can hold
    for(int i = 0; i < max_count; ++i)
    {
       Particle* p = new Particle();
        // give it a random velocity
        p->vel = D3DXVECTOR3(r.rand_float(-3.f,3.f), r.rand_float(-3.f,3.f), r.rand_float(-3.f,3.f));
        // start the particle at the system's origin
        p->pos.x = origin.x;
        p->pos.y = origin.y;
        p->pos.z = origin.z;
        // give it a random lifetime
        p->lifetime = p->max_frames;//r.rand_int(1, p->max_frames);
        // normalize the velocity
        D3DXVec3Normalize(&(p->vel), &(p->vel));
        D3DXVec3Scale(&(p->vel), &(p->vel), 5.f);
        // add it to the system
        ParticleList[i] = p;
        //ParticleList.push_back(p);
	}
    for(int i = 0; i < max_count; ++i)
    {
        ParticleList[i]->render_type = new _P(renderer, "data/clear.png", 5, 5);
    }
  

}
