#include "main.h"
#include <vector>
#include "Particle.h"
#include "Geometry.hpp"

/*	--------------------------------------------------------------------------
@brief	initialize a particle set
@param	p_BucketSize	size of each bucket, keep this value large
@param	p_numBuckets	number of buckets to use, keep this value small
	-------------------------------------------------------------------------- */
ParticleSystem::ParticleSystem( unsigned int p_BucketSize, unsigned int p_numBuckets )
	:	m_BucketSize( p_BucketSize ),
		m_RemainingParticles( p_numBuckets * p_BucketSize )
{
	// resize the list
	m_Buckets.resize( p_numBuckets );
	for( BucketList::iterator i = m_Buckets.begin(); i != m_Buckets.end(); ++i )
		i->Init( m_BucketSize );

	// clear out the template group
	m_Groups.clear();
	ResetGroupTemplate();
}

/*	--------------------------------------------------------------------------
@brief	uninitialize a particle set
	-------------------------------------------------------------------------- */
ParticleSystem::~ParticleSystem( void )
{
	// free up the groups and buckets
	m_Buckets.clear();
	m_Groups.clear();
	ResetGroupTemplate();
}

/*	--------------------------------------------------------------------------
@brief	set functions for a particle group
@param	p_PhyFunc	function to call to apply physics
@param	p_DrawFunc	function to call to draw
@param	p_miscData	pointer to any misc data needed for functions
	-------------------------------------------------------------------------- */
void ParticleSystem::Allocate_SetFunction(	void (* p_PhyFunc)( Particle *, const float &, const void * ),
											void (* p_DrawFunc)( const Particle *, const ParticleImage &, void * ),
											void *p_miscData )
{
	m_GroupTemplate.PhyFunc		= p_PhyFunc;	//  the function to apply physics to the particle
	m_GroupTemplate.DrawFunc	= p_DrawFunc;	//	the function to draw the particle
	m_GroupTemplate.MiscData	= p_miscData;	//	a reference to a struct or variable that the functions may use
}

/*	--------------------------------------------------------------------------
@brief	set flags for use in phyiscs and draw functions
@param	p_flags	flags the functions may use
	-------------------------------------------------------------------------- */
void ParticleSystem::Allocate_SetFlags( unsigned int p_flags )
{
	m_GroupTemplate.Flags = p_flags;
}

/*	--------------------------------------------------------------------------
@brief	clean up the particle groups, for faster runtime speeds
	-------------------------------------------------------------------------- */
//@param	p_numchecks	how many checks to preform
void ParticleSystem::AttemptMaintenance( unsigned int /*p_numchecks*/ )
{
	//
}

/*	--------------------------------------------------------------------------
@brief	draw and move all particles
@param	dt	delta time in ms, 1 ms = 1
	-------------------------------------------------------------------------- */
void ParticleSystem::Process( unsigned int dt )
{
	const float		deltaT = dt / 1000.0f;
	unsigned int	i,j,k;
	ParticleSpan *	s;
	Particle *		p;

	// for each particle set
	unsigned int GroupSize = m_Groups.size();
	for( i = 0; i < GroupSize; ++i )
	{
		// for each group in the list
		const unsigned int spanSize = m_Groups[i].Span.size();
		for( j = 0; j < spanSize; ++j )
		{
			// for each span in the group
			s = &m_Groups[i].Span[j];
			for( k = s->begin; k < 1+s->end; ++k )	// NOTE: do not use <=, due to how it works, 0 < -1 when unsigned
			{
				// get the particle
				p = &m_Buckets[s->bucket].Particles[k];
				if( !p )
					continue;

				// let the particle move/draw/etc.
				m_Groups[i].PhyFunc( p, deltaT, m_Groups[i].MiscData );
				m_Groups[i].DrawFunc( p, m_Groups[i].VisualData, m_Groups[i].MiscData ); 

				// adjust the lifetime of the particle
				if( dt >= p->Lifetime )
				{
					// adjust the extended lifetime of the particle
					if( dt >= p->Lifetime + p->LifetimeExtended )
					{
						// the particle is pronounced dead
						p->LifetimeExtended = 0;

						// if this isn't the last particle in the list
						if( k != s->end )
						{
							// move the last particle into this position and decrease list size
							memcpy( p, &m_Buckets[s->bucket].Particles[s->end], sizeof(Particle) );
							--k;
						}

						// free up last particle
						m_Buckets[s->bucket].Particles[s->end].Active = false;
						++m_Buckets[s->bucket].FreeParticles;
						++m_RemainingParticles;
						--m_Groups[i].numParticles;
						--( s->end );
					}
					else
					{
						// the particle is suffocating
						p->LifetimeExtended -= ( dt - p->Lifetime );
//						p->Alpha *= sqrt( p->Alpha );
					}

					// the particle had a heart attack
					p->Lifetime = 0;
				}
				else
				{
					// the particle's time is running out
					p->Lifetime -= dt;
				}
			}
		}
	}

	// remove the front particle groups if they contain no active particles
	while(	m_Groups.begin() != m_Groups.end() && m_Groups.begin()->numParticles == 0 )
		m_Groups.erase( m_Groups.begin() );
}

/*	--------------------------------------------------------------------------
@brief	clear the template that the particle group will use for a base
	-------------------------------------------------------------------------- */
void ParticleSystem::ResetGroupTemplate( void )
{
	// reset the group template
	Allocate_SetFunction();
	Allocate_SetFlags();

	// remove all particles references in group template
	m_GroupTemplate.Span.clear();
	m_GroupTemplate.numParticles = 0;
}

/*	--------------------------------------------------------------------------
@brief	a quick implementation of integer square-root
@param	v	the number
@return	v = result * result, with some(or much) loss of accuracy
	-------------------------------------------------------------------------- */
unsigned int ParticleSystem::Qsqrt( const unsigned int v ) const
{
	unsigned int c = 0;
	unsigned int b = 1 << ( sizeof(c)*4 - 1 );

	do	{
		c |= b;
		if( c*c > v )
			c &= ~b;
		b >>= 1;
	}	while( b );
	return c;
}