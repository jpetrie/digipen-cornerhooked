#if !defined( GEOMETRY_HPP_ )
#define GEOMETRY_HPP_
////////////////////////////////////////////////////////////////////////////////
/**
@file       Geometry.hpp
@brief      CS300 Advanced Computer Graphics (I)

            This file contains geometric data structures and function
            prototypes used in geometrical calculus, such as vector, line,
            plane, etc.

@author     
@date       Started:    January     07, 2004 15:38
@date       Modified:   February    06, 2004 18:21
@note       Instructor: Xin Li
@note       Copyright © 2004 DigiPen Institute of Technology
*/
////////////////////////////////////////////////////////////////////////////////
#if !defined( __cplusplus )
#error this header cannot be used outside of a C++ program
#endif

#include "main.h"

#include <algorithm> // transform
#include <cmath> // sqrt
#include <functional> // bind2nd, plus, minus, etc.
#include <iostream>
#include <numeric> // inner_product
#include <sstream>
#include <utility> // pair
#include "StdTypes.h"
#include "GeometryExceptions.hpp"

// VC7 warning: "C++ exception specification ignored..."
#if defined( _MSC_VER ) && ( 1310 >= _MSC_VER )
#pragma warning( push )
#pragma warning( disable: 4290 )
#endif

////////////////////////////////////////////////////////////////////////////////
/**
@namespace  Geometry

@brief      Contains the types and functions that make up the Geometry Library.
*/
////////////////////////////////////////////////////////////////////////////////
namespace Geometry
{

// Constants:
/// The value of pi (taking straight from calc.exe.)
const f64_t     PI                      = 3.1415926535897932384626433832795f;

// Forward declarations:
class Vector2D;
class Vector3D;
struct Line3D;
struct Box3D;
struct Sphere3D;
class Plane3D;
class Triangle3D;
class Vector4D;

////////////////////////////////////////////////////////////////////////////////
/**
@enum       ComponentIndex

@brief      Defines constants that can be used as indices for the Geometry
            Library.
*/
////////////////////////////////////////////////////////////////////////////////
enum ComponentIndex
{
    X,
    Y,
    Z,
    W,
    A                                   = X,
    B                                   = Y,
    C                                   = Z,
    D                                   = W,
};


////////////////////////////////////////////////////////////////////////////////
/**
@class      Point2D

@brief      Type defining a two-dimensional point.
*/
////////////////////////////////////////////////////////////////////////////////
class Point2D
{
public:
    /// Type for sequential access.
    typedef f64_t                       *iterator;
    /// Type for sequential access of const object.
    typedef const f64_t                 *const_iterator;

public:
    // Ctor.
    inline explicit
    Point2D( f64_t X = 0, f64_t Y = 0 );

public:
    // Arithmetic operators.
    inline
    Point2D&
    operator+=( const Vector2D& rhs );

    inline
    Point2D&
    operator-=( const Vector2D& rhs );

    // Random-access operators.
    inline
    f64_t&
    operator[]( uint32_t i );

    inline
    const f64_t&
    operator[]( uint32_t i ) const;

public:
    // Sequential-access methods.
    inline
    iterator
    begin( void );

    inline
    const_iterator
    begin( void ) const;

    inline
    iterator
    end( void );

    inline
    const_iterator
    end( void ) const;

private:
    /// Dimension of Point.
    enum{ DIM = 2 };
    /// Array of components.
    f64_t                               m_components[DIM];

};

// (Global) operators.
inline
bool
operator==( const Point2D& lhs, const Point2D& rhs );

inline
bool
operator!=( const Point2D& lhs, const Point2D& rhs );

inline
Point2D
operator+( const Point2D& lhs, const Vector2D& rhs );

inline
Point2D
operator+( const Vector2D& lhs, const Point2D& rhs );

inline
Point2D
operator-( const Point2D& lhs, const Vector2D& rhs );

inline
Vector2D
operator-( const Point2D& lhs, const Point2D& rhs );

// Stream operators:
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Point2D& rhs );

template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Point2D& rhs );


////////////////////////////////////////////////////////////////////////////////
/**
@class      Vector2D

@brief      Type defining a two-dimensional vector.
*/
////////////////////////////////////////////////////////////////////////////////
class Vector2D
{
public:
    /// Type for sequential access.
    typedef f64_t                       *iterator;
    /// Type for sequential access of const object.
    typedef const f64_t                 *const_iterator;

public:
    // Ctor.
    inline explicit
    Vector2D( f64_t X = 0, f64_t Y = 0 );

public:
    // Arithmetic operators.
    inline
    Vector2D&
    operator+=( const Vector2D& rhs );

    inline
    Vector2D&
    operator-=( const Vector2D& rhs );

    inline
    Vector2D&
    operator*=( const f64_t& rhs );

    // Random-access operators.
    inline
    f64_t&
    operator[]( uint32_t i );

    inline
    const f64_t&
    operator[]( uint32_t i ) const;

public:
    // Utility methods.
    inline
    f64_t
    length( void ) const;

    Vector2D
    normal( void ) const throw( ZeroDivide );

public:
    // Sequential-access methods.
    inline
    iterator
    begin( void );

    inline
    const_iterator
    begin( void ) const;

    inline
    iterator
    end( void );

    inline
    const_iterator
    end( void ) const;

private:
    /// Dimension of Vector.
    enum{ DIM = 2 };
    /// Array of components.
    f64_t                               m_components[DIM];

};

// (Global) operators.
inline
bool
operator==( const Vector2D& lhs, const Vector2D& rhs );

inline
bool
operator!=( const Vector2D& lhs, const Vector2D& rhs );

inline
Vector2D
operator+( const Vector2D& lhs, const Vector2D& rhs );

inline
Vector2D
operator-( const Vector2D& lhs, const Vector2D& rhs );

inline
Vector2D
operator*( const Vector2D& lhs, const f64_t& rhs );

inline
Vector2D
operator*( const f64_t& lhs, const Vector2D& rhs );

// Utility function:
inline
f64_t
Dot( const Vector2D& lhs, const Vector2D& rhs );

// Stream operator:
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Vector2D& rhs );

template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Vector2D& rhs );


////////////////////////////////////////////////////////////////////////////////
/**
@struct     Line2D

@brief      Type defining an infinite line in two dimensions.
*/
////////////////////////////////////////////////////////////////////////////////
struct Line2D
{
public:
    inline
    Line2D( void );

    inline
    Line2D( const Point2D& p, const Vector2D& v );

public:
    /// A point on the line.
    Point2D                             point;
    /// A vector parallel to the line.
    Vector2D                            vector;

};

// Utility function:
bool
Intersects( const Line2D& l1, const Line2D& l2, Point2D *p = 0 );


////////////////////////////////////////////////////////////////////////////////
/**
@struct     LineSeg2D

@brief      Type defining a line segment in two dimensions.
*/
////////////////////////////////////////////////////////////////////////////////
struct LineSeg2D
{
public:
    inline
    LineSeg2D( void );

    inline
    LineSeg2D( const Point2D& p1, const Point2D& p2 );

public:
    /// One of the endpoints of the line segment.
    Point2D                             point1;
    /// The other endpoint of the line segment.
    Point2D                             point2;

};

// Utility function:
bool
Intersects( const LineSeg2D& l1, const LineSeg2D& l2, Point2D *p = 0 );


////////////////////////////////////////////////////////////////////////////////
/**
@class      Point3D

@brief      Type defining a three-dimensional point.
*/
////////////////////////////////////////////////////////////////////////////////
class Point3D
{
public:
    /// Type for sequential access.
    typedef f64_t                       *iterator;
    /// Type for sequential access of const object.
    typedef const f64_t                 *const_iterator;

public:
    // Ctor.
    inline explicit
    Point3D( f64_t X = 0, f64_t Y = 0, f64_t Z = 0 );

public:
    // Arithmetic operators.
    inline
    Point3D&
    operator+=( const Vector3D& rhs );

    inline
    Point3D&
    operator-=( const Vector3D& rhs );

    // Random-access operators.
    inline
    f64_t&
    operator[]( uint32_t i );

