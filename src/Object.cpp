////////////////////////////////////////////////////////////////////////////////
/**
@file       Object.cpp
@brief      CS300 Advanced Computer Graphics (I)

            This file contains function definitions for a three-dimensional
            object.

@author     
@date       Started:    February    01, 2004 11:23
@date       Modified:   February    05, 2004 21:46
@note       Instructor: Xin Li
@note       Copyright © 2004 DigiPen Institute of Technology
*/
////////////////////////////////////////////////////////////////////////////////
#include "main.h"

#include "Object.hpp"

/// Static member definition.
uint32_t    Geometry::Object3D::s_id              = 0;
namespace Geometry
{



////////////////////////////////////////////////////////////////////////////////
/**
@brief      Sets the color for every polygon in this object.

@param      Color The color to set.
*/
////////////////////////////////////////////////////////////////////////////////
void
Object3D::SetColor( const ObjColor& Color )
{
    for( PolygonTable::iterator i = polygon_table.begin();
    i != polygon_table.end(); ++i )
    {
        i->color                        = Color;
    }

    return;
}
} // namespace Geometry
