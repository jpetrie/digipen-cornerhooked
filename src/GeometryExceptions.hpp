#if !defined( GEOMETRY_EXCEPTIONS_HPP_ )
#define GEOMETRY_EXCEPTIONS_HPP_
////////////////////////////////////////////////////////////////////////////////
/**
@file       GeometryExceptions.hpp
@brief      CS300 Advanced Computer Graphics (I)

            This file contains exception types used to report errors by the
            Geometry library.

@author     
@date       Started:    January     07, 2004 16:11
@date       Modified:   January     07, 2004 16:11
@note       Instructor: Xin Li
@note       Copyright © 2004 DigiPen Institute of Technology
*/
////////////////////////////////////////////////////////////////////////////////
#if !defined( __cplusplus )
#error this header cannot be used outside of a C++ program
#endif

namespace Geometry
{

////////////////////////////////////////////////////////////////////////////////
/**
@struct     ZeroDivide

@brief      Used for divide-by-zero errors.
*/
////////////////////////////////////////////////////////////////////////////////
struct ZeroDivide
{
};

} // namespace Geometry

#endif // #if !defined( GEOMETRY_EXCEPTIONS_HPP_ )