    inline
    const f64_t&
    operator[]( uint32_t i ) const;

public:
    // Sequential-access methods.
    inline
    iterator
    begin( void );

    inline
    const_iterator
    begin( void ) const;

    inline
    iterator
    end( void );

    inline
    const_iterator
    end( void ) const;

private:
    /// Dimension of Point.
    enum{ DIM = 3 };
    /// Array of components.
    f64_t                               m_components[DIM];

};

// (Global) operators.
inline
bool
operator==( const Point3D& lhs, const Point3D& rhs );

inline
bool
operator!=( const Point3D& lhs, const Point3D& rhs );

inline
Point3D
operator+( const Point3D& lhs, const Vector3D& rhs );

inline
Point3D
operator+( const Vector3D& lhs, const Point3D& rhs );

inline
Point3D
operator-( const Point3D& lhs, const Vector3D& rhs );

inline
Vector3D
operator-( const Point3D& lhs, const Point3D& rhs );

// Utility functions:
f64_t
Distance( const Point3D& p, const Line3D& l );

f64_t
Distance( const Point3D& p, const Plane3D& m );

// Stream operator:
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Point3D& rhs );

template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Point3D& rhs );


////////////////////////////////////////////////////////////////////////////////
/**
@class      Vector3D

@brief      Type defining a three-dimensional vector.
*/
////////////////////////////////////////////////////////////////////////////////
class Vector3D
{
public:
    /// Type for sequential access.
    typedef f64_t                       *iterator;
    /// Type for sequential access of const object.
    typedef const f64_t                 *const_iterator;

public:
    // Ctor.
    inline explicit
    Vector3D( f64_t X = 0, f64_t Y = 0, f64_t Z = 0 );

public:
    // Arithmetic operators.
    inline
    Vector3D&
    operator+=( const Vector3D& rhs );

    inline
    Vector3D&
    operator-=( const Vector3D& rhs );

    inline
    Vector3D&
    operator*=( const f64_t& rhs );

	inline f64_t operator*(const Vector3D &rhs);
	inline Vector3D operator^(const Vector3D &rhs);
	inline Vector3D operator-(void);

    // Random-access operators.
    inline
    f64_t&
    operator[]( uint32_t i );

    inline
    const f64_t&
    operator[]( uint32_t i ) const;

public:
    // Utility methods.
    inline
    f64_t
    length( void ) const;

    Vector3D
    normal( void ) const throw( ZeroDivide );

public:
    // Sequential-access methods.
    inline
    iterator
    begin( void );

    inline
    const_iterator
    begin( void ) const;

    inline
    iterator
    end( void );

    inline
    const_iterator
    end( void ) const;

private:
    /// Dimension of Vector.
    enum{ DIM = 3 };
    /// Array of components.
    f64_t                               m_components[DIM];

};

// (Global) operators.
inline
bool
operator==( const Vector3D& lhs, const Vector3D& rhs );

inline
bool
operator!=( const Vector3D& lhs, const Vector3D& rhs );

inline
Vector3D
operator+( const Vector3D& lhs, const Vector3D& rhs );

inline
Vector3D
operator-( const Vector3D& lhs, const Vector3D& rhs );

inline
Vector3D
operator*( const Vector3D& lhs, const f64_t& rhs );

inline
Vector3D
operator*( const f64_t& lhs, const Vector3D& rhs );

// Utility functions:
inline
f64_t
Dot( const Vector3D& lhs, const Vector3D& rhs );

Vector3D
Cross( const Vector3D& v1, const Vector3D& v2 );

inline f64_t Vector3D::operator*(const Vector3D &rhs)		
{
	return Dot(*this, rhs); 
}
inline Vector3D Vector3D::operator^(const Vector3D &rhs)	
{ 
	return Cross(*this, rhs); 
}
inline Vector3D Vector3D::operator-(void)
{
	for(int i = 0; i < DIM; ++i)
	{
		m_components[i] = -m_components[i];
	}
	return *this;
}

// Stream operator:
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Vector3D& rhs );

template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Vector3D& rhs );


////////////////////////////////////////////////////////////////////////////////
/**
@struct     Line3D

@brief      Type defining an infinite line in three dimensions.
*/
////////////////////////////////////////////////////////////////////////////////
struct Line3D
{
public:
    inline
    Line3D( void );

    inline
    Line3D( const Point3D& p, const Vector3D& v );

public:
    /// A point on the line.
    Point3D                             point;
    /// A vector parallel to the line.
    Vector3D                            vector;

};

// Utility functions:
f64_t
AngleBetween( const Line3D& l1, const Line3D& l2 );

f64_t
AngleBetween( const Line3D& l, const Plane3D& p );

bool
Coplanar( const Line3D& l1, const Line3D& l2 );

bool
Intersects( const Line3D& l, const Plane3D& p, Point3D *m = 0 );

bool
Parallel( const Line3D& l1, const Line3D& l2 );

bool
Perpendicular( const Line3D& l1, const Line3D& l2 );

bool
Parallel( const Line3D& l, const Plane3D& p );

bool
Perpendicular( const Line3D& l, const Plane3D& p );




////////////////////////////////////////////////////////////////////////////////
/**
@struct     LineSeg3D

@brief      Type defining a line segment in three dimensions.
*/
////////////////////////////////////////////////////////////////////////////////
struct LineSeg3D
{
public:
    inline
    LineSeg3D( void );

    inline
    LineSeg3D( const Point3D& p1, const Point3D& p2 );

public:
    bool
    contains( const Point3D& p ) const;

public:
    /// One of the endpoints of the line segment.
    Point3D                             point1;
    /// The other endpoint of the line segment.
    Point3D                             point2;

};

// Utility function:
bool
Intersects( const LineSeg3D& l, const Triangle3D& t, Point3D *p = 0 );


////////////////////////////////////////////////////////////////////////////////
/**
@struct     Ray3D

@brief      Type defining a three-dimensional ray.
*/
////////////////////////////////////////////////////////////////////////////////
struct Ray3D
{
public:
    inline
    Ray3D( void );

    inline
    Ray3D( const Point3D& o, const Vector3D& d );

public:
    bool
    contains( const Point3D& p, f64_t *t = 0 ) const;

public:
    /// The origin of the ray.
    Point3D                             origin;
    /// The direction of the ray.
    Vector3D                            direction;

};

// Utility functions:
int32_t
Intersects( const Ray3D& r, const Sphere3D& s,
    std::pair< Point3D, Point3D > *p = 0 , float *time = 0);

bool
Intersects( const Ray3D& r, const Triangle3D& t, Point3D *p = 0 );

int32_t
Intersects( const Ray3D& r, const Box3D& b,
    std::pair< Point3D, Point3D > *p = 0 );

bool
Intersects( const Ray3D& r, const Plane3D& p, Point3D *m, float *t);


////////////////////////////////////////////////////////////////////////////////
/**
@struct     Box3D

@brief      Type defining a three-dimensional box (is there a two-dimensional
            box?--yes, it's called a square).
*/
////////////////////////////////////////////////////////////////////////////////
struct Box3D
{
public:
    inline
    Box3D( void );

    inline
    Box3D( const Point3D& o, const Point3D& e );

public:
    bool
    contains( const Point3D& p ) const;

public:
    /// Corner with minimum components.
    Point3D                             origin;
    /// Corner with maximum components.
    Point3D                             extent;

};


////////////////////////////////////////////////////////////////////////////////
/**
@struct     Sphere3D

@brief      Type defining a three-dimensional sphere (is there another kind?).
*/
////////////////////////////////////////////////////////////////////////////////
struct Sphere3D
{
public:
    inline
    Sphere3D( void );

    inline
    Sphere3D( const Point3D& c, f32_t r );

public:
    /// The center point of the sphere.
    Point3D                             center;
    /// The radius of the sphere.
    f32_t                               radius;

};


