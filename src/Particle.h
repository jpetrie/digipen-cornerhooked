#ifndef PARTICLE_H
#define PARTICLE_H

#include "main.h"
#include <windows.h>
#include <assert.h>
#include <vector>
#include "Geometry.hpp"

#include "GraphicsRenderer.h"

/*
#define FLAG01	( 1 << 00 )
#define FLAG02	( 1 << 01 )
#define FLAG03	( 1 << 02 )
#define FLAG04	( 1 << 03 )
#define FLAG05	( 1 << 04 )
#define FLAG06	( 1 << 05 )
#define FLAG07	( 1 << 06 )
#define FLAG08	( 1 << 07 )
#define FLAG09	( 1 << 08 )
#define FLAG10	( 1 << 09 )
#define FLAG11	( 1 << 10 )
#define FLAG12	( 1 << 11 )
#define FLAG13	( 1 << 12 )
#define FLAG14	( 1 << 13 )
#define FLAG15	( 1 << 14 )
#define FLAG16	( 1 << 15 )
#define FLAG17	( 1 << 16 )
#define FLAG18	( 1 << 17 )
#define FLAG19	( 1 << 18 )
#define FLAG20	( 1 << 19 )
#define FLAG21	( 1 << 20 )
#define FLAG22	( 1 << 21 )
#define FLAG23	( 1 << 22 )
#define FLAG24	( 1 << 23 )
#define FLAG25	( 1 << 24 )
#define FLAG26	( 1 << 25 )
#define FLAG27	( 1 << 26 )
#define FLAG28	( 1 << 27 )
#define FLAG29	( 1 << 28 )
#define FLAG30	( 1 << 29 )
#define FLAG31	( 1 << 30 )
#define FLAG32	( 1 << 31 )
*/

typedef		Geometry::Point2D	ParticleImage;

/*	--------------------------------------------------------------------------

	-------------------------------------------------------------------------- */
struct Particle
{
	Particle()	:	Active(false) {};

	Geometry::Point3D	Position;
	Geometry::Vector3D	Velocity;
	Geometry::Vector3D	Acceleration;
	double		RotationDegree;
	double		RotationVelocity;
	double		RotationAcceleration;
	double		Gravity[3];
	double		Lifetime;
	double		LifetimeExtended;
	double		Alpha;
	bool		Active;
};

/*	--------------------------------------------------------------------------

	-------------------------------------------------------------------------- */
struct ParticleSpan
{
	ParticleSpan( void )					{};
	ParticleSpan( const unsigned int p1, const unsigned int p2, const unsigned int b )
		:	begin(p1), end(p2), bucket(b)	{};
	unsigned int	begin,end,bucket;
};
typedef		std::vector<ParticleSpan>		SpanList;

struct ParticleGroup
{
	ParticleGroup()
		:	numParticles( 0 ),
			Flags( 0 ),
			PhyFunc( NULL ),
			DrawFunc( NULL )
	{
		Span.clear();
	}

	~ParticleGroup()
	{
		Span.clear();
	}

	void AddSpan( const ParticleSpan p )
	{
		Span.push_back( p );
		numParticles += p.end - p.begin + 1;
	};

	void AddSpan( const unsigned int p1, const unsigned int p2, const unsigned int s )
	{
		assert( p1 <= p2 );
		Span.push_back( ParticleSpan( p1-(p1/s)*s, p2-(p2/s)*s, p1/s ) );
		numParticles += p2 - p1 + 1;
	};

	SpanList		Span;
	unsigned int	numParticles;
	unsigned int 	Flags;

	void			(* PhyFunc)( Particle *p, const float &dt, const void *miscData );
	void			(* DrawFunc)( const Particle *p, const ParticleImage &img, void *miscData );
	void *			MiscData;	// a pointer to data that the functions may use for additional reference
	ParticleImage	VisualData;
};

/*	--------------------------------------------------------------------------

	-------------------------------------------------------------------------- */
struct ParticleBucket
{
	ParticleBucket()
		:	Particles( NULL ),
			FreeParticles( 0 )
	{
		//		
	}
			
