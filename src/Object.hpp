#if !defined( OBJECT_HPP_ )
#define OBJECT_HPP_
////////////////////////////////////////////////////////////////////////////////
/**
@file       Object.hpp
@brief      CS300 Advanced Computer Graphics (I)

            This file contains data structures and function prototypes for a
            three-dimensional object.

@author     
@date       Started:    January     30, 2004 12:49
@date       Modified:   February    06, 2004 18:38
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
#include <cassert>
#include <iostream>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#pragma warning( pop )

#include "Geometry.hpp"
#include "Polygon.hpp"

namespace Geometry
{

////////////////////////////////////////////////////////////////////////////////
/**
@struct     Object3D

@brief      Specifies a three-dimensional object.
*/
////////////////////////////////////////////////////////////////////////////////
struct Object3D
{
public:
    inline explicit
    Object3D( const std::string& Name = std::string() );

public:
    void
    SetColor( const ObjColor& Color );

public:
    void
    Display( bool Wireframe = false ) const;

public:
    /// The object's name (for debugging.)
    std::string                         name;
    /// The object's scaling factor.
    Vector3D                            scale;
    /// The object's rotation.
    Vector3D                            rotation;
    /// The object's offset.
    Vector3D                            offset;
    /// The object's vertex table.
    VertexTable                         vertex_table;
    /// The object's polygon table.
    PolygonTable                        polygon_table;
    /// The object's bounding box.
    Box3D                               bounding_box;

private:
    uint32_t                            m_id;

private:
    static uint32_t                     s_id;

};

// Stream operator:
template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Object3D& rhs );

} // namespace Geometry


namespace Geometry
{

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Ctor allowing the name of the object to be specified.

@param      Name The name of this object.
*/
////////////////////////////////////////////////////////////////////////////////
const float BigNum = 1000000000.0f;
inline
Object3D::Object3D( const std::string& Name )
: name( Name ), scale(), rotation(), offset(), vertex_table(), polygon_table(),
bounding_box( Point3D( BigNum,
    BigNum, BigNum ),
    Point3D( -BigNum,
    -BigNum,
    -BigNum ) ), m_id( s_id++ )
{
    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Extracts a line of characters from an input stream, skipping leading
            whitespace.

            Whitespace is skipped over by utilizing isspace.

@param      input The input stream to extract the line of characters from.
@param      string The storage location for the line of characters.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits, class Allocator >
void
ReadNextLine( std::basic_istream< charT, traits >& input,
    std::basic_string< charT, traits, Allocator >& string )
{
    // Skip leading whitespace.
    while( std::isspace( input.peek(), input.getloc() ) )
    {
        input.ignore();
    }

    std::basic_string< charT, traits, Allocator >   temp;
    // Read in a line of text.
    if( std::getline( input, temp ) )
    {
        string                          = temp;
    }

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      Extracts a line of characters from an input stream, skipping leading
            whitespace.

            Whitespace is skipped over by utilizing isspace.

@param      input The input stream to extract the line of characters from.
@param      stringstream The storage location for the line of characters.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits, class Allocator >
void
ReadNextLine( std::basic_istream< charT, traits >& input,
    std::basic_istringstream< charT, traits, Allocator >& stringstream )
{
    std::basic_string< charT, traits, Allocator >   string;
    ReadNextLine( input, string );
    stringstream.str( string );

    return;
}

////////////////////////////////////////////////////////////////////////////////
/**
@brief      istream extraction operator for three-dimensional object.

@param      lhs The istream to extract from.
@param      rhs The object to extract.

@return     Returns a reference to the istream.
*/
////////////////////////////////////////////////////////////////////////////////
template< class charT, class traits >
std::basic_istream< charT, traits >&
operator>>( std::basic_istream< charT, traits >& lhs, Object3D& rhs )
{
    typedef std::basic_string< charT, traits >  string_type;
    typedef std::basic_istringstream< charT, traits >   strstream_type;

    string_type     buffer;
    strstream_type  sstream;

    // read a line-find the "GeometryClass" tag (2nd tag)
    ReadNextLine( lhs, sstream );
    sstream >> buffer >> buffer;
    sstream.clear();
 ASSERT( "GeometryClass" == buffer );
    buffer.erase();

    // read a line-get the object's name
    Object3D    temp;
    ReadNextLine( lhs, sstream );
    sstream >> temp.name;
    sstream.clear();

    // *read transformation information
    ReadNextLine( lhs, sstream );
    // scale
    sstream >> temp.scale;
    sstream.clear();
    // skip quaternion and axis-angle
    ReadNextLine( lhs, buffer );
    ReadNextLine( lhs, buffer );
    buffer.erase();
    // euler-angle rotation
    ReadNextLine( lhs, sstream );
    sstream >> buffer;
    sstream.clear();
    buffer.erase();
    sstream >> temp.rotation;
    sstream.clear();
    std::swap( temp.rotation[Y], temp.rotation[Z] );
    temp.rotation[Z]                    = -temp.rotation[Z];
    // translation
    ReadNextLine( lhs, sstream );
    sstream >> temp.offset;
    sstream.clear();
    std::swap( temp.offset[Y], temp.offset[Z] );
    temp.offset[Z]                      = -temp.offset[Z];
    // skip two lines
    ReadNextLine( lhs, buffer );
    ReadNextLine( lhs, buffer );
    buffer.erase();

    // *read vertex table
    ReadNextLine( lhs, sstream );
    int     count                       = 0;
    sstream >> count;
    sstream.clear();
    if( count > 0 )
    {
        temp.vertex_table.resize( count );
        for( int i = 0; i < count; ++i )
        {
            Point3D&    point           = temp.vertex_table[i];
            ReadNextLine( lhs, sstream );
            sstream >> buffer;
            sstream.clear();
            sstream >> point;
            sstream.clear();
            std::swap( point[Y], point[Z] );
            point[Z]                    = -point[Z];
        }
    }

    // *read polygon table
    ReadNextLine( lhs, sstream );
    sstream >> buffer >> count;
    sstream.clear();
    if( count > 0 )
    {
        temp.polygon_table.resize( count );
        for( int i = 0; i < count; ++i )
        {
            PolyData&   poly            = temp.polygon_table[i];
            ReadNextLine( lhs, buffer );
            ReadNextLine( lhs, buffer );

            ReadNextLine( lhs, sstream );
            sstream >> poly.points[0];
            sstream.clear();
            ReadNextLine( lhs, sstream );
            sstream >> poly.points[1];
            sstream.clear();
            ReadNextLine( lhs, sstream );
            sstream >> poly.points[2];
            sstream.clear();
        }
    }

    // If the extraction was a success, save the return value.
    if( lhs.good() || (lhs.eof() && (!lhs.fail() && !lhs.bad())) )
    {
        rhs                             = temp;
    }

    return lhs;
}

} // namespace Geometry

#endif // #if !defined( OBJECT_HPP_ )
