/*!
	@file	PlayfieldPocket.h
	@author	Scott Smith
	@date	June 21, 2004

	@brief	Interface to pocket class.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#ifndef _PLAYFIELD_POCKET_H_
#define _PLAYFIELD_POCKET_H_

#include "main.h"

#include "GraphicsRenderer.h"
#include "GraphicsPrimitive.h"

#include "Physics.h"

/*!
	@class	Pocket
	@author Scott
	@date	06-21-2004
	@ingroup ch
	@brief Represents a playfield pocket.
*//*__________________________________________________________________________*/
class Pocket : public Graphics::Primitive
{
public:
	Pocket(Graphics::Renderer *renderer,const D3DXVECTOR3 &corner,float size);
	~Pocket(void);
	
	virtual void RestoreDeviceObjects(void);
	virtual void ReleaseDeviceObjects(void);
	
	virtual void Render(const D3DXVECTOR3 &eyePos,const D3DXVECTOR3 &eyeVec);
	
	/*!
		@param id The id of a plane that builds the pocket
	*//*__________________________________________________________________________*/
	void AddPlane(const unsigned int id)		{ mPlanes.push_back(id); }
	
	/*!
		@param id An id to test within the pocket
		@return True, if the plane is part of the pocket.  False otherwise.
	*//*__________________________________________________________________________*/
	bool QueryPlane(const unsigned int id)
	{
		mIt = mPlanes.begin();
		while(mIt != mPlanes.end())
		{
			if(*mIt == id)
				return true;
			++mIt;
		}
		return false;
	}
	
    bool isLit;	

    D3DXVECTOR3 CornerPoint(void)const                            { return mCornerPoint; }
    std::vector< Physics::BoundedPlane > PhysicsPlanes(void)const { return mPhysicsPlanes; }

private:
  // disabled
	Pocket(const Pocket &s);
  Pocket operator=(const Pocket &s);
  
	struct Vertex
	{
	  enum { Format = D3DFVF_XYZ | D3DFVF_DIFFUSE };
	  D3DXVECTOR3  pos;
	  DWORD        color;
	};
	
	void MakePocket(void);
	
	std::vector< unsigned int >           mPlanes;			   ///< The id's of the planes in the pocket.
	std::vector< Physics::BoundedPlane >  mPhysicsPlanes;  ///<  
	
	D3DXVECTOR3						        mCornerPoint;  ///< The point at the back of the pocket.
public:
	D3DXVECTOR3						        mCornerX;      ///< X axis point.
	D3DXVECTOR3						        mCornerY;      ///< Y axis point.
	D3DXVECTOR3						        mCornerZ;      ///< Z axis point.
private:
	float                         mSize;         ///< Pocket scale factor.
	
	LPDIRECT3DVERTEXBUFFER9  mVtxBuffer;  ///< Pocket vertex buffer.
	LPDIRECT3DVERTEXBUFFER9  mLitVtxBuffer;  ///< Pocket vertex buffer.
	
	std::vector< unsigned int >::iterator	mIt;		///< An iterator for mPlanes.
};

#endif  /* _PLAYFIELD_POCKET_H_ */