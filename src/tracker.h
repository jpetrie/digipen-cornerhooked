#ifndef TRACKER_H
#define TRACKER_H

// includes
#include "main.h"
#include <string>
#include <vector>

// disable warning
#pragma warning(disable: 4512)

// map
template <class T>
struct TrackerMap
{
	TrackerMap( const std::string &p_name, T &p_val )
		:	name( p_name ), val( p_val )	{}

	const std::string	name;
	T					val;
};

// tracker
template <class T>
class Tracker
{
	public:
		// functions
		Tracker( const T &p_default );
		T &get( const std::string p_name );
		T &get( const char *p_name );
		void create( const std::string p_name );
		void create( const char *p_name );
		void remove( const std::string p_name );
		void remove( const char *p_name );

		// inline functions
		inline const TrackerMap<T>
			TrackInfo( unsigned int i )	{	TrackerMap<T> l( mMapS[i], mMapV[i] );	return l;	};
		inline unsigned int
			TrackSize( void )			{	return mMapS.size();		};

	private:
		std::vector< std::string >		mMapS;
		std::vector< T >				mMapV;
		T								mDefault;
};

// constructor
template <class T>
Tracker<T>::Tracker( const T &p_default )
	:	mDefault( p_default )
{}

// create value
template <class T>
void Tracker<T>::create( const char *p_name )
{
	get( p_name );
}

template <class T>
void Tracker<T>::create( const std::string p_name )
{
	get( p_name );
}

// remove value
template <class T>
void Tracker<T>::remove( const char *p_name )
{
	std::string temp( p_name );
	return remove( temp );
}

template <class T>
void Tracker<T>::remove( const std::string p_name )
{
	ASSERT( mMapS.size() == mMapV.size() );
	for( unsigned int i = 0; i < mMapS.size(); ++i )
	{
		if( mMapS[i] == p_name )
		{
			mMapV.remove( i );
			mMapS.remove( i );
			return;
		}
	}
}

// get value
template <class T>
T &Tracker<T>::get( const char *p_name )
{
	std::string temp( p_name );
	return get( temp );
}

template <class T>
T &Tracker<T>::get( const std::string p_name )
{
	// make sure both vectors are of equal size
	ASSERT( mMapS.size() == mMapV.size() );

	// search for value
	for( unsigned int i = 0; i < mMapS.size(); ++i )
	{
		if( mMapS[i] == p_name )
			return mMapV[i];
	}
	// value not found, create it
	mMapS.push_back( p_name );
	mMapV.push_back( mDefault );
	return mMapV[ mMapS.size()-1 ];
}


/*
template <class T>
class Tracker
{
	private:
		std::vector< TrackerMap<T> >	mMap;
		T								mDefault;
};
*/

/*
// get value
template <class T>
T &Tracker<T>::get( const std::string p_name )
{
	// search for value
	std::vector< TrackerMap<T> >::iterator i;
	for( i = mMap.begin(); i != mMap.end(); ++i )
	{
		if( i->name == p_name )
			return i->val;
	}
	// value not found, create it
	TrackerMap<T> newMap( p_name, mDefault );
	mMap.push_back( newMap );
	return mMap[ mMap.size()-1 ].val;
}
*/

#endif