////////////////////////////////////////////////////////////////////////////////
/**
@class      Plane3D

@brief      Type defining a three-dimensional plane.
*/
////////////////////////////////////////////////////////////////////////////////
class Plane3D
{
public:
    /// Type for sequential access.
    typedef f64_t                       *iterator;
    /// Type for sequential access of const object.
    typedef const f64_t                 *const_iterator;

public:
    // Ctor.
    inline explicit
    Plane3D( f64_t A = 0, f64_t B = 0, f64_t C = 0, f64_t D = 0 );

public:
    // Random-access operators.
    inline
    f64_t&
    operator[]( uint32_t i );

    inline
    const f64_t&
    operator[]( uint32_t i ) const;

public:
    // Utility methods.
    inline
    Vector3D
    normal( void ) const;

public:
    // Sequential-access methods.
    inline
    iterator
    begin( void );

    inline
    const_iterator
    begin( void ) const;

    inline
    iterator
    end( void );

    inline
    const_iterator
    end( void ) const;

private:
    enum{ DIM = 4 };
    /// Array of components.
    f64_t                               m_components[DIM];

};

// Utility functions:
f64_t
AngleBetween( const Plane3D& p1, const Plane3D& p2 );

bool
Parallel( const Plane3D& p1, const Plane3D& p2 );

bool
Perpendicular( const Plane3D& p1, const Plane3D& p2 );


////////////////////////////////////////////////////////////////////////////////
/**
@class      Triangle3D

@brief      Type defining a three-dimensional triangle.
*/
////////////////////////////////////////////////////////////////////////////////
class Triangle3D
{
public:
    /// Type for sequential access.
    typedef Point3D                     *iterator;
    /// Type for sequential access of const object.
    typedef const Point3D               *const_iterator;

public:
    inline
    Triangle3D( void );

    inline
    Triangle3D( const Point3D& p1, const Point3D& p2, const Point3D& p3 );

public:
    inline
    Point3D&
    operator[]( uint32_t i );

    inline
    const Point3D&
    operator[]( uint32_t i ) const;

public:
    bool
    contains( const Point3D& p ) const;

public:
    // Sequential-access methods.
    inline
    iterator
    begin( void );

    inline
    const_iterator
    begin( void ) const;

    inline
    iterator
    end( void );

    inline
    const_iterator
    end( void ) const;

private:
    enum{ DIM = 3 };
    /// Array of points.
    Point3D                             m_points[DIM];

};

// Utility function:
int32_t
Intersects( const Triangle3D& t1, const Triangle3D& t2,
    std::pair< Point3D, Point3D > *p = 0 );


////////////////////////////////////////////////////////////////////////////////
/**
@class      Point4D

@brief      Type defining a four-dimensional point.
*/
////////////////////////////////////////////////////////////////////////////////
class Point4D
{
public:
    /// Type for sequential access.
    typedef f64_t                       *iterator;
    /// Type for sequential access of const object.
    typedef const f64_t                 *const_iterator;

public:
    // Ctor.
    inline explicit
    Point4D( f64_t X = 0, f64_t Y = 0, f64_t Z = 0, f64_t W = 0 );

public:
    // Arithmetic operators.
    inline
    Point4D&
    operator+=( const Vector4D& rhs );

    inline
    Point4D&
    operator-=( const Vector4D& rhs );

    // Random-access operators.
    inline
    f64_t&
    operator[]( uint32_t i );

    inline
    const f64_t&
    operator[]( uint32_t i ) const;

public:
    // Sequential-access methods.
    inline
    iterator
    begin( void );

    inline
    const_iterator
    begin( void ) const;

    inline
    iterator
    end( void );

    inline
    const_iterator
    end( void ) const;

private:
    /// Dimension of Point.
    enum{ DIM = 4 };
    /// Array of components.
    f64_t                               m_components[DIM];

};

// (Global) operators.
inline
bool
operator==( const Point4D& lhs, const Point4D& rhs );

inline
bool
operator!=( const Point4D& lhs, const Point4D& rhs );

inline
Point4D
operator+( const Point4D& lhs, const Vector4D& rhs );

inline
Point4D
operator+( const Vector4D& lhs, const Point4D& rhs );

inline
Point4D
operator-( const Point4D& lhs, const Vector4D& rhs );

inline
Vector4D
operator-( const Point4D& lhs, const Point4D& rhs );

// Stream operator:
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Point4D& rhs );

template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Point4D& rhs );


////////////////////////////////////////////////////////////////////////////////
/**
@class      Vector4D

@brief      Type defining a four-dimensional vector.
*/
////////////////////////////////////////////////////////////////////////////////
class Vector4D
{
public:
    /// Type for sequential access.
    typedef f64_t                       *iterator;
    /// Type for sequential access of const object.
    typedef const f64_t                 *const_iterator;

public:
    // Ctor.
    inline explicit
    Vector4D( f64_t X = 0, f64_t Y = 0, f64_t Z = 0, f64_t W = 0 );

public:
    // Arithmetic operators.
    inline
    Vector4D&
    operator+=( const Vector4D& rhs );

    inline
    Vector4D&
    operator-=( const Vector4D& rhs );

    inline
    Vector4D&
    operator*=( const f64_t& rhs );

    // Random-access operators.
    inline
    f64_t&
    operator[]( uint32_t i );

    inline
    const f64_t&
    operator[]( uint32_t i ) const;

public:
    // Utility methods.
    inline
    f64_t
    length( void ) const;

    Vector4D
    normal( void ) const throw( ZeroDivide );

public:
    // Sequential-access methods.
    inline
    iterator
    begin( void );

    inline
    const_iterator
    begin( void ) const;

    inline
    iterator
    end( void );

    inline
    const_iterator
    end( void ) const;

private:
    /// Dimension of Vector.
    enum{ DIM = 4 };
    /// Array of components.
    f64_t                               m_components[DIM];

};

// (Global) operators.
inline
bool
operator==( const Vector4D& lhs, const Vector4D& rhs );

inline
bool
operator!=( const Vector4D& lhs, const Vector4D& rhs );

inline
Vector4D
operator+( const Vector4D& lhs, const Vector4D& rhs );

inline
Vector4D
operator-( const Vector4D& lhs, const Vector4D& rhs );

inline
Vector4D
operator*( const Vector4D& lhs, const f64_t& rhs );

inline
Vector4D
operator*( const f64_t& lhs, const Vector4D& rhs );

// Utility function:
inline
f64_t
Dot( const Vector4D& lhs, const Vector4D& rhs );

// Stream operator:
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Vector4D& rhs );

template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Vector4D& rhs );


// Utility function templates:
template< class T >
T
ToRadians( const T& Degrees );

template< class Sequence >
bool
IsZero( const Sequence& s );

template< class T >
const T&
MIN( const T& first, const T& second );

template< class T >
const T&
MAX( const T& first, const T& second );

} // namespace Geometry


