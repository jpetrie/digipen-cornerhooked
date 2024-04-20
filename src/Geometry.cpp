////////////////////////////////////////////////////////////////////////////////
/**
@file       Geometry.cpp
@brief      CS300 Advanced Computer Graphics (I)

            This file contains geometric function definitions used in
            geometrical calculus.

@author     
@date       Started:    January     07, 2004 15:38
@date       Modified:   February    05, 2004 21:46
@note       Instructor: Xin Li
@note       Copyright © 2004 DigiPen Institute of Technology
*/
////////////////////////////////////////////////////////////////////////////////

#include "main.h"

#include "Geometry.hpp"

namespace // 'anonymous'
{

////////////////////////////////////////////////////////////////////////////////
/**
@struct     AbsVal

@brief      Used to disambiguate the call to the overloaded std::fabs function
            when used as a function template argument.
*/
////////////////////////////////////////////////////////////////////////////////
struct AbsVal : public std::unary_function< f64_t, f64_t >
{
public:
    inline
    f64_t
    operator()( f64_t Val ) const;

};

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Overloaded function-call operator. Returns the absolute value of its
            argument.

@param      Val The value to compute the absolute value of.

@return     Returns the absolute value of the argument.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t
AbsVal::operator()( f64_t Val ) const
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::fabs;
#endif
    return fabs( Val );
}

} // namespace 'anonymous'


