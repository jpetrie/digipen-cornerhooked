#if !defined( MATRIX_HPP_ )
#define MATRIX_HPP_
////////////////////////////////////////////////////////////////////////////////
/**
@file       Matrix.hpp
@brief      CS300 Advanced Computer Graphics (I)

            This file contains geometric data structures and function
            prototypes used in matrix calculations and transformations.

@author     
@date       Started:    January     22, 2004 10:30
@date       Modified:   February    03, 2004 08:59
@note       Instructor: Xin Li
@note       Copyright © 2004 DigiPen Institute of Technology
*/
////////////////////////////////////////////////////////////////////////////////
#if !defined( __cplusplus )
#error this header cannot be used outside of a C++ program
#endif

#include <algorithm> // fill
#include <iostream>
#include <sstream>
#include "StdTypes.h"
#include "Geometry.hpp"

namespace Geometry
{

// Forward declaration:
class Matrix16;

////////////////////////////////////////////////////////////////////////////////
/**
@class      Matrix4x4

@brief      Type defining a four-dimensional (homogeneous) matrix.
*/
////////////////////////////////////////////////////////////////////////////////
class Matrix4x4
{
public:
    /// Type for sequential access.
    typedef Vector4D                    *iterator;
    /// Type for sequential access of const object.
    typedef const Vector4D              *const_iterator;

public:
    // Ctor.
    inline explicit
    Matrix4x4( const Vector4D& row1 = Vector4D( 1, 0, 0, 0 ),
        const Vector4D& row2 = Vector4D( 0, 1, 0, 0 ),
        const Vector4D& row3 = Vector4D( 0, 0, 1, 0 ),
        const Vector4D& row4 = Vector4D( 0, 0, 0, 1 ) );

    inline explicit
    Matrix4x4( const Matrix16& M );

public:
    // Arithmetic operators.
    Matrix4x4&
    operator*=( const Matrix4x4& rhs );

    // Random-access operators.
    inline
    Vector4D&
    operator[]( uint32_t i );

    inline
    const Vector4D&
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

public:
    void
    reset( void );

private:
    /// Dimension of Matrix.
    enum{ DIM = 4 };
    /// Array of components.
    Vector4D                            m_rows[DIM];

};

// (Global) operators.
inline
Matrix4x4
operator*( const Matrix4x4& lhs, const Matrix4x4& rhs );

inline
Vector3D
operator*( const Matrix4x4& lhs, const Vector3D& rhs );

inline
Point3D
operator*( const Matrix4x4& lhs, const Point3D& rhs );

Vector4D
operator*( const Matrix4x4& lhs, const Vector4D& rhs );

Point4D
operator*( const Matrix4x4& lhs, const Point4D& rhs );

// Stream operator:
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Matrix4x4& rhs );

// Utility functions:
Matrix4x4
LookAt( const Point3D& center, const Point3D& direction, const Vector3D& up );

Matrix4x4
Perspective( f64_t width, f64_t height, f64_t nearPlane, f64_t farPlane );

// Stream operator:
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Vector4D& rhs );


////////////////////////////////////////////////////////////////////////////////
/**
@class      Matrix16

@brief      Type defining a four-dimensional (column-major) matrix.
*/
////////////////////////////////////////////////////////////////////////////////
class Matrix16
{
public:
    /// Type for sequential access.
    typedef f64_t                       *iterator;
    /// Type for sequential access of const object.
    typedef const f64_t                 *const_iterator;

public:
    explicit
    Matrix16( const Matrix4x4& M = Matrix4x4() );

public:
    Matrix16&
    operator*=( const Matrix16& rhs );

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

public:
    inline
    void
    reset( void );

public:
    /// Dimension of Matrix.
    enum{ DIM = 16 };
    /// Array of components.
    f64_t                               m_components[DIM];

};

// (Global) operators.
inline
Matrix16
operator*( const Matrix16& lhs, const Matrix16& rhs );

inline
Vector3D
operator*( const Matrix16& lhs, const Vector3D& rhs );

