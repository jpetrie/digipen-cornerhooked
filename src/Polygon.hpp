#if !defined( POLYGON_HPP_ )
#define POLYGON_HPP_
////////////////////////////////////////////////////////////////////////////////
/**
@file       Polygon.hpp
@brief      CS300 Advanced Computer Graphics (I)

            This file contains data structures and function prototypes for
            polygons.

@author     
@date       Started:    January     30, 2004 11:21
@date       Modified:   February    05, 2004 21:46
@note       Instructor: Xin Li
@note       Copyright © 2004 DigiPen Institute of Technology
*/
////////////////////////////////////////////////////////////////////////////////
#if !defined( __cplusplus )
#error this header cannot be used outside of a C++ program
#endif

#pragma warning( push, 3 )	// shut up the compiler warning 4702
#pragma warning( disable: 4702 )
#include <vector>
#pragma warning( pop )

#include "Geometry.hpp"

namespace Geometry
{

// Forward declarations:
struct PolyData;
struct ObjColor;

/// Synonym for container for three-dimensional points.
typedef std::vector< Point3D >          VertexTable;
/// Synonym for container for three-dimensional polygons.
typedef std::vector< PolyData >         PolygonTable;

////////////////////////////////////////////////////////////////////////////////
/**
@struct     ObjColor

@brief      Represents a color in the form RGB. Each component is in the range
            [0:1].
*/
////////////////////////////////////////////////////////////////////////////////
struct ObjColor
{
public:
    inline explicit
    ObjColor( f64_t Red = 0, f64_t Green = 0, f64_t Blue = 0, f64_t Alpha = 1 );

public:
    inline
    ObjColor&
    operator+=( const ObjColor& rhs );

    inline
    ObjColor&
    operator-=( const ObjColor& rhs );

    inline
    ObjColor&
    operator*=( const f64_t& rhs );

	ObjColor& operator=( const ObjColor& rhs)
	{
		red = rhs.red;
		green = rhs.green;
		blue = rhs.blue;
		alpha = rhs.alpha;
		return *this;
	}

public:
    /// The red component.
    f64_t                               red;
    /// The green component.
    f64_t                               green;
    /// The blue component.
    f64_t                               blue;
	f64_t								alpha;

};

////////////////////////////////////////////////////////////////////////////////
/**
@struct     PolyData

@brief      Specifies a three-dimensional polygon (triangle.)
*/
////////////////////////////////////////////////////////////////////////////////
struct PolyData
{
public:
    inline
    PolyData( void );

public:
    /// The number of vertices in this polygon. (Fixed at three.)
    enum{ DIM = 3 };
    /// Array of point indices.
    uint32_t                            points[DIM];
    /// The plane that this polygon lies in.
    Plane3D                             plane;
    /// The color of this polygon.
    ObjColor                            color;
    /// A flag that indicates this polygon has been chosen as a root.
    bool                                processed;
	//bool operator<(const PolyData &rhs)const { return color.alpha < rhs.color.alpha; }
	/*PolyData & operator=(const PolyData &rhs)
	{
		points[0] = rhs.points[0];
		points[1] = rhs.points[1];
		points[2] = rhs.points[2];
		plane = rhs.plane;
		color = rhs.color;
		processed = rhs.processed;
		return *this;
	}*/
	bool operator==(const PolyData&p)
	{
		for(int i = 0; i < DIM; ++i)
			if(points[i] != p.points[i])
				return false;
		if(plane[X] != p.plane[X] || plane[Y] != p.plane[Y] || plane[Z] != p.plane[Z])
			return false;
		if(color.red != p.color.red || color.green != p.color.green || color.blue != p.color.blue)
			return false;
		if(processed != p.processed)
			return false;
		return true;
	}
};

// Utility function:
void
DrawPolygonTable( const PolygonTable& PT, const VertexTable& VT,
    bool WireFrame = false );

} // namespace Geometry


namespace Geometry
{

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the red, blue, and green components of the color to be
            specified.

@param      Red The red component.
@param      Green The blue component.
@param      Blue The green component.

@note       The color components are intended to be in the range [0:1].
*/
////////////////////////////////////////////////////////////////////////////////
inline
ObjColor::ObjColor( f64_t Red, f64_t Green, f64_t Blue, f64_t Alpha )
: red( Red ), green( Green ), blue( Blue ), alpha(Alpha)
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (addition) operator. Adds the given color and
            returns the result.

@param      rhs The color to be added.

@return     Returns the result of adding the two colors (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
ObjColor&
ObjColor::operator+=( const ObjColor& rhs )
{
    red                                 += rhs.red;
    green                               += rhs.green;
    blue                                += rhs.blue;
	alpha								+= rhs.alpha;
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (subtraction) operator. Subtracts the given
            color and returns the result.

@param      rhs The color to be subtracted.

@return     Returns the result of subtracting the color (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
ObjColor&
ObjColor::operator-=( const ObjColor& rhs )
{
    red                                 -= rhs.red;
    green                               -= rhs.green;
    blue                                -= rhs.blue;
	alpha								-= rhs.alpha;
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (multiplication) operator. Multiplies the color
            by the given scalar and returns the result.

@param      rhs The scaling factor for the color.

@return     Returns the now-scaled color (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
ObjColor&
ObjColor::operator*=( const f64_t& rhs )
{
    red                                 *= rhs;
    green                               *= rhs;
    blue                                *= rhs;
	alpha								*= rhs;
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Default ctor for polygon.
*/
////////////////////////////////////////////////////////////////////////////////
inline
PolyData::PolyData( void )
: plane(), color(), processed( false )
{
    std::fill_n( points, static_cast< unsigned int >( DIM ), 0 );

    return;
}

} // namespace Geometry

#endif // #if !defined( POLYGON_HPP_ )