namespace Geometry
{

/// A small number (useful for comparing floating-point numbers to zero.)
const f64_t epsilon                     = std::numeric_limits<
    float >::epsilon();

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Calculates the normal (unit-length) vector in the same direction
            as the two-dimensional vector.

@return     Returns the unit-length vector in the same direction as the vector.

@throw      Throws ZeroDivide if a division by zero would result in performing
            the operation.
*/
////////////////////////////////////////////////////////////////////////////////
Vector2D
Vector2D::normal( void ) const throw( ZeroDivide )
{
    f64_t   len                         = length();

    if( !len )
    {
        throw ZeroDivide();
    }

    Vector2D    ret;

    std::transform( begin(), end(), ret.begin(),
        std::bind2nd( std::divides< f64_t >(), len ) );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if two two-dimensional lines intersect, and if so, where.

@param      l1 The first line.
@param      l2 The second line.
@param      p Optional pointer to storage location for potential intersection
            point. (Pass zero (the default) if the value is not needed.)

@return     Returns true if the lines intersect, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Intersects( const Line2D& l1, const Line2D& l2, Point2D *p )
{
    bool    ret                         = false;

    /* special case #1: parallel lines (i.e. cross product equals zero) */
    f64_t   intersect                   = l2.vector[X]*l1.vector[Y]
        - l1.vector[X]*l2.vector[Y];

    /* intersection condition: p2q1 != p1q2 */
    if( intersect )
    {
        if( p )
        {
            (*p)[X]                     = (l1.vector[X]*l2.vector[X]
                *(l2.point[Y] - l1.point[Y])
                + l2.vector[X]*l1.vector[Y]*l1.point[X]
                - l1.vector[X]*l2.vector[Y]*l2.point[X])
                / intersect;
            (*p)[Y]                     = (l1.vector[Y]*l2.vector[Y]
                *(l1.point[X] - l2.point[X])
                + l2.vector[X]*l1.vector[Y]*l2.point[Y]
                - l1.vector[X]*l2.vector[Y]*l1.point[Y])
                / intersect;
        }

        /* indicate the intersection */
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if two two-dimensional line segments intersect, and if
            so, where.

@param      l1 The first line segment.
@param      l2 The second line segment.
@param      p Optional pointer to storage location for potential intersection
            point. (Pass zero (the default) if the value is not needed.)

@return     Returns true if the line segments intersect, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Intersects( const LineSeg2D& l1, const LineSeg2D& l2, Point2D *p )
{
    bool    ret                         = false;
    /* convert line segments into lines so code can be leveraged */
    Line2D  line1;
    Line2D  line2;
    
    line1.point[X]                      = l1.point1[X];
    line1.point[Y]                      = l1.point1[Y];
    line1.vector[X]                     = l1.point1[X] - l1.point2[X];
    line1.vector[Y]                     = l1.point1[Y] - l1.point2[Y];

    line2.point[X]                      = l2.point1[X];
    line2.point[Y]                      = l2.point1[Y];
    line2.vector[X]                     = l2.point1[X] - l2.point2[X];
    line2.vector[Y]                     = l2.point1[Y] - l2.point2[Y];

    Point2D     M;

    /* leverage code: */
    if( Intersects( line1, line2, &M ) )
    {
        /* containment tests */
        if( MIN( l1.point1[X], l1.point2[X] ) <= M[X]
            && M[X] <= MAX( l1.point1[X], l1.point2[X] )
            && MIN( l1.point1[Y], l1.point2[Y] ) <= M[Y]
            && M[Y] <= MAX( l1.point1[Y], l1.point2[Y] )
            && MIN( l2.point1[X], l2.point2[X] ) <= M[X]
            && M[X] <= MAX( l2.point1[X], l2.point2[X] )
            && MIN( l2.point1[Y], l2.point2[Y] ) <= M[Y]
            && M[Y] <= MAX( l2.point1[Y], l2.point2[Y] ) )
        {
            if( p )
            {
                *p                      = M;
            }
            ret                         = true;
        }
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines the distance between a three-dimensional point and line.

@param      p The point.
@param      l The line.

@return     Returns the distance between the point and line.
*/
////////////////////////////////////////////////////////////////////////////////
f64_t
Distance( const Point3D& p, const Line3D& l )
{
    f64_t   ret                         = 0;

    /*
    **     |u x v|
    ** d = -------
    **       |v|
    */
    f64_t   v_len                       = l.vector.length();

    /* special case #1: length of v is zero */
    if( v_len )
    {
        /* vector u runs between point `p' and a point on line `l' */
        Vector3D    u( l.point - p );

        /*
        ** vector w is the cross product of the direction vector of line `l'
        ** and the vector u
        */
        Vector3D    w( Cross( l.vector, u ) );

        ret                             = w.length()/v_len;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines the distance between a three-dimensional point and plane.

@param      p The point.
@param      m The plane.

@return     Returns the distance between the point and plane.
*/
////////////////////////////////////////////////////////////////////////////////
f64_t
Distance( const Point3D& p, const Plane3D& m )
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::fabs;
#endif
    f64_t   ret                         = 0;

    /* n_len is the length of the normal to plane `p' */
    f64_t   n_len                       = m.normal().length();

    /* special case #1: length of vector `n' is zero */
    if( n_len )
    {
        /* make sure that the return value is a positive value */
        ret                             = fabs( m[A]*p[X] + m[B]*p[Y]
            + m[C]*p[Z] + m[D] ) / n_len;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Calculates the normal (unit-length) vector in the same direction
            as the three-dimensional vector.

@return     Returns the unit-length vector in the same direction as the vector.

@throw      Throws ZeroDivide if a division by zero would result in performing
            the operation.
*/
////////////////////////////////////////////////////////////////////////////////
Vector3D
Vector3D::normal( void ) const throw( ZeroDivide )
{
    f64_t   len                         = length();

    if( !len )
    {
        //throw ZeroDivide();
		return Vector3D();
    }

    Vector3D    ret;

    std::transform( begin(), end(), ret.begin(),
        std::bind2nd( std::divides< f64_t >(), len ) );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Cross product for three-dimensional vector.

@param      v1 The first vector.
@param      v2 The second vector.

@return     Returns the cross product of the two vectors.
*/
////////////////////////////////////////////////////////////////////////////////
Vector3D
Cross( const Vector3D& v1, const Vector3D& v2 )
{
    Vector3D    ret;

    ret[X]                                = v1[Y]*v2[Z] - v1[Z]*v2[Y];
    ret[Y]                                = v1[Z]*v2[X] - v1[X]*v2[Z];
    ret[Z]                                = v1[X]*v2[Y] - v1[Y]*v2[X];

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines the angle between two three-dimensional lines.

@param      l1 The first line.
@param      l2 The second line.

@return     Returns the angle between the two lines.
*/
////////////////////////////////////////////////////////////////////////////////
f64_t
AngleBetween( const Line3D& l1, const Line3D& l2 )
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::acos;
#endif
    f64_t   ret                         = 0;

    /*
    ** l_1_len and l_2_len are the lengths of the direction vectors for the
    ** lines `l1' and `l2', respectively
    */
    f64_t   l_1_len                     = l1.vector.length();
    f64_t   l_2_len                     = l2.vector.length();

    /* special case #1: either direction vector has length zero */
    if( l_1_len && l_2_len )
    {
        ret                             = acos( Dot( l1.vector,
            l2.vector ) / (l_1_len*l_2_len) );
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines the angle between a three-dimensional line and plane.

@param      l The line.
@param      p The plane.

@return     Returns the angle between the line and plane.
*/
////////////////////////////////////////////////////////////////////////////////
f64_t
AngleBetween( const Line3D& l, const Plane3D& p )
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::asin;
#endif
    f64_t   ret                         = 0;

    const Vector3D  n                   = p.normal();
    f64_t   dot                         = Dot( l.vector, n );

    f64_t   vlen                        = l.vector.length();
    f64_t   nlen                        = n.length();
    f64_t   vn                          = vlen*nlen;

    if( vn )
    {
        ret                             = asin( dot/vn );
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if two three-dimensional lines are coplanar.

@param      l1 The first line.
@param      l2 The second line.

@return     Returns true if the two lines are coplanar, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Coplanar( const Line3D& l1, const Line3D& l2 )
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::fabs;
#endif
    bool    ret                         = false;

    Vector3D    N                       = Cross( l1.vector, l2.vector );
    Vector3D    M                       = l1.point - l2.point;

    f64_t       dot                     = fabs( Dot( N, M ) );

    if( dot < epsilon )
    {
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional line and plane intersect and if
            so, where.

@param      l The line.
@param      p The plane.
@param      m Optional pointer to storage location for potential intersection
            point. (Pass zero (the default) if the value is not needed.)

@return     Returns true if the line and plane intersect, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Intersects( const Line3D& l, const Plane3D& p, Point3D *m )
{
    bool        ret                     = false;
    f64_t       dot                     = Dot( l.vector, p.normal() );

    /* are the line and the plane parallel? */
    if( dot )
    {
        f64_t   t                       = -(p[A]*l.point[X] + p[B]*l.point[Y]
            + p[C]*l.point[Z] + p[D])/dot;

        if( m )
        {
            *m                          = l.point + t*l.vector;
        }
        /* indicate intersection */
        ret                             = true;
    }

    return ret;
}
////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional ray and plane intersect and if
            so, where.

@param      r The ray.
@param      p The plane.
@param      m Optional pointer to storage location for potential intersection
            point. (Pass zero (the default) if the value is not needed.)

@return     Returns true if the ray and plane intersect, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Intersects( const Ray3D& r, const Plane3D& p, Point3D *m, float *time)
{
    bool        ret                     = false;
    f64_t       dot                     = Dot( r.direction, p.normal() );
    
    /* are the line and the plane parallel? */
    if( dot < 0 )
    {
        f64_t   t                       = -(p[A]*r.origin[X] + p[B]*r.origin[Y]
        + p[C]*r.origin[Z] + p[D])/dot;
        
        if( m )
        {
            *m                          = r.origin + t*r.direction;
        }
        /* indicate intersection */
        ret                             = true;
        if(time)
            *time = t;
    }

    return ret;
}
////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if two three-dimensional lines are parallel.

@param      l1 The first line.
@param      l2 The second line.

@return     Returns true if the two lines are parallel, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Parallel( const Line3D& l1, const Line3D& l2 )
{
    bool    ret                         = false;

    Vector3D    v1                      = l1.vector.normal();
    Vector3D    v2                      = l2.vector.normal();

    std::transform( v1.begin(), v1.end(), v1.begin(), AbsVal() );
    std::transform( v2.begin(), v2.end(), v2.begin(), AbsVal() );

    if( v1 == v2 )
    {
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if two three-dimensional lines are perpendicular.

@param      l1 The first line.
@param      l2 The second line.

@return     Returns true if the two lines are perpendicular, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Perpendicular( const Line3D& l1, const Line3D& l2 )
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::fabs;
#endif
    bool    ret                         = false;

    f64_t   dot                         = fabs( Dot( l1.vector, l2.vector ) );

    if( dot < epsilon )
    {
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional line and plane are parallel.

@param      l The line.
@param      p The plane.

@return     Returns true if the line and plane are parallel, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Parallel( const Line3D& l, const Plane3D& p )
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::fabs;
#endif
    bool    ret                         = false;

    f64_t   dot                         = fabs( Dot( l.vector, p.normal() ) );

    if( dot < epsilon )
    {
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional line and plane are perpendicular.

@param      l The line.
@param      p The plane.

@return     Returns true if the line and plane are perpendicular, false
            otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Perpendicular( const Line3D& l, const Plane3D& p )
{
    bool    ret                         = false;

    Vector3D    v1                      = l.vector.normal();
    Vector3D    v2                      = p.normal().normal();

    std::transform( v1.begin(), v1.end(), v1.begin(), AbsVal() );
    std::transform( v2.begin(), v2.end(), v2.begin(), AbsVal() );

    if( v1 == v2 )
    {
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional point is contained within a line
            segment.

@param      p The point.

@return     Returns true if the point is contained within the line segment,
            false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
LineSeg3D::contains( const Point3D& p ) const
{
    bool    ret                         = false;

    /* simple containment tests */
    if( MIN( point1[X], point2[X] ) <= p[X]
        && p[X] <= MAX( point1[X], point2[X] )
        && MIN( point1[Y], point2[Y] ) <= p[Y]
        && p[Y] <= MAX( point1[Y], point2[Y] )
        && MIN( point1[Z], point2[Z] ) <= p[Z]
        && p[Z] <= MAX( point1[Z], point2[Z] ) )
    {
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional line segment and triangle
            intersect and if so, where.

@param      l The line segment.
@param      t The triangle.
@param      p Optional pointer to storage location for potential intersection
            point. (Pass zero (the default) if the value is not needed.)

@return     Returns true if the line segment and triangle intersect, false
            otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Intersects( const LineSeg3D& l, const Triangle3D& t, Point3D *p )
{
    bool        ret                     = false;

    /* convert line segment to line, so that we can leverage code */
    Vector3D    v1                      = t[0] - t[1];
    Vector3D    v2                      = t[2] - t[1];
    Vector3D    n                       = Cross( v1, v2 );

    Line3D      line( l.point1, l.point1 - l.point2 );

    /* build the plane */
    Plane3D     plane( n[X], n[Y], n[Z] );
    plane[D]                        = -(plane[A]*t[0][X] + plane[B]*t[0][Y]
        + plane[C]*t[0][Z]);

    Point3D     P;

    /* does the line intersects the plane? */
    if( Intersects( line, plane, &P ) )
    {
        /* is the intersection point contained by the line segment? */
        if( l.contains( P ) )
        {
            /* is the intersection point contained by the triangle? */
            if( t.contains( P ) )
            {
                if( p )
                {
                    *p              = P;
                }
                ret                 = true;
            }
        }
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional point is contained within a ray.

@param      p The point.
@param      t Optional pointer to storage location for t-value of intersection.
            (Pass zero (the default) if the value is not needed.)
*/
////////////////////////////////////////////////////////////////////////////////
bool
Ray3D::contains( const Point3D& p, f64_t *t ) const
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::fabs;
#endif
    bool    ret                         = true;
    f64_t   y_t                         = (p[Y] - origin[Y]) / direction[Y];
    f64_t   z_t                         = (p[Z] - origin[Z]) / direction[Z];

    f64_t   y_diff;
    f64_t   z_diff;
    f64_t   T;

    T                                   = (p[X] - origin[X])/direction[X];

    y_diff                              = fabs( T - y_t );
    z_diff                              = fabs( T - z_t );

    if( y_diff > epsilon || z_diff > epsilon || T < epsilon )
    {
        ret                             = false;
    }
    else if( t )
    {
        *t                              = T;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional ray and sphere intersect and if
            so, where.

@param      r The ray.
@param      s The sphere.
@param      p Optional pointer to storage location for potential intersection
            point(s). (Pass zero (the default) if the value(s) are not needed.)

@return     Returns the number of intersection points.
*/
////////////////////////////////////////////////////////////////////////////////
int32_t
Intersects( const Ray3D& r, const Sphere3D& s,
std::pair< Point3D, Point3D > *p, float* time)
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::sqrt;
#endif
    int32_t ret                         = 0;

    /* p² + q² + r² = V² */
    f64_t   a                           = Dot( r.direction, r.direction );
    /* 2V·(P-O) */
    f64_t   b                           = 2*Dot( r.direction,
        r.origin - s.center );
    /* O² - 2O·P + P² - r² */
    f64_t   c                           = Dot( s.center - Point3D(),
        s.center - Point3D() ) - 2*Dot( s.center - Point3D(),
        r.origin - Point3D() )
        + Dot( r.origin - Point3D(), r.origin - Point3D() )
        - s.radius*s.radius;
    /* b² - 4ac */
    f64_t   discriminant                = b*b - 4*a*c;

    f64_t   t;
    if(time)
        *time = 0.0;
    /* intersection condition */
    if( discriminant >= 0 )
    {
        /* tangential condition */
        if( discriminant <= epsilon )
        {
            t                           = -b / (2*a);
            ret                         = 1;
            if(time)
                *time = t;
        }
        else
        {
            f64_t   sqrt_discrim        = sqrt( discriminant );
            t                           = ( -b - sqrt_discrim ) / (2*a);
            if(time)
                *time = t;
            if( p )
            {
                p->second               = r.origin + t*r.direction;
            }
            t                           = ( -b + sqrt_discrim ) / (2*a);
            if(time)
            {
                if(t < *time)
                    *time = t;
            }
            ret                         = 2;
        }
        if( p )
        {
            p->first                    = r.origin + t*r.direction;
        }
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional ray and triangle intersect and if
            so, where.

@param      r The ray.
@param      t The triangle.
@param      p Optional pointer to storage location for potential intersection
            point. (Pass zero (the default) if the value is not needed.)

@return     Returns true if the ray and triangle intersect, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Intersects( const Ray3D& r, const Triangle3D& t, Point3D *p )
{
    bool        ret                     = false;
    Vector3D    vector1                 = t[1] - t[0];
    Vector3D    vector2                 = t[2] - t[0];
    Vector3D    crossProduct            = Cross( vector1, vector2 );
    Plane3D     plane( crossProduct[X], crossProduct[Y], crossProduct[Z] );
    plane[D]                            = -(crossProduct[X]*t[0][X]
        + crossProduct[Y]*t[0][Y] + crossProduct[Z]*t[0][Z]);

    Line3D      line( r.origin, r.direction );

    Point3D     outPoint;

    if( Intersects( line, plane, &outPoint ) )
    {
        if( r.contains( outPoint ) )
        {
            if( t.contains( outPoint ) )
            {
                if( p )
                {
                    *p                  = outPoint;
                }
                ret                     = true;
            }
        }
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional ray and box intersect and if so,
            where.

@param      r The ray.
@param      b The box.
@param      p Optional pointer to storage location for potential intersection
            point(s). (Pass zero (the default) if the value(s) are not needed.)

@return     Returns the number of intersection points.
*/
////////////////////////////////////////////////////////////////////////////////
int32_t
Intersects( const Ray3D& r, const Box3D& b,
    std::pair< Point3D, Point3D > *p )
{
    int32_t     ret                     = 0;

    f64_t       k_min;
    f64_t       k_max;

    Point3D     min_int;
    Point3D     max_int;
    Point3D     outPoint[2];

    /* special case #1: p == 0 (doesn't intersect yz plane) */
    if( r.direction[X] )
    {
        k_min                           = (b.origin[X] - r.origin[X])
            / r.direction[X];
        min_int[X]                      = b.origin[X];
        min_int[Y]                      = r.origin[Y]
            + r.direction[Y]*k_min;
        min_int[Z]                      = r.origin[Z]
            + r.direction[Z]*k_min;

        /* intersection condition #1: point is within the box */
        if( b.contains( min_int ) )
        {
            /* intersection condition #2: contained by ray */
            if( r.contains( min_int ) )
            {
                outPoint[ret]           = min_int;
                ret++;
            }
        }
        k_max                           = (b.extent[X] - r.origin[X])
            / r.direction[X];
        max_int[X]                      = b.extent[X];
        max_int[Y]                      = r.origin[Y]
            + r.direction[Y]*k_max;
        max_int[Z]                      = r.origin[Z]
            + r.direction[Z]*k_max;

        /* intersection condition #1: point is within the box */
        if( b.contains( max_int ) )
        {
            /* intersection condition #2: contained by ray */
            if( r.contains( max_int ) )
            {
                outPoint[ret]           = max_int;
                ret++;
            }
        }
    }

    /* special case #1: q == 0 (doesn't intersect xz plane) */
    if( 2 > ret && r.direction[Y] )
    {
        k_min                           = (b.origin[Y] - r.origin[Y])
            / r.direction[Y];
        min_int[X]                      = r.origin[X]
            + r.direction[X]*k_min;
        min_int[Y]                      = b.origin[Y];
        min_int[Z]                      = r.origin[Z]
            + r.direction[Z]*k_min;

        /* intersection condition #1: point is within the box */
        if( b.contains( min_int ) )
        {
            /* intersection condition #2: contained by ray */
            if( r.contains( min_int ) )
            {
                outPoint[ret]           = min_int;
                ret++;
            }
        }
        if( 2 > ret )
        {
            k_max                       = (b.extent[Y] - r.origin[Y])
                / r.direction[Y];
            max_int[X]                  = r.origin[X]
                + r.direction[X]*k_max;
            max_int[Y]                  = b.extent[Y];
            max_int[Z]                  = r.origin[Z]
                + r.direction[Z]*k_max;

            /* intersection condition #1: point is within the box */
            if( b.contains( max_int ) )
            {
                /* intersection condition #2: contained by ray */
                if( r.contains( max_int ) )
                {
                    outPoint[ret]       = max_int;
                    ret++;
                }
            }
        }
    }

    /* special case #1: r == 0 (doesn't intersect xy plane) */
    if( 2 > ret && r.direction[Z] )
    {
        k_min                           = (b.origin[Z] - r.origin[Z])
            / r.direction[Z];
        min_int[X]                      = r.origin[X]
            + r.direction[X]*k_min;
        min_int[Y]                      = r.origin[Y]
            + r.direction[Y]*k_min;
        min_int[Z]                      = b.origin[Z];

        /* intersection condition #1: point is within the box */
        if( b.contains( min_int ) )
        {
            /* intersection condition #2: contained by ray */
            if( r.contains( min_int ) )
            {
                outPoint[ret]           = min_int;
                ret++;
            }
        }
        if( 2 > ret )
        {
            k_max                       = (b.extent[Z] - r.origin[Z])
                / r.direction[Z];
            max_int[X]                  = r.origin[X]
                + r.direction[X]*k_max;
            max_int[Y]                  = r.origin[Y]
                + r.direction[Y]*k_max;
            max_int[Z]                  = b.extent[Z];

            /* intersection condition #1: point is within the box */
            if( b.contains( max_int ) )
            {
                /* intersection condition #2: contained by ray */
                if( r.contains( max_int ) )
                {
                    outPoint[ret]       = max_int;
                    ret++;
                }
            }
        }
    }

    if( ret && p )
    {
        p->first                        = outPoint[0];
        if( ret > 1 )
        {
            p->second                   = outPoint[1];
        }
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional point is contained within a box.

@param      p The point.

@return     Returns true if the point is contained within the box, false
            otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Box3D::contains( const Point3D& p ) const
{
    bool    ret                         = false;

    if( origin[X] <= p[X] && p[X] <= extent[X]
        && origin[Y] <= p[Y] && p[Y] <= extent[Y]
        && origin[Z] <= p[Z] && p[Z] <= extent[Z] )
    {
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines the angle between two three-dimensional planes.

@param      p1 The first plane.
@param      p2 The second plane.

@return     Returns the angle between the two planes.
*/
////////////////////////////////////////////////////////////////////////////////
f64_t
AngleBetween( const Plane3D& p1, const Plane3D& p2 )
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::acos;
#endif
    f64_t           ret                 = 0;
    const Vector3D  n1                  = p1.normal();
    const Vector3D  n2                  = p2.normal();

    const f64_t     dot                 = Dot( n1, n2 );
    const f64_t     len1                = n1.length();
    const f64_t     len2                = n2.length();

    if( len1 && len2 )
    {
        ret                             = acos( dot/(len1*len2) );
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if two three-dimensional planes are parallel.

@param      p1 The first plane.
@param      p2 The second plane.

@return     Returns true if the two planes are parallel, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Parallel( const Plane3D& p1, const Plane3D& p2 )
{
    bool    ret                         = false;

    Vector3D    v1                      = p1.normal().normal();
    Vector3D    v2                      = p2.normal().normal();

    std::transform( v1.begin(), v1.end(), v1.begin(), AbsVal() );
    std::transform( v2.begin(), v2.end(), v2.begin(), AbsVal() );

    if( v1 == v2 )
    {
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if two three-dimensional planes are perpendicular.

@param      p1 The first plane.
@param      p2 The second plane.

@return     Returns true if the two planes are perpendicular, false otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Perpendicular( const Plane3D& p1, const Plane3D& p2 )
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::fabs;
#endif
    bool    ret                         = false;

    f64_t   dot                         = fabs( Dot( p1.normal(),
        p2.normal() ) );

    if( dot < epsilon )
    {
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if a three-dimensional point is contained within a
            triangle.

@param      p The point.

@return     Returns true if the point is contained within the triangle, false
            otherwise.
*/
////////////////////////////////////////////////////////////////////////////////
bool
Triangle3D::contains( const Point3D& p ) const
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::fabs;
#endif
    bool        ret                     = false;

    const Triangle3D&   t               = *this;

    /* form vectors from each vertex of the triangle and the point */
    const Vector3D  pv1( p - t[0] );
    const Vector3D  pv2( p - t[1] );
    const Vector3D  pv3( p - t[2] );

    /* form vectors between the vertices of the triangle */
    const Vector3D  v1( t[1] - t[0] );
    const Vector3D  v2( t[2] - t[1] );
    const Vector3D  v3( t[0] - t[2] );

    /*
    ** use the vector cross product: if they all point in the same direction,
    ** then the point is contained in the triangle
    */
    const Vector3D  x1( Cross( v1, pv1 ).normal() );
    const Vector3D  x2( Cross( v2, pv2 ).normal() );
    const Vector3D  x3( Cross( v3, pv3 ).normal() );

    f64_t           x12diff             = fabs( x1[X] - x2[X] );
    f64_t           x13diff             = fabs( x1[X] - x3[X] );
    f64_t           y12diff             = fabs( x1[Y] - x2[Y] );
    f64_t           y13diff             = fabs( x1[Y] - x3[Y] );
    f64_t           z12diff             = fabs( x1[Z] - x2[Z] );
    f64_t           z13diff             = fabs( x1[Z] - x3[Z] );

    /*
    ** must carefully check for equality:
    ** the precision must be reduced to allow comparison
    */
	const double eps = 2.1;
    if( x12diff <= eps && x13diff <= eps
        && y12diff <= eps && y13diff <= eps
        && z12diff <= eps && z13diff <= eps )
    {
        ret                             = true;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Determines if two three-dimensional triangles intersect and if so,
            where.

@param      t1 The first triangle.
@param      t2 The second triangle.
@param      p Optional pointer to storage location for potential intersection
            point(s). (Pass zero (the default) if the value(s) are not needed.)

@return     Returns the number of intersection points.

@note       If there is only one intersection point and argument p is non-zero,
            the intersection point will be stored in p->first.
*/
////////////////////////////////////////////////////////////////////////////////
int32_t
Intersects( const Triangle3D& t1, const Triangle3D& t2,
    std::pair< Point3D, Point3D > *p )
{
    int32_t     ret                     = 0;
    /* array of pointers to arrays of Point3Ds */
    const Triangle3D    (*tri[2])       = { &t1, &t2, };
    Point3D             (*points[2])    = { &p->first, &p->second, };

    for( int cur = 0; cur < 2; cur++ )
    {
        int         i;
        int         j;
        for( i = 0, j = 2; i < 3; j = i++ )
        {
            /* the current edge to check */
            LineSeg3D   e( (*tri[cur])[i], (*tri[cur])[j] );

            if( Intersects( e, *tri[!cur], points[ret] ) )
            {
                ++ret;
                if( ret > 1 )
                {
                    break;
                }
            }
        }
        if( ret > 1 )
        {
            break;
        }
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Calculates the normal (unit-length) vector in the same direction
            as the four-dimensional vector.

@return     Returns the unit-length vector in the same direction as the vector.

@throw      Throws ZeroDivide if a division by zero would result in performing
            the operation.
*/
////////////////////////////////////////////////////////////////////////////////
Vector4D
Vector4D::normal( void ) const throw( ZeroDivide )
{
    f64_t   len                         = length();

    if( !len )
    {
        throw ZeroDivide();
    }

    Vector4D    ret;

    std::transform( begin(), end(), ret.begin(),
        std::bind2nd( std::divides< f64_t >(), len ) );

    return ret;
}

} // namespace Geometry
