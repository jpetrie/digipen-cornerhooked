#include "main.h"
#include "Game.h"
#include "Particle.h"
#include "Geometry.hpp"
#include "DXSphere.h"
#include "GraphicsRenderer.h"

// Sample Particle Usage
void DrawFunc0( const Particle *P, const ParticleImage &img, const void *miscData );
void DrawFunc1( const Particle *P, const ParticleImage &img, const void *miscData );
void DrawFunc2( const Particle *P, const ParticleImage &img, const void *miscData );
void DrawFunc3( const Particle *P, const ParticleImage &img, const void *miscData );
void NoMovement( Particle *P, const float &dt, const void *miscData );
void EulerMovement( Particle *P, const float &dt, const void *miscData );
void RandomMovement( Particle *P, const float &dt, const void *miscData );
void SetMarker( Particle &P );
void Generate( Particle &P );
void GenerateParticleExplosion( Particle &P );

// Global values so things can be referenced in generating functions
DXSphere *g_sphere1 = NULL;
DXSphere *g_sphere2 = NULL;
Geometry::Point3D	g_Origin;
Geometry::Vector3D	g_Direct;
Geometry::Matrix16	g_Matrix;

// ---------------------------------------------------------------------------
//	Like rand( int, int ), only uses doubles, should attempt to use small values
// ---------------------------------------------------------------------------
static f64_t ParticleRandom( const double a, const double b )
{
	return static_cast<f64_t>(
		( static_cast<double>(rand()&0x0000FFFF) / 65535.0 ) * (b-a) + a );
}

// ---------------------------------------------------------------------------
//	PHYSICS FUNCTIONS
// ---------------------------------------------------------------------------
void NoMovement( Particle * /*P*/, const float & /*dt*/, const void * /*miscData*/ )
{
	// nothing
}

void EulerMovement( Particle *P, const float &dt, const void * /*miscData */)
{
	// Basic movement
	P->Velocity += P->Acceleration * dt;
	P->Position += P->Velocity * dt;
}

void RandomMovement( Particle *P, const float &dt, const void * /*miscData */)
{
	// Random movements along with a constant velocity, suitable for snow or object trails
	const Geometry::Vector3D v(	static_cast<f64_t>( ((rand()&0xFE)-127)/508.0 + P->Velocity[0] ),
								static_cast<f64_t>( ((rand()&0xFE)-127)/508.0 + P->Velocity[1] ),
								static_cast<f64_t>( ((rand()&0xFE)-127)/508.0 + P->Velocity[2] ) );
	P->Position += v * dt;
}

// ---------------------------------------------------------------------------
//	GENERATING FUNCTIONS
// ---------------------------------------------------------------------------

void SetMarker( Particle &P )
{
	P.Position = Geometry::Point3D( g_Origin[0], g_Origin[1], g_Origin[2] );
	P.Lifetime = 15000; //  15.0 sec
}

void Generate( Particle &P )
{
	double	theta = ( rand() % 361 ) * 2 * 3.14159 / 360;
	f64_t	x = f64_t( 100 * cos(theta) ),
			y = f64_t( (rand()%11) - 5 ),
			z = f64_t( 100 * sin(theta) ),
			v = f64_t( 10 + (rand()%5)*10 ),
			a = f64_t( 0 + (rand()%5)*10 );
	P.Position = Geometry::Point3D( x , y, z );
	P.Velocity = Geometry::Vector3D( 0.0, v, 0.0 );
	P.Acceleration = Geometry::Vector3D( 0.0, -a, 0.0 );
	P.Lifetime = 4000; //  4.0 sec
	P.LifetimeExtended = 1000; // 1.0 sec
	P.Alpha = P.LifetimeExtended;
}

