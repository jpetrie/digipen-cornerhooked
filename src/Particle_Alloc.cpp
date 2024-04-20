#include "main.h"
#include <math.h>
#include <vector>
#include "Particle.h"
#include "Geometry.hpp"

/*	--------------------------------------------------------------------------
@brief	initialize a particle set
@param	p_numParticles	# of particles to load
@param	p_img			image to use
@param	p_GenFunc		generating function
@param	p_GrowBucket	if true, will increase the number of buckets
@return	true if particles could be allocated, not neccesarily all
	-------------------------------------------------------------------------- */
bool ParticleSystem::AllocateParticles( unsigned int p_numParticles,
										ParticleImage p_img,
										void (*p_GenFunc)( Particle & ),
										unsigned int /*p_priority*/,
										bool p_GrowBucket )
{
	// we must generate the particle information somehow
	if( !p_GenFunc )
		return false;

	// the draw and physics functions must exist
	if( m_GroupTemplate.PhyFunc == NULL ||
		m_GroupTemplate.DrawFunc == NULL )
		return false;

	// if there is no room for all the particles
	if( p_numParticles > m_RemainingParticles )
	{
		// we will add to available space
		if( p_GrowBucket )
		{
			// add space needed to meet requirements
			const unsigned int needSpace = p_numParticles - m_RemainingParticles;
			const unsigned int addBuckets = ( needSpace + m_BucketSize - 1 ) / m_BucketSize;
			m_Buckets.resize( m_Buckets.size() + addBuckets );

			// insert particles into available space as needed
			const unsigned int Begin = ( m_Buckets.size() - addBuckets ) * m_BucketSize;
			const unsigned int NewSpace = addBuckets * m_BucketSize;
			const unsigned int UseSpace = ( p_numParticles >= NewSpace )?NewSpace:p_numParticles;
			AddParticle( Begin, Begin+UseSpace-1 );
			p_numParticles -= UseSpace;
			// @todo: set particles to active
		}
		else
		{
/*			// find what extra space we earn by removing other lesser particles
			unsigned int extraSpace = 0;
			for( i = m_Groups.begin(); i != m_Groups.end(); ++i )
			{
				if( p_priority > 0 )
					extraSpace += 0;
			}

			// if we can't dump enough particles
			if( m_RemainingParticles + extraSpace < p_numParticles )
				return false;

			// free particles
*/			return false;
		}
	}

	// Try to find large consecutive groups of data to insert into
	while( p_numParticles )
	{
		// find the preferred size of a span
		unsigned int	prefSize = ( p_numParticles > m_BucketSize )?( m_BucketSize ):( p_numParticles );
		ParticleSpan	span = FindLongestSpan( prefSize );
		unsigned int	spanSize = span.end-span.begin+1;

		// add new span
		m_GroupTemplate.AddSpan( span );
		p_numParticles -= spanSize;

		// set new span to active, so longestspan doesn't grab this span again
		for( unsigned int s = span.begin; s <= span.end; ++s )
			m_Buckets[ span.bucket ].Particles[s].Active = true;

		// at some point, we don't want to look for more spans as they become too small
		if( Qsqrt(p_numParticles) > spanSize && Qsqrt(m_BucketSize) > spanSize )
			break;
	}

	// search for particles one at a time to add
	unsigned int count = 0;
	for( BucketList::iterator i = m_Buckets.begin(); i != m_Buckets.end(); ++i, ++count )
	{
		// done searching
		if( !p_numParticles )
			break;

		// search each bucket for free particles
		Particle * p = &( i->Particles[0] );
		for( unsigned int j = 0; j < m_BucketSize; ++j,++p )
		{
			// a free particle, add it
			if( p->Active == false )
			{
				p->Active = true;
				AddParticle( count*m_BucketSize + j );
				--p_numParticles;
			}
			// done searching
			if( !p_numParticles )
				break;
		}
	}

	// generate the particles in the group
	for( unsigned int g = 0; g < m_GroupTemplate.Span.size(); ++g )
	{
		for( unsigned int p = m_GroupTemplate.Span[g].begin; p <= m_GroupTemplate.Span[g].end; ++p )
		{
			unsigned int b = m_GroupTemplate.Span[g].bucket;
			p_GenFunc( m_Buckets[b].Particles[p] );
			--m_RemainingParticles;
		}
	}
	memcpy( &m_GroupTemplate.VisualData, &p_img, sizeof(ParticleImage) );

	// move template group to group list and clear template
	m_Groups.push_back( m_GroupTemplate );
	m_GroupTemplate.Span.clear();
	m_GroupTemplate.numParticles = 0;
	return true;
}

/*	--------------------------------------------------------------------------
@brief	finds the most ideal span
@param	p_prefSpan		ideal length of a span
@param	p_cSpan			current span length
@param	p_mSpan			current maximum span
@param	p_mOvershoot	maximum overshoot
@param	p_BucketSize	bucket size
	-------------------------------------------------------------------------- */
