/*!
	@file	PhysicsDefs.h
	@author	Scott Smith
	@date	May 04, 2004

	@brief	Internal implementation file for physics simulation.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#pragma once

#ifndef _PHYSICSDEFS_H_
#define	_PHYSICSDEFS_H_

#include "MathDefs.h"
#include "Geometry.hpp"
#include "Polygon.hpp"

using Geometry::Vector3D;

namespace Physics
{
  // config constant defaults
  const float  kDef_MaxLinearVel  = 100.0f;
  const float  kDef_MaxAngularVel = 10.0f;
  const float  kDef_MaxAngularMom = 10;
  const float  kDef_DragCoeff     = 50;

	///< @enum eInertiaKind	Inertial types for different bodies.
	enum eInertiaKind	{ kI_Immobile = 0, kI_Sphere };
	enum eCollisionKind	{ kC_Plane = 0, kC_Sphere, kC_BoundedPlane};

	/*!
	 @class		Geometry
	 @ingroup	Physics Engine Proto
	 @date		05-04-2004
	 @author	Scott
	*//*__________________________________________________________________________*/
	class GeometryType
	{
	public:
		GeometryType() {}
		virtual ~GeometryType() {}
		virtual uint32_t Kind() = 0;
	};

	/*!
	 @class		Plane
	 @ingroup	Physics Engine Proto
	 @date		05-04-2004
	 @author	Scott
	*//*__________________________________________________________________________*/
	class Plane : public GeometryType
	{
	public:
		Plane(const Geometry::Plane3D plane) : mPlane(plane) {}
		virtual ~Plane() {}
		uint32_t Kind()	{ return kC_Plane; }

		Geometry::Plane3D mPlane;
	};
	/*!
	 @class		Plane
	 @ingroup	Physics Engine Proto
	 @date		05-04-2004
	 @author	Scott
	*//*__________________________________________________________________________*/
	class BoundedPlane : public Plane
	{
	public:
		BoundedPlane():Plane(Geometry::Plane3D()) {}
		BoundedPlane(const Geometry::Point3D p0, const Geometry::Point3D p1, const Geometry::Point3D p2):Plane(Geometry::Plane3D())
		{
            Geometry::Vector3D n = ((p1 - p0).normal() ^ (p2 - p0)).normal();
			Geometry::Vector3D pt = (p0 - Geometry::Point3D());
			mPlane = Geometry::Plane3D(n[0], n[1], n[2], -(n * pt));
			mPolygon[0] = p0;
			mPolygon[1] = p1;
			mPolygon[2] = p2;
		}
		virtual ~BoundedPlane() {}
		uint32_t Kind()	{ return kC_BoundedPlane; }
		Geometry::Triangle3D	mPolygon;
	};
	/*!
	 @class		Sphere
	 @ingroup	Physics Engine Proto
	 @date		05-04-2004
	 @author	Scott
	*//*__________________________________________________________________________*/
	class Sphere : public GeometryType
	{
	public:
		Sphere(const Real radius) : mRadius(radius) {}
		virtual ~Sphere() {}
		uint32_t Kind()	{ return kC_Sphere; }

		Real mRadius;
	};
}	// namespace Physics

#endif