	~ParticleBucket()
	{
		if( Particles )
			delete []Particles;
		FreeParticles = 0;
	}

	void Init( const unsigned int p_size )
	{
		if( !Particles )
		{
			Particles = new Particle[p_size];
			FreeParticles = p_size;
		}
	}

	Particle *		Particles;
	unsigned int	FreeParticles;
};

/*	--------------------------------------------------------------------------

	-------------------------------------------------------------------------- */

typedef		std::vector<ParticleBucket>		BucketList;
typedef		std::vector<ParticleGroup>		GroupList;

class ParticleSystem
{
	public:
		ParticleSystem( unsigned int p_BucketSize = 64, unsigned int p_numBuckets = 4 );
		~ParticleSystem( void );

		// Particle Allocation
		void Allocate_SetFunction(	void (* p_PhyFunc)( Particle *, const float &, const void * ) = NULL,
									void (* p_DrawFunc)( const Particle *, const ParticleImage &, void * ) = NULL,
									void *p_miscData = NULL );
		void Allocate_SetFlags( unsigned int p_flags = 0 );
		bool AllocateParticles(	unsigned int p_numParticles,
								ParticleImage p_img,
								void (*p_GenFunc)( Particle & ),
								unsigned int p_priority = 0,
								bool p_GrowBucket = false );

		// Particle Misc
		void Process( unsigned int dt );
		void AttemptMaintenance( unsigned int p_numchecks = 100 );

		inline const GroupList *	DEBUG_GetGroups() const
		{	return &m_Groups;	};
		inline const BucketList *	DEBUG_GetBucket() const
		{	return &m_Buckets;	};
		inline unsigned int DEBUG_ParticlesInUse( void ) const
		{	return m_BucketSize * m_Buckets.size() - m_RemainingParticles;	};

	private:
		// variables
		BucketList				m_Buckets;
		GroupList				m_Groups;
		const unsigned int		m_BucketSize;			// size of each bucket
		unsigned int			m_RemainingParticles;
		Particle				m_TempParticle;
		ParticleGroup			m_GroupTemplate;

		// Functions
		void ResetGroupTemplate( void );
		void AddParticle( const unsigned int p_id );
		void AddParticle( const unsigned int p_id1, const unsigned int p_id2 );
		ParticleSpan FindLongestSpan( const unsigned int p_prefSpan ) const;
		unsigned int Qsqrt( const unsigned int v ) const;

		// Inline Functions
		inline Particle *GetParticle( const unsigned int p_id )
		{
			if( p_id >= m_BucketSize * m_Buckets.size() )
				return NULL;
			unsigned int index = p_id/m_BucketSize;
			return &( m_Buckets[index].Particles[ p_id - index*m_BucketSize ] );
		};
		inline unsigned int GetBucket( const unsigned int p_id )
		{
			return p_id / m_BucketSize;
		};
		inline unsigned int GetRelativeID( const unsigned int p_id )
		{
			return p_id - GetBucket(p_id)*m_BucketSize;
		};
        ParticleSystem& operator=(const ParticleSystem&);
};

/*
Flags:
	0 - Particle Activated
	1 - Gravity on
	2 - Point Gravity
	3 - 

	1 - rotate particles
	1+2 - { 0=gravity off,1=gravity on,2=rotation on, 3=
	2 - point gravity
	3 - alpha-fade out
*/



class Game;
void CreatePExplosion(	ParticleSystem *p_part, Geometry::Point3D p_origin, unsigned int p_num );
void CreatePMarker(		ParticleSystem *p_part, Geometry::Point3D p_origin );
void CreatePTrail(		ParticleSystem *p_part, Geometry::Point3D p_origin, Geometry::Vector3D p_dir );
void CreatePShockwave(	ParticleSystem *p_part, Geometry::Point3D p_origin, Geometry::Vector3D p_dir, const double Speed );
void ParticleTrail();
void SetParticleSphere();
void UnsetParticleSphere();

#endif