static void LongestSpan_CompareSpans(	const unsigned int &p_prefSpan,
										const ParticleSpan &p_cSpan,
										ParticleSpan &p_mSpan,
										unsigned int &p_mOvershoot,
										const unsigned int p_BucketSize )
{
	// get span sizes and the overshoot
	const unsigned int SpanSize = p_cSpan.end - p_cSpan.begin + 1;
	const unsigned int MaxSize = p_mSpan.end - p_mSpan.begin + 1;
	const unsigned int Overshoot = ( SpanSize > p_prefSpan )?( SpanSize - p_prefSpan ):(0);

	// check if there is a selected bucket at all
	if( p_mSpan.bucket == p_BucketSize )
	{
		// copy data
		memcpy( &p_mSpan, &p_cSpan, sizeof(ParticleSpan) );

		// check if the span is too large
		if( SpanSize > p_prefSpan )
			p_mSpan.end = p_mSpan.begin + p_prefSpan-1;
		p_mOvershoot = Overshoot;
	}

	// check if the new span is larger than requested
	else if( Overshoot > 0 )
	{
		// if the largest span fits perfectly, don't copy
		if( p_mOvershoot == 0 && MaxSize == p_prefSpan )
			return;

		// if the span is larger than the previous span
		if( p_mOvershoot != 0 && Overshoot >= p_mOvershoot )
			return;

		// new span with minimal overshoot
		memcpy( &p_mSpan, &p_cSpan, sizeof(ParticleSpan) );
		p_mSpan.end = p_mSpan.begin + p_prefSpan-1;
		p_mOvershoot = Overshoot;
	}

	// check if this span is larger than the previous record
	else if( SpanSize > MaxSize )
		memcpy( &p_mSpan, &p_cSpan, sizeof(ParticleSpan) );
}

/*	--------------------------------------------------------------------------
@brief	searches through list for a span of the largest size with minimal overshoot
@param	p_prefSpan	ideal span size
@return	Largest span in the list, or the most fitting if the largest span is greater than the preferred span size
	-------------------------------------------------------------------------- */
ParticleSpan ParticleSystem::FindLongestSpan( const unsigned int p_prefSpan ) const
{
	ParticleSpan	cSpan( 0, 0, 0 );
	ParticleSpan	mSpan( 0, 0, m_Buckets.size() );
	unsigned int	mOvershoot;

	Particle *		p;
	unsigned int	j;

	// look through entire list for a span
	for( BucketList::const_iterator i = m_Buckets.begin(); i != m_Buckets.end(); ++i, ++cSpan.bucket )
	{
		// set the span to be 0 initially as a span cannot reach beyond a bucket
		p = &( i->Particles[0] );
		cSpan.begin	= 0;
		cSpan.end	= 0;

		// check every particle in list
		for( j = 0; j < m_BucketSize; ++j,++p )
		{
			// check if there is an end of a span
			if( p->Active == true )
			{
				//check if the span was the longest
				--cSpan.end;
				LongestSpan_CompareSpans( p_prefSpan, cSpan, mSpan, mOvershoot, m_Buckets.size() );
				++cSpan.end;

				// set span to begin at next value
				cSpan.begin = ++cSpan.end;
			}
			else
			{
				// span is still going, continue
				++cSpan.end;
			}
		}

		// the end of this list
		if( i->Particles[m_BucketSize-1].Active == false )
		{
			--cSpan.end;
			LongestSpan_CompareSpans( p_prefSpan, cSpan, mSpan, mOvershoot, m_Buckets.size() );
		}
	}
	return mSpan;
}

/*	--------------------------------------------------------------------------
@brief	insert a particle into a span, or a unique span if one is unavailable
@param	p_id	global ID of a particle
	-------------------------------------------------------------------------- */
void ParticleSystem::AddParticle( const unsigned int p_id )
{
	// if inserting into an empty group
	if( m_GroupTemplate.Span.empty() )
	{
		m_GroupTemplate.AddSpan( p_id, p_id, m_BucketSize );
		return;
	}

	// check if we can insert the particle into an existing group
	unsigned int RelativeID = GetRelativeID(p_id);
	for( SpanList::iterator i = m_GroupTemplate.Span.begin(); i != m_GroupTemplate.Span.end(); ++i )
	{
		if( p_id == i->begin-1 && RelativeID != m_BucketSize-1 )
		{
			// inserting particle in the front of a group
			--( i->begin );
			++m_GroupTemplate.numParticles;
			return;
		}
		else if( p_id == i->end+1 && RelativeID != 0 )
		{
			// inserting particle in the back of a group
			++( i->end );
			++m_GroupTemplate.numParticles;
			return;
		}
	}

	// create a new group
	m_GroupTemplate.AddSpan( p_id, p_id, m_BucketSize );
}

/*	--------------------------------------------------------------------------
@brief	insert a particle span
@param	p_id1	global start ID of a particle
@param	p_id2	global end ID of a particle
	-------------------------------------------------------------------------- */
void ParticleSystem::AddParticle( const unsigned int p_id1, const unsigned int p_id2 )
{
	// if we are adding one particle, just use the other function
	if( p_id1 == p_id2 )
	{
		AddParticle( p_id1 );
		return;
	}

	// if the indicies are reversed, fix that
	if( p_id1 > p_id2 )
	{
		AddParticle( p_id2, p_id1 );
		return;
	}

	// calculate the bucket that the particle is in
	unsigned int Bucket1 = GetBucket(p_id1);
	unsigned int Bucket2 = GetBucket(p_id2);

	// split particles up into different groups
	if( Bucket1 != Bucket2 )
	{
		unsigned int Begin = p_id1;
		unsigned int End = p_id1 + ( m_BucketSize-1 - (p_id1-Bucket1*m_BucketSize) );

		while( End < p_id2 )
		{
			AddParticle( Begin, End );
			Begin = ++End;
			End += m_BucketSize-1;
		}

		AddParticle( Begin, p_id2 );
	}

	// create a new group
	m_GroupTemplate.AddSpan( p_id1, p_id2, m_BucketSize );
}