inline
Point3D
operator*( const Matrix16& lhs, const Point3D& rhs );

inline
Vector4D
operator*( const Matrix16& lhs, const Vector4D& rhs );

inline
Point4D
operator*( const Matrix16& lhs, const Point4D& rhs );

// Utility function:
Matrix16
Transformation( const Vector3D& scaling, const Vector3D& rotation,
    const Vector3D& translation );

// Stream operator:
template< class charT, class traits >
inline
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Matrix16& rhs );

} // namespace Geometry


namespace Geometry
{

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the rows of a four-dimensional matrix to be specified.

@param      row1 The first row.
@param      row2 The second row.
@param      row3 The third row.
@param      row4 The fourth row.

@note       Default values, if used, yield the identity matrix.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Matrix4x4::Matrix4x4( const Vector4D& row1, const Vector4D& row2,
    const Vector4D& row3, const Vector4D& row4 )
{
    iterator    it                      = begin();

    *it                                 = row1;
    *++it                               = row2;
    *++it                               = row3;
    *++it                               = row4;

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor converting from a four-dimensional column-major matrix to a
            four-dimensional row-major matrix.

@param      M A matrix in column-major order.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Matrix4x4::Matrix4x4( const Matrix16& M )
{
    iterator    it                      = begin();

    *it                                 = Vector4D( M[0], M[4], M[8], M[12] );
    *++it                               = Vector4D( M[1], M[5], M[9], M[13] );
    *++it                               = Vector4D( M[2], M[6], M[10], M[14] );
    *++it                               = Vector4D( M[3], M[7], M[11], M[15] );

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for four-dimensional matrix.

@param      i The index of the row desired.

@return     Returns a reference to the row desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D&
Matrix4x4::operator[]( uint32_t i )
{
    return m_rows[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for const four-dimensional matrix.

@param      i The index of the row desired.

@return     Returns a const reference to the row desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
const Vector4D&
Matrix4x4::operator[]( uint32_t i ) const
{
    return m_rows[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the rows of a four-dimensional
            matrix.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Matrix4x4::iterator
Matrix4x4::begin( void )
{
    return m_rows + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the rows of a const
            four-dimensional matrix.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Matrix4x4::const_iterator
Matrix4x4::begin( void ) const
{
    return m_rows + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Matrix4x4::iterator
Matrix4x4::end( void )
{
    return m_rows + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Defines the end of the sequence for const object.

@return     Returns an iterator one past the end of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Matrix4x4::const_iterator
Matrix4x4::end( void ) const
{
    return m_rows + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies two four-dimensional matrices
            and returns the result.

@param      lhs The first matrix.
@param      rhs The second matrix.

@return     Returns the result of multiplying the matrices.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Matrix4x4
operator*( const Matrix4x4& lhs, const Matrix4x4& rhs )
{
    Matrix4x4   ret( lhs );

    ret                                 *= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies a three-dimensional vector by
            a four-dimensional (column-major) matrix and returns the result.

@param      lhs The matrix.
@param      rhs The vector.

@return     Returns the result of transforming the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D
operator*( const Matrix4x4& lhs, const Vector3D& rhs )
{
    Vector4D    temp( rhs[X], rhs[Y], rhs[Z], 0 );
    temp                                = lhs*temp;

    return Vector3D( temp[X], temp[Y], temp[Z] );
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies a three-dimensional point by
            a four-dimensional (column-major) matrix and returns the result.

@param      lhs The matrix.
@param      rhs The point.

@return     Returns the result of transforming the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D
operator*( const Matrix4x4& lhs, const Point3D& rhs )
{
    Point4D     temp( rhs[X], rhs[Y], rhs[Z], 1.0 );
    temp                                = lhs*temp;

    return Point3D( temp[X], temp[Y], temp[Z] );
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      ostream insertion operator for four-dimensional matrix.

@param      lhs The ostream to insert into.
@param      rhs The matrix to insert.

@return     Returns a reference to the ostream.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Matrix4x4& rhs )
{
    std::basic_ostringstream< charT, traits >   s;
    s.flags( lhs.flags() );
    s.imbue( lhs.getloc() );
    s.precision( lhs.precision() );

    for( typename Matrix4x4::const_iterator i = rhs.begin(); i != rhs.end();
    ++i )
    {
        s << *i << '\n';
    }

    return lhs << s.str();
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for four-dimensional column-major matrix.

@param      i The index of the component desired.

@return     Returns a reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
f64_t&
Matrix16::operator[]( uint32_t i )
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Random-access operator for const four-dimensional column-major
            matrix.

@param      i The index of the component desired.

@return     Returns a const reference to the component desired.

@warning    No range-checking is performed.
*/
////////////////////////////////////////////////////////////////////////////////
inline
const f64_t&
Matrix16::operator[]( uint32_t i ) const
{
    return m_components[i];
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the rows of a four-dimensional
            column-major matrix.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Matrix16::iterator
Matrix16::begin( void )
{
    return m_components + 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Allows fast sequential access to the rows of a const
            four-dimensional column-major matrix.

@return     Returns an iterator to the beginning of the sequence.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Matrix16::const_iterator
Matrix16::begin( void ) const
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
Matrix16::iterator
Matrix16::end( void )
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
Matrix16::const_iterator
Matrix16::end( void ) const
{
    return m_components + DIM;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Resets a four-dimensional column-major matrix to the identity
            matrix.
*/
////////////////////////////////////////////////////////////////////////////////
inline
void
Matrix16::reset( void )
{
    *this                               = Matrix16();

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies two four-dimensional
            column-major matrices and returns the result.

@param      lhs The first matrix.
@param      rhs The second matrix.

@return     Returns the result of multiplying the matrices.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Matrix16
operator*( const Matrix16& lhs, const Matrix16& rhs )
{
    Matrix16    ret                     = lhs;
    ret                                 *= rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies a three-dimensional vector by
            a four-dimensional (row-major) matrix and returns the result.

@param      lhs The matrix.
@param      rhs The vector.

@return     Returns the result of transforming the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector3D
operator*( const Matrix16& lhs, const Vector3D& rhs )
{
    Vector4D    temp( rhs[X], rhs[Y], rhs[Z], 0 );
    temp                                = lhs*temp;

    return Vector3D( temp[X], temp[Y], temp[Z] );
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies a three-dimensional point by
            a four-dimensional (row-major) matrix and returns the result.

@param      lhs The matrix.
@param      rhs The point.

@return     Returns the result of transforming the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point3D
operator*( const Matrix16& lhs, const Point3D& rhs )
{
    Point4D     temp( rhs[X], rhs[Y], rhs[Z], 1.0 );
    temp                                = lhs*temp;

    return Point3D( temp[X], temp[Y], temp[Z] );
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies a four-dimensional vector by
            a four-dimensional column-major matrix and returns the result.

@param      lhs The matrix.
@param      rhs The vector.

@return     Returns the result of transforming the vector.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Vector4D
operator*( const Matrix16& lhs, const Vector4D& rhs )
{
    Vector4D    ret                     = Matrix4x4( lhs )*rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Multiplication operator. Multiplies a four-dimensional point by
            a four-dimensional column-major matrix and returns the result.

@param      lhs The matrix.
@param      rhs The point.

@return     Returns the result of transforming the point.
*/
////////////////////////////////////////////////////////////////////////////////
inline
Point4D
operator*( const Matrix16& lhs, const Point4D& rhs )
{
    Point4D     ret                     = Matrix4x4( lhs )*rhs;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      ostream insertion operator for four-dimensional column-major matrix.

@param      lhs The ostream to insert into.
@param      rhs The matrix to insert.

@return     Returns a reference to the ostream.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
inline
std::basic_ostream< charT, traits >&
operator<<( std::basic_ostream< charT, traits >& lhs, const Matrix16& rhs )
{
    return lhs << Matrix4x4( rhs );
}

} // namespace Geometry

#endif // #if !defined( MATRIX_HPP_ )