namespace Geometry
{

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the x- and y-components of the two-dimensional point
            to be specified.

@param      X The x-component.
@param      Y The y-component.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point2D::Point2D( f64_t X, f64_t Y )
{
    iterator    it                      = begin();

    *it                                 = X;
    *++it                               = Y;

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (addition) operator. Adds the given
            two-dimensional vector to the point and returns the result.

@param      rhs The vector to be added to the point.

@return     Returns the result of adding the point and vector together (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point2D&
Point2D::operator+=( const Vector2D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::plus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (subtraction) operator. Subtracts the given
            two-dimensional vector from the point and returns the result.

@param      rhs The vector to be subtracted from the point.

@return     Returns the result of subtracting the vector from the point (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point2D&
Point2D::operator-=( const Vector2D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::minus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for two-dimensional point.

@param      i The index of the component desired.

@return     Returns a reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t&
Point2D::operator[]( uint32_t i )
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for const two-dimensional point.

@param      i The index of the component desired.

@return     Returns a const reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
const f64_t&
Point2D::operator[]( uint32_t i ) const
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a two-dimensional
            point.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point2D::iterator
Point2D::begin( void )
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a const
            two-dimensional point.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point2D::const_iterator
Point2D::begin( void ) const
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point2D::iterator
Point2D::end( void )
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence for const object.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point2D::const_iterator
Point2D::end( void ) const
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Equality operator for two-dimensional point.

@param      lhs The first point to compare.
@param      rhs The second point to compare.

@return     Returns true if the two points have the same components, false
            otherwise.

@warning    Demands strict equality. (May wish to introduce an epsilon value.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator==( const Point2D& lhs, const Point2D& rhs )
{
    bool    ret                         = std::equal( lhs.begin(), lhs.end(),
        rhs.begin() );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Inequality operator for two-dimensional point.

@param      lhs The first point to compare.
@param      rhs The second point to compare.

@return     Returns true if the two points are not the same, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator!=( const Point2D& lhs, const Point2D& rhs )
{
    // Reuse the equality operator.
    bool    ret                         = !(lhs == rhs);

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Addition operator. Adds a two-dimensional vector to a point and
            returns the result.

@param      lhs The point.
@param      rhs The vector.

@return     Returns the result of adding the vector to the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point2D
operator+( const Point2D& lhs, const Vector2D& rhs )
{
    Point2D ret( lhs );

    // Reuse the class method.
    ret                                 += rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Addition operator. Adds a two-dimensional vector to a point and
            returns the result.

@param      lhs The vector.
@param      rhs The point.

@return     Returns the result of adding the vector to the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point2D
operator+( const Vector2D& lhs, const Point2D& rhs )
{
    // Reorder and reuse.
    return rhs + lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Subtraction operator. Subtracts a two-dimensional vector from 
            point and returns the result.

@param      lhs The point.
@param      rhs The vector.

@return     Returns the result of subtracting the vector from the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point2D
operator-( const Point2D& lhs, const Vector2D& rhs )
{
    Point2D ret( lhs );

    ret                                 -= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Subtraction operator. Subtracts two two-dimensional points and
            returns the result.

@param      lhs The first point.
@param      rhs The second point.

@return     Returns the result of subtracting one point from another.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D
operator-( const Point2D& lhs, const Point2D& rhs )
{
    Vector2D    ret;

    std::transform( lhs.begin(), lhs.end(), rhs.begin(), ret.begin(),
        std::minus< f64_t >() );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      ostream insertion operator for two-dimensional point.

@param      lhs The ostream to insert into.
@param      rhs The point to insert.

@return     Returns a reference to the ostream.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Point2D& rhs )
{
    std::basic_ostringstream< charT, traits >   s;
    s.flags( lhs.flags() );
    s.imbue( lhs.getloc() );
    s.precision( lhs.precision() );

    typename Point2D::const_iterator    i   = rhs.begin();
    s << '(' << *i;

    for( ++i; i != rhs.end(); ++i )
    {
        s << ',' << *i;
    }
    s << ')';

    return lhs << s.str();
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      istream extraction operator for two-dimensional point.

@param      lhs The istream to extract from.
@param      rhs The point to extract.

@return     Returns a reference to the istream.

@note       The point can be of the form:
@note       '*' can be one of '(', '<', '[', '{'.
@note       '$' can be one of ')', '>', ']', '}'.
            @li * 1, 2, ..., D $
            @li 1, 2, ..., D
            @li 1 2 ... D
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Point2D& rhs )
{
    // If this character is a delimiter, get rid of it.
    charT   c;
    if( lhs >> c && '(' != c && '<' != c && '[' != c && '{' != c )
    {
        lhs.putback( c );
    }

    // Next, we should get the components.
    Point2D     temp;
    for( typename Point2D::iterator i = temp.begin(); i != temp.end(); ++i )
    {
        lhs >> *i;
        // There might be a comma or a ending delimiter next, so ditch it.
        if( lhs.eof() )
        {
            break;
        }
        if( lhs >> c && ',' != c && ')' != c && '>' != c && ']' != c
            && '}' != c )
        {
            lhs.putback( c );
        }
    }
    // If all went well, save the extracted point.
    if( lhs.good() || (lhs.eof() && !lhs.bad()) )
    {
        rhs                             = temp;
    }
    // Set the stream fail state.
    else
    {
        lhs.setstate( std::ios_base::failbit );
    }

    return lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the x- and y-components of the two-dimensional vector
            to be specified.

@param      X The x-component.
@param      Y The y-component.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D::Vector2D( f64_t X, f64_t Y )
{
    iterator    it                      = begin();

    *it                                 = X;
    *++it                               = Y;

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (addition) operator. Adds the given
            two-dimensional vector and returns the result.

@param      rhs The vector to be added.

@return     Returns the result of adding the two vectors (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D&
Vector2D::operator+=( const Vector2D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::plus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (subtraction) operator. Subtracts the given
            two-dimensional vector and returns the result.

@param      rhs The vector to be subtracted.

@return     Returns the result of subtracting the vector (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D&
Vector2D::operator-=( const Vector2D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::minus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (multiplication) operator. Multiplies the
            two-dimensional vector by the given scalar and returns the result.

@param      rhs The scaling factor for the vector.

@return     Returns the now-scaled vector (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D&
Vector2D::operator*=( const f64_t& rhs )
{
    std::transform( begin(), end(), begin(),
        std::bind2nd( std::multiplies< f64_t >(), rhs ) );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for two-dimensional vector.

@param      i The index of the component desired.

@return     Returns a reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t&
Vector2D::operator[]( uint32_t i )
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for const two-dimensional vector.

@param      i The index of the component desired.

@return     Returns a const reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
const f64_t&
Vector2D::operator[]( uint32_t i ) const
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Length-determining method for two-dimensional vector.

@return     Returns the length of the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t
Vector2D::length( void ) const
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::sqrt;
#endif
    f64_t   ret                         = sqrt( Dot( *this, *this ) );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a two-dimensional
            vector.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D::iterator
Vector2D::begin( void )
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a const
            two-dimensional vector.

@return     Returns iterator to beginning of sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D::const_iterator
Vector2D::begin( void ) const
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D::iterator
Vector2D::end( void )
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence for const object.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D::const_iterator
Vector2D::end( void ) const
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Equality operator for two-dimensional vector.

@param      lhs The first vector to compare.
@param      rhs The second vector to compare.

@return     Returns true if the two vectors have the same components, false
            otherwise.

@warning    Demands strict equality. (May wish to introduce an epsilon value.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator==( const Vector2D& lhs, const Vector2D& rhs )
{
    bool    ret                         = std::equal( lhs.begin(), lhs.end(),
        rhs.begin() );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Inequality operator for two-dimensional vector.

@param      lhs The first vector to compare.
@param      rhs The second vector to compare.

@return     Returns true if the two vectors are not the same, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator!=( const Vector2D& lhs, const Vector2D& rhs )
{
    bool    ret                         = !(lhs == rhs);

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Addition operator. Adds two two-dimensional vectors and returns the
            result.

@param      lhs The first vector.
@param      rhs The second vector.

@return     Returns the result of adding the two vectors.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D
operator+( const Vector2D& lhs, const Vector2D& rhs )
{
    Vector2D    ret( lhs );

    ret                                 += rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Subtraction operator. Subtracts two two-dimensional vectors and
            returns the result.

@param      lhs The first vector.
@param      rhs The second vector.

@return     Returns the result of subtracting the vectors.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D
operator-( const Vector2D& lhs, const Vector2D& rhs )
{
    Vector2D    ret( lhs );

    ret                                 -= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies the two-dimensional vector by
            the given scalar and returns the result.

@param      lhs The vector.
@param      rhs The scalar.

@return     Returns the result of scaling the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D
operator*( const Vector2D& lhs, const f64_t& rhs )
{
    Vector2D    ret( lhs );

    ret                                 *= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies the two-dimensional vector by
            the given scalar and returns the result.

@param      lhs The scalar.
@param      rhs The vector.

@return     Returns the result of scaling the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector2D
operator*( const f64_t& lhs, const Vector2D& rhs )
{
    // Reorder and reuse.
    return rhs*lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Dot product for two-dimensional vector.

@param      lhs The first vector.
@param      rhs The second vector.

@return     Returns the dot product of the two vectors.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t
Dot( const Vector2D& lhs, const Vector2D& rhs )
{
    f64_t   ret                         = std::inner_product( lhs.begin(),
        lhs.end(), rhs.begin(), f64_t( 0 ) );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      ostream insertion operator for two-dimensional vector.

@param      lhs The ostream to insert into.
@param      rhs The vector to insert.

@return     Returns a reference to the ostream.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Vector2D& rhs )
{
    std::basic_ostringstream< charT, traits >   s;
    s.flags( lhs.flags() );
    s.imbue( lhs.getloc() );
    s.precision( lhs.precision() );

    typename Vector2D::const_iterator   i   = rhs.begin();
    s << '(' << *i;

    for( ++i; i != rhs.end(); ++i )
    {
        s << ',' << *i;
    }
    s << ')';

    return lhs << s.str();
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      istream extraction operator for two-dimensional vector.

@param      lhs The istream to extract from.
@param      rhs The vector to extract.

@return     Returns a reference to the istream.

@note       The vector can be of the form:
@note       '*' can be one of '(', '<', '[', '{'.
@note       '$' can be one of ')', '>', ']', '}'.
            @li * 1, 2, ..., D $
            @li 1, 2, ..., D
            @li 1 2 ... D
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Vector2D& rhs )
{
    // If this character is a delimiter, get rid of it.
    charT   c;
    if( lhs >> c && '(' != c && '<' != c && '[' != c && '{' != c )
    {
        lhs.putback( c );
    }

    // Next, we should get the components.
    Vector2D    temp;
    for( typename Vector2D::iterator i = temp.begin(); i != temp.end(); ++i )
    {
        lhs >> *i;
        // There might be a comma or a ending delimiter next, so ditch it.
        if( lhs.eof() )
        {
            break;
        }
        if( lhs >> c && ',' != c && ')' != c && '>' != c && ']' != c
            && '}' != c )
        {
            lhs.putback( c );
        }
    }
    // If all went well, save the extracted vector.
    if( lhs.good() || (lhs.eof() && !lhs.bad()) )
    {
        rhs                             = temp;
    }
    // Set the stream fail state.
    else
    {
        lhs.setstate( std::ios_base::failbit );
    }

    return lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Default ctor for two-dimensional line.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Line2D::Line2D( void )
: point(), vector()
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the components (point and vector) of the
            two-dimensional line to be specified.

@param      p A point on the line.
@param      v A vector parallel to the line.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Line2D::Line2D( const Point2D& p, const Vector2D& v )
: point( p ), vector( v )
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Default ctor for two-dimensional line segment.
*/
////////////////////////////////////////////////////////////////////////////////
inline
LineSeg2D::LineSeg2D( void )
: point1(), point2()
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the components (the two endpoints) of the
            two-dimensional line segment to be specified.

@param      p1 The first endpoint.
@param      p2 The second endpoint.
*/
////////////////////////////////////////////////////////////////////////////////
inline
LineSeg2D::LineSeg2D( const Point2D& p1, const Point2D& p2 )
: point1( p1 ), point2( p2 )
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the x-, y-, and z-components of the three-dimensional
            point to be specified.

@param      X The x-component.
@param      Y The y-component.
@param      Z The z-component.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D::Point3D( f64_t X, f64_t Y, f64_t Z )
{
    iterator    it                      = begin();

    *it                                 = X;
    *++it                               = Y;
    *++it                               = Z;

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (addition) operator. Adds the given
            three-dimensional vector to the point and returns the result.

@param      rhs The vector to be added to the point.

@return     Returns the result of adding the point and vector together (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D&
Point3D::operator+=( const Vector3D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::plus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (subtraction) operator. Subtracts the given
            three-dimensional vector from the point and returns the result.

@param      rhs The vector to be subtracted from the point.

@return     Returns the result of subtracting the vector from the point (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D&
Point3D::operator-=( const Vector3D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::minus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for three-dimensional point.

@param      i The index of the component desired.

@return     Returns a reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t&
Point3D::operator[]( uint32_t i )
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for const three-dimensional point.

@param      i The index of the component desired.

@return     Returns a const reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
const f64_t&
Point3D::operator[]( uint32_t i ) const
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a
            three-dimensional point.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D::iterator
Point3D::begin( void )
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a const
            three-dimensional point.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D::const_iterator
Point3D::begin( void ) const
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D::iterator
Point3D::end( void )
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence for const object.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D::const_iterator
Point3D::end( void ) const
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Equality operator for three-dimensional point.

@param      lhs The first point to compare.
@param      rhs The second point to compare.

@return     Returns true if the two points have the same components, false
            otherwise.

@warning    Demands strict equality. (May wish to introduce an epsilon value.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator==( const Point3D& lhs, const Point3D& rhs )
{
    bool    ret                         = std::equal( lhs.begin(), lhs.end(),
        rhs.begin() );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Inequality operator for three-dimensional point.

@param      lhs The first point to compare.
@param      rhs The second point to compare.

@return     Returns true if the two points are not the same, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator!=( const Point3D& lhs, const Point3D& rhs )
{
    bool    ret                         = !(lhs == rhs);

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Addition operator. Adds a three-dimensional vector to a point and
            returns the result.

@param      lhs The point.
@param      rhs The vector.

@return     Returns the result of adding the vector to the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D
operator+( const Point3D& lhs, const Vector3D& rhs )
{
    Point3D ret( lhs );

    ret                                 += rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Addition operator. Adds a three-dimensional vector to a point and
            returns the result.

@param      lhs The vector.
@param      rhs The point.

@return     Returns the result of adding the vector to the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D
operator+( const Vector3D& lhs, const Point3D& rhs )
{
    // Reorder and reuse.
    return rhs + lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Subtraction operator. Subtracts a three-dimensional vector from 
            point and returns the result.

@param      lhs The point.
@param      rhs The vector.

@return     Returns the result of subtracting the vector from the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D
operator-( const Point3D& lhs, const Vector3D& rhs )
{
    Point3D ret( lhs );

    ret                                 -= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Subtraction operator. Subtracts two three-dimensional points and
            returns the result.

@param      lhs The first point.
@param      rhs The second point.

@return     Returns the result of subtracting one point from another.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D
operator-( const Point3D& lhs, const Point3D& rhs )
{
    Vector3D    ret;

    std::transform( lhs.begin(), lhs.end(), rhs.begin(), ret.begin(),
        std::minus< f64_t >() );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      ostream insertion operator for three-dimensional point.

@param      lhs The ostream to insert into.
@param      rhs The point to insert.

@return     Returns a reference to the ostream.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Point3D& rhs )
{
    std::basic_ostringstream< charT, traits >   s;
    s.flags( lhs.flags() );
    s.imbue( lhs.getloc() );
    s.precision( lhs.precision() );

    typename Point3D::const_iterator    i   = rhs.begin();
    s << '(' << *i;

    for( ++i; i != rhs.end(); ++i )
    {
        s << ',' << *i;
    }
    s << ')';

    return lhs << s.str();
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      istream extraction operator for three-dimensional point.

@param      lhs The istream to extract from.
@param      rhs The point to extract.

@return     Returns a reference to the istream.

@note       The point can be of the form:
@note       '*' can be one of '(', '<', '[', '{'.
@note       '$' can be one of ')', '>', ']', '}'.
            @li * 1, 2, ..., D $
            @li 1, 2, ..., D
            @li 1 2 ... D
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Point3D& rhs )
{
    // If this character is a delimiter, get rid of it.
    charT   c;
    if( lhs >> c && '(' != c && '<' != c && '[' != c && '{' != c )
    {
        lhs.putback( c );
    }

    // Next, we should get the components.
    Point3D     temp;
    for( typename Point3D::iterator i = temp.begin(); i != temp.end(); ++i )
    {
        lhs >> *i;
        // There might be a comma or a ending delimiter next, so ditch it.
        if( lhs.eof() )
        {
            break;
        }
        if( lhs >> c && ',' != c && ')' != c && '>' != c && ']' != c
            && '}' != c )
        {
            lhs.putback( c );
        }
    }
    // If all went well, save the extracted point.
    if( lhs.good() || (lhs.eof() && !lhs.bad()) )
    {
        rhs                             = temp;
    }
    // Set the stream fail state.
    else
    {
        lhs.setstate( std::ios_base::failbit );
    }

    return lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the x-, y-, and z-components of the three-dimensional
            vector to be specified.

@param      X The x-component.
@param      Y The y-component.
@param      Z The z-component.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D::Vector3D( f64_t X, f64_t Y, f64_t Z )
{
    iterator    it                      = begin();

    *it                                 = X;
    *++it                               = Y;
    *++it                               = Z;

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (addition) operator. Adds the given
            three-dimensional vector and returns the result.

@param      rhs The vector to be added.

@return     Returns the result of adding the two vectors (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D&
Vector3D::operator+=( const Vector3D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::plus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (subtraction) operator. Subtracts the given
            three-dimensional vector and returns the result.

@param      rhs The vector to be subtracted.

@return     Returns the result of subtracting the vector (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D&
Vector3D::operator-=( const Vector3D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::minus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (multiplication) operator. Multiplies the
            three-dimensional vector by the given scalar and returns the result.

@param      rhs The scaling factor for the vector.

@return     Returns the now-scaled vector (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D&
Vector3D::operator*=( const f64_t& rhs )
{
    std::transform( begin(), end(), begin(),
        std::bind2nd( std::multiplies< f64_t >(), rhs ) );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for three-dimensional vector.

@param      i The index of the component desired.

@return     Returns a reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t&
Vector3D::operator[]( uint32_t i )
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for const three-dimensional vector.

@param      i The index of the component desired.

@return     Returns a const reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
const f64_t&
Vector3D::operator[]( uint32_t i ) const
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Length-determining method for three-dimensional vector.

@return     Returns the length of the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t
Vector3D::length( void ) const
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::sqrt;
#endif
    f64_t   ret                         = sqrt( Dot( *this, *this ) );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a
            three-dimensional vector.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D::iterator
Vector3D::begin( void )
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a const
            three-dimensional vector.

@return     Returns iterator to beginning of sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D::const_iterator
Vector3D::begin( void ) const
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D::iterator
Vector3D::end( void )
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence for const object.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D::const_iterator
Vector3D::end( void ) const
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Equality operator for three-dimensional vector.

@param      lhs The first vector to compare.
@param      rhs The second vector to compare.

@return     Returns true if the two vectors have the same components, false
            otherwise.

@warning    Demands strict equality. (May wish to introduce an epsilon value.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator==( const Vector3D& lhs, const Vector3D& rhs )
{
    bool    ret                         = std::equal( lhs.begin(), lhs.end(),
        rhs.begin() );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Inequality operator for three-dimensional vector.

@param      lhs The first vector to compare.
@param      rhs The second vector to compare.

@return     Returns true if the two vectors are not the same, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator!=( const Vector3D& lhs, const Vector3D& rhs )
{
    bool    ret                         = !(lhs == rhs);

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Addition operator. Adds two three-dimensional vectors and returns
            the result.

@param      lhs The first vector.
@param      rhs The second vector.

@return     Returns the result of adding the two vectors.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D
operator+( const Vector3D& lhs, const Vector3D& rhs )
{
    Vector3D    ret( lhs );

    ret                                 += rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Subtraction operator. Subtracts two three-dimensional vectors and
            returns the result.

@param      lhs The first vector.
@param      rhs The second vector.

@return     Returns the result of subtracting the vectors.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D
operator-( const Vector3D& lhs, const Vector3D& rhs )
{
    Vector3D    ret( lhs );

    ret                                 -= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies the three-dimensional vector by
            the given scalar and returns the result.

@param      lhs The vector.
@param      rhs The scalar.

@return     Returns the result of scaling the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D
operator*( const Vector3D& lhs, const f64_t& rhs )
{
    Vector3D    ret( lhs );

    ret                                 *= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies the three-dimensional vector by
            the given scalar and returns the result.

@param      lhs The scalar.
@param      rhs The vector.

@return     Returns the result of scaling the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D
operator*( const f64_t& lhs, const Vector3D& rhs )
{
    // Reorder and reuse.
    return rhs*lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Dot product for three-dimensional vector.

@param      lhs The first vector.
@param      rhs The second vector.

@return     Returns the dot product of the two vectors.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t
Dot( const Vector3D& lhs, const Vector3D& rhs )
{
    f64_t   ret                         = std::inner_product( lhs.begin(),
        lhs.end(), rhs.begin(), f64_t( 0 ) );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      ostream insertion operator for three-dimensional vector.

@param      lhs The ostream to insert into.
@param      rhs The vector to insert.

@return     Returns a reference to the ostream.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Vector3D& rhs )
{
    std::basic_ostringstream< charT, traits >   s;
    s.flags( lhs.flags() );
    s.imbue( lhs.getloc() );
    s.precision( lhs.precision() );

    typename Vector3D::const_iterator   i   = rhs.begin();
    s << '(' << *i;

    for( ++i; i != rhs.end(); ++i )
    {
        s << ',' << *i;
    }
    s << ')';

    return lhs << s.str();
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      istream extraction operator for three-dimensional vector.

@param      lhs The istream to extract from.
@param      rhs The vector to extract.

@return     Returns a reference to the istream.

@note       The vector can be of the form:
@note       '*' can be one of '(', '<', '[', '{'.
@note       '$' can be one of ')', '>', ']', '}'.
            @li * 1, 2, ..., D $
            @li 1, 2, ..., D
            @li 1 2 ... D
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Vector3D& rhs )
{
    // If this character is a delimiter, get rid of it.
    charT   c;
    if( lhs >> c && '(' != c && '<' != c && '[' != c && '{' != c )
    {
        lhs.putback( c );
    }

    // Next, we should get the components.
    Vector3D    temp;
    for( typename Vector3D::iterator i = temp.begin(); i != temp.end(); ++i )
    {
        lhs >> *i;
        // There might be a comma or a ending delimiter next, so ditch it.
        if( lhs.eof() )
        {
            break;
        }
        if( lhs >> c && ',' != c && ')' != c && '>' != c && ']' != c
            && '}' != c )
        {
            lhs.putback( c );
        }
    }
    // If all went well, save the extracted vector.
    if( lhs.good() || (lhs.eof() && !lhs.bad()) )
    {
        rhs                             = temp;
    }
    // Set the stream fail state.
    else
    {
        lhs.setstate( std::ios_base::failbit );
    }

    return lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Default ctor for three-dimensional line.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Line3D::Line3D( void )
: point(), vector()
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the components (point and vector) of the
            three-dimensional line to be specified.

@param      p A point on the line.
@param      v A vector parallel to the line.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Line3D::Line3D( const Point3D& p, const Vector3D& v )
: point( p ), vector( v )
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Default ctor for two-dimensional line segment.
*/
////////////////////////////////////////////////////////////////////////////////
inline
LineSeg3D::LineSeg3D( void )
: point1(), point2()
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the components (the two endpoints) of the
            three-dimensional line segment to be specified.

@param      p1 The first endpoint.
@param      p2 The second endpoint.
*/
////////////////////////////////////////////////////////////////////////////////
inline
LineSeg3D::LineSeg3D( const Point3D& p1, const Point3D& p2 )
: point1( p1 ), point2( p2 )
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Default ctor for three-dimensional ray.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Ray3D::Ray3D( void )
: origin(), direction()
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the components (origin and direction) of the
            three-dimensional ray to be specified.

@param      o The origin of the ray.
@param      d The direction of the ray.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Ray3D::Ray3D( const Point3D& o, const Vector3D& d )
: origin( o ), direction( d )
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Default ctor for three-dimensional box.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Box3D::Box3D( void )
: origin(), extent()
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the components (origin and extent) of the
            three-dimensional box to be specified.

@param      o The origin of the box.
@param      e The extent of the box.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Box3D::Box3D( const Point3D& o, const Point3D& e )
: origin( o ), extent( e )
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Default ctor for sphere.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Sphere3D::Sphere3D( void )
: center(), radius( 0 )
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the components (center and radius) of the sphere to be
            specified.

@param      c The center of the sphere.
@param      r The length of the radius of the sphere.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Sphere3D::Sphere3D( const Point3D& c, f32_t r )
: center( c ), radius( r )
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the a-, b-, c- and d-components of the plane to be
            specified.

@param      A The a-component.
@param      B The b-component.
@param      C The c-component.
@param      D The d-component.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Plane3D::Plane3D( f64_t A, f64_t B, f64_t C, f64_t D )
{
    iterator    it                      = begin();

    *it                                 = A;
    *++it                               = B;
    *++it                               = C;
    *++it                               = D;

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for plane.

@param      i The index of the component desired.

@return     Returns a reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t&
Plane3D::operator[]( uint32_t i )
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for const plane.

@param      i The index of the component desired.

@return     Returns a const reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
const f64_t&
Plane3D::operator[]( uint32_t i ) const
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines the normal to the plane.

@return     Returns a three-dimensional normal (not necessarily unit-length) to
            the plane.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D
Plane3D::normal( void ) const
{
    return Vector3D( m_components[A], m_components[B], m_components[C] );
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a plane.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Plane3D::iterator
Plane3D::begin( void )
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a const plane.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Plane3D::const_iterator
Plane3D::begin( void ) const
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Plane3D::iterator
Plane3D::end( void )
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence for const object.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Plane3D::const_iterator
Plane3D::end( void ) const
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Default ctor for three-dimensional triangle.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Triangle3D::Triangle3D( void )
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the components (the three vertex points) of the
            three-dimensional triangle to be specified.

@param      p1 The first point of the triangle.
@param      p2 The second point of the triangle.
@param      p3 The third point of the triangle.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Triangle3D::Triangle3D( const Point3D& p1, const Point3D& p2,
    const Point3D& p3 )
{
    iterator    it                      = begin();

    *it                                 = p1;
    *++it                               = p2;
    *++it                               = p3;

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for three-dimensional triangle.

@param      i The index of the vertex point desired.

@return     Returns a reference to the vertex point desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D&
Triangle3D::operator[]( uint32_t i )
{
    return m_points[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for const three-dimensional triangle.

@param      i The index of the vertex point desired.

@return     Returns a const reference to the vertex point desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
const Point3D&
Triangle3D::operator[]( uint32_t i ) const
{
    return m_points[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the vertex points of a
            three-dimensional triangle.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Triangle3D::iterator
Triangle3D::begin( void )
{
    return m_points + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the vertex points of a const
            three-dimensional triangle.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Triangle3D::const_iterator
Triangle3D::begin( void ) const
{
    return m_points + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Triangle3D::iterator
Triangle3D::end( void )
{
    return m_points + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence for const object.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Triangle3D::const_iterator
Triangle3D::end( void ) const
{
    return m_points + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the x-, y-, z-, and w-components of the
            four-dimensional point to be specified.

@param      X The x-component.
@param      Y The y-component.
@param      Z The z-component.
@param      W The w-component.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D::Point4D( f64_t X, f64_t Y, f64_t Z, f64_t W )
{
    iterator    it                      = begin();

    *it                                 = X;
    *++it                               = Y;
    *++it                               = Z;
    *++it                               = W;

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (addition) operator. Adds the given
            four-dimensional vector to the point and returns the result.

@param      rhs The vector to be added to the point.

@return     Returns the result of adding the point and vector together (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D&
Point4D::operator+=( const Vector4D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::plus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (subtraction) operator. Subtracts the given
            four-dimensional vector from the point and returns the result.

@param      rhs The vector to be subtracted from the point.

@return     Returns the result of subtracting the vector from the point (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D&
Point4D::operator-=( const Vector4D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::minus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for four-dimensional point.

@param      i The index of the component desired.

@return     Returns a reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t&
Point4D::operator[]( uint32_t i )
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for const four-dimensional point.

@param      i The index of the component desired.

@return     Returns a const reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
const f64_t&
Point4D::operator[]( uint32_t i ) const
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a
            four-dimensional point.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D::iterator
Point4D::begin( void )
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a const
            four-dimensional point.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D::const_iterator
Point4D::begin( void ) const
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D::iterator
Point4D::end( void )
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence for const object.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D::const_iterator
Point4D::end( void ) const
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Equality operator for four-dimensional point.

@param      lhs The first point to compare.
@param      rhs The second point to compare.

@return     Returns true if the two points have the same components, false
            otherwise.

@warning    Demands strict equality. (May wish to introduce an epsilon value.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator==( const Point4D& lhs, const Point4D& rhs )
{
    bool    ret                         = std::equal( lhs.begin(), lhs.end(),
        rhs.begin() );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Inequality operator for four-dimensional point.

@param      lhs The first point to compare.
@param      rhs The second point to compare.

@return     Returns true if the two points are not the same, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator!=( const Point4D& lhs, const Point4D& rhs )
{
    bool    ret                         = !(lhs == rhs);

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Addition operator. Adds a four-dimensional vector to a point and
            returns the result.

@param      lhs The point.
@param      rhs The vector.

@return     Returns the result of adding the vector to the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D
operator+( const Point4D& lhs, const Vector4D& rhs )
{
    Point4D ret( lhs );

    ret                                 += rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Addition operator. Adds a four-dimensional vector to a point and
            returns the result.

@param      lhs The vector.
@param      rhs The point.

@return     Returns the result of adding the vector to the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D
operator+( const Vector4D& lhs, const Point4D& rhs )
{
    // Reorder and reuse.
    return rhs + lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Subtraction operator. Subtracts a four-dimensional vector from 
            point and returns the result.

@param      lhs The point.
@param      rhs The vector.

@return     Returns the result of subtracting the vector from the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D
operator-( const Point4D& lhs, const Vector4D& rhs )
{
    Point4D ret( lhs );

    ret                                 -= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Subtraction operator. Subtracts two four-dimensional points and
            returns the result.

@param      lhs The first point.
@param      rhs The second point.

@return     Returns the result of subtracting one point from another.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D
operator-( const Point4D& lhs, const Point4D& rhs )
{
    Vector4D    ret;

    std::transform( lhs.begin(), lhs.end(), rhs.begin(), ret.begin(),
        std::minus< f64_t >() );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      ostream insertion operator for four-dimensional point.

@param      lhs The ostream to insert into.
@param      rhs The point to insert.

@return     Returns a reference to the ostream.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Point4D& rhs )
{
    std::basic_ostringstream< charT, traits >   s;
    s.flags( lhs.flags() );
    s.imbue( lhs.getloc() );
    s.precision( lhs.precision() );

    typename Point4D::const_iterator    i   = rhs.begin();
    s << '(' << *i;

    for( ++i; i != rhs.end(); ++i )
    {
        s << ',' << *i;
    }
    s << ')';

    return lhs << s.str();
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      istream extraction operator for four-dimensional point.

@param      lhs The istream to extract from.
@param      rhs The point to extract.

@return     Returns a reference to the istream.

@note       The point can be of the form:
@note       '*' can be one of '(', '<', '[', '{'.
@note       '$' can be one of ')', '>', ']', '}'.
            @li * 1, 2, ..., D $
            @li 1, 2, ..., D
            @li 1 2 ... D
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Point4D& rhs )
{
    // If this character is a delimiter, get rid of it.
    charT   c;
    if( lhs >> c && '(' != c && '<' != c && '[' != c && '{' != c )
    {
        lhs.putback( c );
    }

    // Next, we should get the components.
    Point4D     temp;
    for( typename Point4D::iterator i = temp.begin(); i != temp.end(); ++i )
    {
        lhs >> *i;
        // There might be a comma or a ending delimiter next, so ditch it.
        if( lhs.eof() )
        {
            break;
        }
        if( lhs >> c && ',' != c && ')' != c && '>' != c && ']' != c
            && '}' != c )
        {
            lhs.putback( c );
        }
    }
    // If all went well, save the extracted point.
    if( lhs.good() || (lhs.eof() && !lhs.bad()) )
    {
        rhs                             = temp;
    }
    // Set the stream fail state.
    else
    {
        lhs.setstate( std::ios_base::failbit );
    }

    return lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the x-, y-, and z-components of the four-dimensional
            vector to be specified.

@param      X The x-component.
@param      Y The y-component.
@param      Z The z-component.
@param      W The w-component.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D::Vector4D( f64_t X, f64_t Y, f64_t Z, f64_t W )
{
    iterator    it                      = begin();

    *it                                 = X;
    *++it                               = Y;
    *++it                               = Z;
    *++it                               = W;

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (addition) operator. Adds the given
            four-dimensional vector and returns the result.

@param      rhs The vector to be added.

@return     Returns the result of adding the two vectors (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D&
Vector4D::operator+=( const Vector4D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::plus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (subtraction) operator. Subtracts the given
            four-dimensional vector and returns the result.

@param      rhs The vector to be subtracted.

@return     Returns the result of subtracting the vector (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D&
Vector4D::operator-=( const Vector4D& rhs )
{
    std::transform( begin(), end(), rhs.begin(), begin(),
        std::minus< f64_t >() );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (multiplication) operator. Multiplies the
            four-dimensional vector by the given scalar and returns the result.

@param      rhs The scaling factor for the vector.

@return     Returns the now-scaled vector (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D&
Vector4D::operator*=( const f64_t& rhs )
{
    std::transform( begin(), end(), begin(),
        std::bind2nd( std::multiplies< f64_t >(), rhs ) );

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for four-dimensional vector.

@param      i The index of the component desired.

@return     Returns a reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t&
Vector4D::operator[]( uint32_t i )
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for const four-dimensional vector.

@param      i The index of the component desired.

@return     Returns a const reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
const f64_t&
Vector4D::operator[]( uint32_t i ) const
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Length-determining method for four-dimensional vector.

@return     Returns the length of the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t
Vector4D::length( void ) const
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::sqrt;
#endif
    f64_t   ret                         = sqrt( Dot( *this, *this ) );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a
            four-dimensional vector.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D::iterator
Vector4D::begin( void )
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the components of a const
            four-dimensional vector.

@return     Returns iterator to beginning of sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D::const_iterator
Vector4D::begin( void ) const
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D::iterator
Vector4D::end( void )
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence for const object.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D::const_iterator
Vector4D::end( void ) const
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Equality operator for four-dimensional vector.

@param      lhs The first vector to compare.
@param      rhs The second vector to compare.

@return     Returns true if the two vectors have the same components, false
            otherwise.

@warning    Demands strict equality. (May wish to introduce an epsilon value.)
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator==( const Vector4D& lhs, const Vector4D& rhs )
{
    bool    ret                         = std::equal( lhs.begin(), lhs.end(),
        rhs.begin() );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Inequality operator for four-dimensional vector.

@param      lhs The first vector to compare.
@param      rhs The second vector to compare.

@return     Returns true if the two vectors are not the same, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
inline
bool
operator!=( const Vector4D& lhs, const Vector4D& rhs )
{
    bool    ret                         = !(lhs == rhs);

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Addition operator. Adds two four-dimensional vectors and returns
            the result.

@param      lhs The first vector.
@param      rhs The second vector.

@return     Returns the result of adding the two vectors.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D
operator+( const Vector4D& lhs, const Vector4D& rhs )
{
    Vector4D    ret( lhs );

    ret                                 += rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Subtraction operator. Subtracts two four-dimensional vectors and
            returns the result.

@param      lhs The first vector.
@param      rhs The second vector.

@return     Returns the result of subtracting the vectors.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D
operator-( const Vector4D& lhs, const Vector4D& rhs )
{
    Vector4D    ret( lhs );

    ret                                 -= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies the four-dimensional vector by
            the given scalar and returns the result.

@param      lhs The vector.
@param      rhs The scalar.

@return     Returns the result of scaling the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D
operator*( const Vector4D& lhs, const f64_t& rhs )
{
    Vector4D    ret( lhs );

    ret                                 *= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies the four-dimensional vector by
            the given scalar and returns the result.

@param      lhs The scalar.
@param      rhs The vector.

@return     Returns the result of scaling the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D
operator*( const f64_t& lhs, const Vector4D& rhs )
{
    // Reorder and reuse.
    return rhs*lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Dot product for four-dimensional vector.

@param      lhs The first vector.
@param      rhs The second vector.

@return     Returns the dot product of the two vectors.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t
Dot( const Vector4D& lhs, const Vector4D& rhs )
{
    f64_t   ret                         = std::inner_product( lhs.begin(),
        lhs.end(), rhs.begin(), f64_t( 0 ) );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      ostream insertion operator for four-dimensional vector.

@param      lhs The ostream to insert into.
@param      rhs The vector to insert.

@return     Returns a reference to the ostream.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Vector4D& rhs )
{
    std::basic_ostringstream< charT, traits >   s;
    s.flags( lhs.flags() );
    s.imbue( lhs.getloc() );
    s.precision( lhs.precision() );

    typename Vector4D::const_iterator   i   = rhs.begin();
    s << '(' << *i;

    for( ++i; i != rhs.end(); ++i )
    {
        s << ',' << *i;
    }
    s << ')';

    return lhs << s.str();
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      istream extraction operator for four-dimensional vector.

@param      lhs The istream to extract from.
@param      rhs The vector to extract.

@return     Returns a reference to the istream.

@note       The vector can be of the form:
@note       '*' can be one of '(', '<', '[', '{'.
@note       '$' can be one of ')', '>', ']', '}'.
            @li * 1, 2, ..., D $
            @li 1, 2, ..., D
            @li 1 2 ... D
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Vector4D& rhs )
{
    // If this character is a delimiter, get rid of it.
    charT   c;
    if( lhs >> c && '(' != c && '<' != c && '[' != c && '{' != c )
    {
        lhs.putback( c );
    }

    // Next, we should get the components.
    Vector4D    temp;
    for( typename Vector4D::iterator i = temp.begin(); i != temp.end(); ++i )
    {
        lhs >> *i;
        // There might be a comma or a ending delimiter next, so ditch it.
        if( lhs.eof() )
        {
            break;
        }
        if( lhs >> c && ',' != c && ')' != c && '>' != c && ']' != c
            && '}' != c )
        {
            lhs.putback( c );
        }
    }
    // If all went well, save the extracted vector.
    if( lhs.good() || (lhs.eof() && !lhs.bad()) )
    {
        rhs                             = temp;
    }
    // Set the stream fail state.
    else
    {
        lhs.setstate( std::ios_base::failbit );
    }

    return lhs;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Converts an angle from degree measure to radian measure.

@param      Degrees The angle measured in degrees.

@return     Returns the angle measured in radians.
*/
////////////////////////////////////////////////////////////////////////////////
template< class T >
T
ToRadians( const T& Degrees )
{
    f64_t   ret                         = (PI*Degrees)/180.0f;

    return T( ret );
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if all of the elements in a sequence equal zero.

@return     Returns true if all elements of the sequence are zero, false
            otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
template< class Sequence >
bool
IsZero( const Sequence& s )
{
    bool ret                            = true;

    for( typename Sequence::const_iterator i = s.begin(); i != s.end(); ++i )
    {
        if( 0 != *i )
        {
            ret                         = false;
            break;
        }
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines the minimum of two objects using the less-than operator.

@param      first The first object to compare.
@param      second The second object to compare.

@return     Returns the minimum of the two objects.
*/
////////////////////////////////////////////////////////////////////////////////
template< class T >
const T&
MIN( const T& first, const T& second )
{
    return first < second ? first : second;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines the maximum of two objects using the greater-than
            operator.

@param      first The first object to compare.
@param      second The second object to compare.

@return     Returns the maximum of the two objects.
*/
////////////////////////////////////////////////////////////////////////////////
template< class T >
const T&
MAX( const T& first, const T& second )
{
    return first > second ? first : second;
}

} // namespace Geometry


// Restore the warnings to their previous state.
#if defined( _MSC_VER ) && ( 1300 >= _MSC_VER )
#pragma warning( pop )
#endif

#endif // #if !defined( GEOMETRY_HPP_ )