void GenerateParticleExplosion( Particle &P )
{
	f64_t	vx = f64_t(( (rand()%51)-25 )/2.0),
			vy = f64_t(( (rand()%51)-25 )/2.0),
			vz = f64_t(( (rand()%51)-25 )/2.0);
	P.Position = Geometry::Point3D( g_Origin[0], g_Origin[1], g_Origin[2] );

	// aim towards the center
	if( !( g_Origin[0] > 0 && vx > 0 ) || ( g_Origin[0] < 0 && vx < 0 ) )
		vx = -vx;
	if( !( g_Origin[1] > 0 && vy > 0 ) || ( g_Origin[1] < 0 && vy < 0 ) )
		vy = -vy;
	if( !( g_Origin[2] > 0 && vz > 0 ) || ( g_Origin[2] < 0 && vz < 0 ) )
		vz = -vz;

	P.Velocity = Geometry::Vector3D( vx, vy, vz );
	P.Acceleration = Geometry::Vector3D( 0.0, 0.0, 0.0 );
	P.Lifetime = 1500 + (rand()%1001);
	P.LifetimeExtended = 0;
}

void GenerateTrail( Particle &P )
{
	P.Position = Geometry::Point3D( g_Origin );
	P.Velocity = Geometry::Vector3D( g_Direct );
	P.Acceleration = Geometry::Vector3D( 0.0, 0.0, 0.0 );
	P.Lifetime = 500;
	P.LifetimeExtended = 0;
}

void GenerateShockwave( Particle &P )
{
	P.Position = Geometry::Point3D( g_Origin );
	P.Velocity = Geometry::Vector3D( g_Direct );
	P.Acceleration = Geometry::Vector3D( 0.0, 0.0, 0.0 );
	P.Lifetime = 1000;
	P.LifetimeExtended = 0;

	// rotate vector
	g_Direct = g_Matrix * g_Direct;
}

// ---------------------------------------------------------------------------
//	DRAWING FUNCTIONS
// ---------------------------------------------------------------------------
// draw trail+marker
void DrawFunc0( const Particle *P, const ParticleImage &/*img*/, void *miscData )
{
	// physical properties
	g_sphere1->SetTranslation( P->Position[0], P->Position[1], P->Position[2] );
	g_sphere1->SetScale( 0.2f, 0.2f, 0.2f );

	// draw properties
	D3DXVECTOR3	pos, vec;
	static_cast<Game *>( miscData )->GetCamera()->GetPosVector( &pos );
	static_cast<Game *>( miscData )->GetCamera()->GetTargetVector( &vec );
	g_sphere1->UseShader("BallPartV11P11");
	g_sphere1->Render( pos, vec );
}

// not used
void DrawFunc1( const Particle *P, const ParticleImage &/*img*/, void *miscData )
{
	// physical properties
	float s = static_cast<float>( P->LifetimeExtended / P->Alpha );
	g_sphere1->SetTranslation( P->Position[0], P->Position[1], P->Position[2] );
	g_sphere1->SetScale( s, s, s );

	// draw properties
	D3DXVECTOR3	pos, vec;
	static_cast<Game *>( miscData )->GetCamera()->GetPosVector( &pos );
	static_cast<Game *>( miscData )->GetCamera()->GetTargetVector( &vec );
	g_sphere1->UseShader("BallPartV11P11");
	g_sphere1->Render( pos, vec );
}

// shockwave
void DrawFunc2( const Particle *P, const ParticleImage &/*img*/, void *miscData )
{
	// physical properties
	float s = static_cast<float>( P->Lifetime + 1000 ) / 5000.0f;
	g_sphere1->SetTranslation( P->Position[0], P->Position[1], P->Position[2] );
	g_sphere1->SetScale( s, s, s );

	// draw properties
	D3DXVECTOR3	pos, vec;
	static_cast<Game *>( miscData )->GetCamera()->GetPosVector( &pos );
	static_cast<Game *>( miscData )->GetCamera()->GetTargetVector( &vec );
	g_sphere1->Render( pos, vec );
}

