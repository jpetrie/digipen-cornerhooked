////////////////////////////////////////////////////////////////////////////////
/**
@file       Matrix.cpp
@brief      CS300 Advanced Computer Graphics (I)

            This file contains geometric function definitions used in matrix
            calculations and transformations.

@author     
@date       Started:    January     22, 2004 10:30
@date       Modified:   February    05, 2004 21:46
@note       Instructor: Xin Li
@note       Copyright © 2004 DigiPen Institute of Technology
*/
////////////////////////////////////////////////////////////////////////////////
#include "main.h"

#include "Matrix.hpp"

namespace Geometry
{

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (multiplication) operator. Multiplies two
            four-dimensional matrices and returns the result.

@param      rhs The matrix to multiply by.

@return     Returns the multiplied matrix (*this.)
*/
////////////////////////////////////////////////////////////////////////////////
Matrix4x4&
Matrix4x4::operator*=( const Matrix4x4& rhs )
{
    Matrix4x4   M3;
    const Matrix4x4&    M1              = *this;
    const Matrix4x4&    M2              = rhs;
    for( int i = 0; i < 4; ++i )
    {
        for( int j = 0; j < 4; ++j )
        {
            M3[i][j]                    = M1[i][0]*M2[0][j] + M1[i][1]*M2[1][j]
                + M1[i][2]*M2[2][j] + M1[i][3]*M2[3][j];
        }
    }

    *this                               = M3;

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Resets a four-dimensional matrix to the identity matrix.
*/
////////////////////////////////////////////////////////////////////////////////
void
Matrix4x4::reset( void )
{
    std::fill( begin(), end(), Vector4D() );

    (*this)[X][X]                       = 1;
    (*this)[Y][Y]                       = 1;
    (*this)[Z][Z]                       = 1;
    (*this)[W][W]                       = 1;

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies a four-dimensional vector by
            a four-dimensional matrix and returns the result.

@param      lhs The matrix.
@param      rhs The vector.

@return     Returns the result of transforming the vector.
*/
////////////////////////////////////////////////////////////////////////////////
Vector4D
operator*( const Matrix4x4& lhs, const Vector4D& rhs )
{
    Vector4D    ret;

    for( int i = 0; i < 4; ++i )
    {
        ret[i]                          = lhs[i][0]*rhs[0] + lhs[i][1]*rhs[1]
            + lhs[i][2]*rhs[2] + lhs[i][3]*rhs[3];
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies a four-dimensional point by
            a four-dimensional matrix and returns the result.

@param      lhs The matrix.
@param      rhs The point.

@return     Returns the result of transforming the point.
*/
////////////////////////////////////////////////////////////////////////////////
Point4D
operator*( const Matrix4x4& lhs, const Point4D& rhs )
{
    Vector4D    temp( rhs[X], rhs[Y], rhs[Z], rhs[W] );
    temp                                = lhs*temp;

    Point4D     ret( temp[X], temp[Y], temp[Z], temp[W] );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Calculates a four-dimensional matrix that transforms geometry from
            the world coordinate system to the camera coordinate system.

@param      center The center-point of the camera location.
@param      direction A point along the desired line of sight, typically some
            location in the center of the scene being looked at.
@param      up A vector implying the ``up'' direction (the direction from the
            bottom to the top of the viewing volume.)

@return     Returns a matrix that can perform the desired transformation.

@note       The center, direction, and up arguments are all defined wrt to world
            coordinate system.
@note       It is assumed that the view reference system is left-handed.
@note       The transformation matrix is stored in row-major order.
*/
////////////////////////////////////////////////////////////////////////////////
Matrix4x4
LookAt( const Point3D& center, const Point3D& direction, const Vector3D& up )
{
    /* define w-vector as that going between camera and view point */
    Vector3D    w( (center - direction).normal() );
    /* u-vector is perpendicular to w and up vectors */
    Vector3D    u( Cross( up, w ).normal() );
    /* v-vector is perpendicular to u and w vectors */
    Vector3D    v( Cross( w, u ).normal() );

    Matrix4x4   ret;

    /* assign the resulting rotation matrix */
    ret[X]                              = Vector4D( u[X], u[Y], u[Z] );
    ret[Y]                              = Vector4D( v[X], v[Y], v[Z] );
    ret[Z]                              = Vector4D( w[X], w[Y], w[Z] );

    Matrix4x4   trans;

    /* assign the translation component of the matrix */
    trans[X][W]                         = -center[X];
    trans[Y][W]                         = -center[Y];
    trans[Z][W]                         = -center[Z];

    ret                                 *= trans;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Calculates a four-dimensional matrix that transforms geometry from
            the camera coordinate system to the perspective projection
            coordinate system. The function parameters define the view frustum.

@param      width The width of the view plane.
@param      height The height of the view plane.
@param      nearPlane The distance from the eye point to the view plane.
@param      farPlane The distance from the eye point to the far clipping plane.

@return     Returns a matrix that can perform the desired transformation.

@note       The transformation matrix is stored in row-major order.
*/
////////////////////////////////////////////////////////////////////////////////
Matrix4x4
Perspective( f64_t width, f64_t height, f64_t nearPlane, f64_t farPlane )
{
    Matrix4x4   ret;

    ret[X][X]                           = height;
    ret[Y][Y]                           = width;
    ret[Z][Z]                           = (height*width*farPlane)
        /(nearPlane*(farPlane - nearPlane));
    ret[Z][W]                           = -(height*width*farPlane)
        /(farPlane - nearPlane);
    ret[W][Z]                           = (height*width)/nearPlane;
    ret[W][W]                           = 0;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor converting from a four-dimensional row-major matrix to a
            four-dimensional column-major matrix.

@param      M A matrix in row-major order.
*/
////////////////////////////////////////////////////////////////////////////////
Matrix16::Matrix16( const Matrix4x4& M )
{
    iterator    it                      = begin();

    *it   = M[0][0];    *++it = M[1][0];    *++it = M[2][0];    *++it = M[3][0];
    *++it = M[0][1];    *++it = M[1][1];    *++it = M[2][1];    *++it = M[3][1];
    *++it = M[0][2];    *++it = M[1][2];    *++it = M[2][2];    *++it = M[3][2];
    *++it = M[0][3];    *++it = M[1][3];    *++it = M[2][3];    *++it = M[3][3];

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Compound assignment (multiplication) operator. Multiplies two
            four-dimensional column-major matrices and returns the result.

@param      rhs The matrix to multiply by.

@return     Returns the multiplied matrix (*this.)
*/
Matrix16&
Matrix16::operator*=( const Matrix16& rhs )
{
    Matrix16    M3( Matrix4x4( *this )*Matrix4x4( rhs ) );

    *this                               = M3;

    return *this;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Calculates a four-dimensional matrix with the given scaling,
            rotation, and translation characteristics.

@param      scaling A vector specifying the x-, y-, and z-scaling factors.
@param      rotation A vector specifying the x-, y-, and z-rotation factors.
@param      translation A vector specifying the x-, y-, and z-translation.

@return     Returns a matrix that can perform the desired transformation.

@note       The transformation matrix is stored in column-major order.
*/
////////////////////////////////////////////////////////////////////////////////
Matrix16
Transformation( const Vector3D& scaling, const Vector3D& rotation,
    const Vector3D& translation )
{
// compiler bug-fix - std namespace includes C library functions
// (It's not really a bug--it's a problem with the headers, and it only affects
// VC6.)
#if !defined( _MSC_VER ) || ( 1200 < _MSC_VER )
    using std::sin;
    using std::cos;
#endif
    const f64_t sina                    = sin( ToRadians( rotation[X] ) );
    const f64_t sinb                    = sin( ToRadians( rotation[Y] ) );
    const f64_t sinc                    = sin( ToRadians( rotation[Z] ) );
    const f64_t cosa                    = cos( ToRadians( rotation[X] ) );
    const f64_t cosb                    = cos( ToRadians( rotation[Y] ) );
    const f64_t cosc                    = cos( ToRadians( rotation[Z] ) );

    Matrix16    ret;
    ret[0]                              = cosb*cosc*scaling[X];
    ret[1]                              = cosb*sinc*scaling[X];
    ret[2]                              = -sinb*scaling[X];
    ret[3]                              = 0;

    ret[4]                              = (cosc*sina*sinb - cosa*sinc)
        *scaling[Y];
    ret[5]                              = (cosa*cosc + sina*sinb*sinc)
        *scaling[Y];
    ret[6]                              = cosb*sina*scaling[Y];
    ret[7]                              = 0;

    ret[8]                              = (cosa*cosc*sinb + sina*sinc)
        *scaling[Z];
    ret[9]                              = (cosa*sinb*sinc - cosc*sina)
        *scaling[Z];
    ret[10]                             = cosa*cosb*scaling[Z];
    ret[11]                             = 0;

    ret[12]                             = translation[X];
    ret[13]                             = translation[Y];
    ret[14]                             = translation[Z];
    ret[15]                             = 1.0;

    return ret;
}

} // namespace Geometry