// explosion
void DrawFunc3( const Particle *P, const ParticleImage &/*img*/, void *miscData )
{
	// physical properties
	float s = static_cast<float>( P->Lifetime + 1000 ) / 3000.0f;
	g_sphere2->SetTranslation( P->Position[0], P->Position[1], P->Position[2] );
	g_sphere2->SetScale( s*s, s*s, s*s );

	// draw properties
	D3DXVECTOR3	pos, vec;
	static_cast<Game *>( miscData )->GetCamera()->GetPosVector( &pos );
	static_cast<Game *>( miscData )->GetCamera()->GetTargetVector( &vec );
	g_sphere2->Render( pos, vec );
}



// ---------------------------------------------------------------------------
//	PRIMARY FUNCTIONS
// ---------------------------------------------------------------------------
// Drop a particle, doesn't move or anything but instead draws only
void CreatePMarker( ParticleSystem *p_part, Geometry::Point3D p_origin )
{
	g_Origin = p_origin;
	ParticleImage	NULLItem;
	p_part->Allocate_SetFunction( &NoMovement, &DrawFunc0, static_cast<void*>(Game::Get()) );
	p_part->AllocateParticles( 1, NULLItem, &SetMarker );
}

// Creates a set of particles to simulate something shattering
void CreatePExplosion( ParticleSystem *p_part, Geometry::Point3D p_origin, unsigned int p_num )
{
	g_Origin = p_origin;
	ParticleImage	NULLItem;
	p_part->Allocate_SetFunction( &EulerMovement, &DrawFunc3, static_cast<void*>(Game::Get()) );
	p_part->AllocateParticles( p_num, NULLItem, &GenerateParticleExplosion );
}

// Creates a particle that loosely has the original object's movement properties
void CreatePTrail( ParticleSystem *p_part, Geometry::Point3D p_origin, Geometry::Vector3D p_dir )
{
	const Geometry::Vector3D NormDir = p_dir * ( (f64_t)(1.0) / p_dir.length() );
	g_Origin = p_origin - NormDir +
		Geometry::Vector3D( ParticleRandom(-1.0, 1.0), ParticleRandom(-1.0, 1.0), ParticleRandom(-1.0, 1.0) );
	g_Direct = p_dir * (f64_t)(0.5);
	ParticleImage	NULLItem;
	p_part->Allocate_SetFunction( &RandomMovement, &DrawFunc0, static_cast<void*>(Game::Get()) );
	p_part->AllocateParticles( 1, NULLItem, &GenerateTrail );
}

static const unsigned int	particleShockDetail = 16;
static const double particledTheta = 2.0 * 3.14159 / (double)particleShockDetail;
static const double particleC = cos( particledTheta );
static const double particleS = sin( particledTheta );
static const double particleT = 1 - particleC;

// creates a shockwave effect, uses velocity as a normal for a plane
void CreatePShockwave( ParticleSystem *p_part, Geometry::Point3D p_origin, Geometry::Vector3D p_dir, const double Speed )
{
	// set references
	g_Origin = p_origin;

	// normalize
	f64_t l = p_dir.length();
	p_dir[0] /= l;
	p_dir[1] /= l;
	p_dir[2] /= l;

	do	{
		// project a random point onto the plane
		g_Direct = Geometry::Vector3D(	rand()/static_cast<f64_t>(256.0),
										rand()/static_cast<f64_t>(256.0),
										rand()/static_cast<f64_t>(256.0) );
		g_Direct -= p_dir * Geometry::Dot( p_dir, g_Direct );
	}	while( g_Direct == Geometry::Vector3D(0.0,0.0,0.0) );

	// normalize vector towards point
	l = static_cast<f64_t>(Speed) / g_Direct.length();
	g_Direct[0] *= l;
	g_Direct[1] *= l;
	g_Direct[2] *= l;

	// get variables
	const f64_t xx = p_dir[0] * p_dir[0];
	const f64_t xy = p_dir[0] * p_dir[1];
	const f64_t xz = p_dir[0] * p_dir[2];
	const f64_t yy = p_dir[1] * p_dir[1];
	const f64_t yz = p_dir[1] * p_dir[2];
	const f64_t zz = p_dir[2] * p_dir[2];

	// calculate rotation
	g_Matrix[0] = static_cast<const f64_t>( particleT * xx + particleC );
	g_Matrix[1] = static_cast<const f64_t>( particleT * xy + particleS * p_dir[2] );
	g_Matrix[2] = static_cast<const f64_t>( particleT * xz - particleS * p_dir[1] );
	g_Matrix[3] = 0.0;
	g_Matrix[4] = static_cast<const f64_t>( particleT * xy - particleS * p_dir[2] );
	g_Matrix[5] = static_cast<const f64_t>( particleT * yy + particleC );
	g_Matrix[6] = static_cast<const f64_t>( particleT * yz + particleS * p_dir[0] );
	g_Matrix[7] = 0.0;
	g_Matrix[8] = static_cast<const f64_t>( particleT * xz + particleS * p_dir[1] );
	g_Matrix[9] = static_cast<const f64_t>( particleT * yz - particleS * p_dir[0] );
	g_Matrix[10] = static_cast<const f64_t>( particleT * zz + particleC );
	g_Matrix[11] = 0.0;
	g_Matrix[12] = 0.0;
	g_Matrix[13] = 0.0;
	g_Matrix[14] = 0.0;
	g_Matrix[15] = 1.0;

	// finish up
	ParticleImage	NULLItem;
	p_part->Allocate_SetFunction( &EulerMovement, &DrawFunc2, static_cast<void*>(Game::Get()) );
	p_part->AllocateParticles( particleShockDetail, NULLItem, &GenerateShockwave );
}

void SetParticleSphere( void )
{
	if( !g_sphere1 )
	{
		// particle trail / marker / shockwave
		g_sphere1 = new DXSphere( Game::Get()->GetWindow()->GetRenderer(), "", 2, 2 );
	  g_sphere1->UseShader("BallPartV11P11");
	}
	if( !g_sphere2 )
	{
		// particle explosion
		g_sphere2 = new DXSphere( Game::Get()->GetWindow()->GetRenderer(), "", 3, 4 );
//	  g_sphere2->UseShader("BallExploV11P11");
	}
}

void UnsetParticleSphere( void )
{
	if( g_sphere1 )
		delete g_sphere1;
	if( g_sphere2 )
		delete g_sphere2;
}

// ---------------------------------------------------------------------------
//	Particle Trail Function
// ---------------------------------------------------------------------------
void ParticleTrail()
{
	const	static Geometry::Point3D	ori(  0.0, 0.0, 0.0 );
	const	static Geometry::Vector3D	stop( 0.0, 0.0, 0.0 );
			static unsigned int			it;
			static int					id;
			static Physics::Engine *	pe;
			static Playfield *			pf;
			static Geometry::Point3D	pos;
			static Geometry::Vector3D	vel;
			static Game *				g;
			static f64_t				v;

	// assign variables
	g = Game::Get();
	pe = g->GetPhysics();
	pf = g->GetPlayfield();
	// for each ball
	for( it = 0; it < pf->mBalls.size(); ++it )
	{
		// get id
		id = pf->mBalls[it]->ID();

		// get velocity
		vel = pe->RigidBodyVector3D( id, Physics::Engine::eRigidBodyVector::propVeloctity );
		
		// check if we can generate particle
		v = 16*vel.length();
		if( vel != stop && v > double(rand()&0xFF) )
		{
			// get position
			pos = ori + pe->RigidBodyVector3D( id, Physics::Engine::eRigidBodyVector::propPosition );
			// create particle(s)
			while( v > 64 )
			{
                CreatePTrail( g->GetParticles(), pos, vel );
				v -= 64;
			}
		}
	}
}