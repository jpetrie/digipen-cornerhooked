/* =========================================================================
   
    @file    Camera.cpp
    @author  jmp
    @brief   Implementation of camera class.
    
   ========================================================================= */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Camera.h"
#include "Game.h"

/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  const float  kMinDistanceToTarget = 5.0f;    //!< Minimum distance to target.
  const float  kMaxDistanceToTarget = 500.0f;  //!< Maximum distance to target.
  const float  kDefDistanceToTarget = 100.0f;  //!< Initial distance to target.
};


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
Camera::Camera(void)
/*! Default constructor.
*/
: mTarget(0.0f,0.0f,0.0f),mPosition(0.0f,0.0f,-kDefDistanceToTarget),
  mUp(0.0f,1.0f,0.0f),mView(0.0f,0.0f,1.0f),mCross(1.0f,0.0f,0.0f),
  mAngleXZ(90.0f),mDistToTarget(kDefDistanceToTarget)
{
}

/*  ________________________________________________________________________ */
Camera::~Camera(void)
/*! Destructor.
*/
{
}

/*  ________________________________________________________________________ */
void Camera::GetViewTransform(D3DXMATRIX *viewMatrix) const
/*! Retrieve the view matrix corresponding to the camera's state.

    @param viewMat  Receives the view matrix.
*/
{
  if(0 != viewMatrix)
    D3DXMatrixLookAtLH(viewMatrix,&mPosition,&mTarget,&mUp);
}

/*  ________________________________________________________________________ */
void Camera::GetViewVector(D3DXVECTOR3 *viewVector) const
/*! Retrieve the camera's view vector.

    @param viewVector  Receives the view vector.
*/
{
  if(0 != viewVector)
    *viewVector = mView;
}

/*  ________________________________________________________________________ */
void Camera::GetPosVector(D3DXVECTOR3 *posVector) const
/*! Retrieve the position of camera in vector form.

    @param posVector Receives the position vector.
*/
{
  if(0 != posVector)
    *posVector = mPosition;
}

/*  ________________________________________________________________________ */
void Camera::GetTargetVector(D3DXVECTOR3 *targetVector) const
/*! Retrieve the target position of the camera in vector form.

    @param targetVector Receives the target position vector.
*/
{
  if(0 != targetVector)
    *targetVector = mTarget;
}

/*  ________________________________________________________________________ */
void Camera::GetUpVector(D3DXVECTOR3 *upVector) const
/*! Retrieve the up position of the camera.

    @param upVector Receives the up vector.
*/
{
  if(0 != upVector)
    *upVector = mUp;
}

/*  ________________________________________________________________________ */
void Camera::SetTarget(float x,float y,float z,bool /*update_position*/)
/*! Set the camera's view target.

    @param x                The X coordinate of the camera's new target.
    @param y                The Y coordinate of the camera's new target.
    @param z                The Z coordinate of the camera's new target.
    @param update_position  If true, update the position of the camera to
                            keep the target distance the same.
*/
{
  mTarget.x = x;
  mTarget.y = y;
  mTarget.z = z;
  
  D3DXVECTOR3 temp = (mTarget - mPosition);
  mDistToTarget = D3DXVec3Length(&temp);
  
  //if(update_position)
  //{
  //  mPosition.x = mTarget.x - mView.x * mDistToTarget;
  //  mPosition.y = mTarget.y - mView.y * mDistToTarget;
  //  mPosition.z = mTarget.z - mView.z * mDistToTarget;
  //}
  
  nRecomputeBasis();
}

/*  ________________________________________________________________________ */
void Camera::SetPosition(float x,float y,float z,bool /*update_target*/)
/*! Set the camera's position.

    @param x              The X coordinate of the camera's new position.
    @param y              The Y coordinate of the camera's new position.
    @param z              The Z coordinate of the camera's new position.
    @param update_target  If true, update the target of the camera to
                          keep the target distance the same.
*/
{
  mPosition.x = x;
  mPosition.y = y;
  mPosition.z = z;
  
  D3DXVECTOR3 temp = (mTarget - mPosition);
  mDistToTarget = D3DXVec3Length(&temp);
  
  //if(update_target)
  //{
  //  mTarget.x = mPosition.x + mView.x * mDistToTarget;
  //  mTarget.y = mPosition.y + mView.y * mDistToTarget;
  //  mTarget.z = mPosition.z + mView.z * mDistToTarget;
  //}
  
  nRecomputeBasis();
}

/*  ________________________________________________________________________ */
void Camera::OrbitX(float deg)
/*! Orbits the camera in the X direction.

    OrbitX() orbits the camera about the target point in such a way that it
    appears as though the camera is moving in a "left-right" aligned manner.
    
    @param deg  The amount to orbit, in degrees. Positive values orbit the
                the camera to the left of the target.
*/
{
D3DXMATRIX   m;
D3DXVECTOR3  up(0.0f,1.0f,0.0f);
D3DXVECTOR4  nview;
D3DXVECTOR4  nup;
D3DXVECTOR4  ncross;

 /* if(abs(deg) < 0.01f)
    return;*/

  // All three basis vectors will be rotated around the world up
  // vector (0,1,0). This makes for a less disorienting orbit.
  D3DXMatrixRotationAxis(&m,&up,deg * (D3DX_PI / 180.0f));
  D3DXVec3Transform(&nview,&mView,&m);
  D3DXVec3Transform(&nup,&mUp,&m);
  D3DXVec3Transform(&ncross,&mCross,&m);

  // Update the camera position. Subtract since nview is in the opposite
  // direction from that in which we want to move.
D3DXVECTOR3  nview2(nview.x,nview.y,nview.z);

  D3DXVec3Normalize(&nview2,&nview2);
  mPosition.x = mTarget.x - nview2.x * mDistToTarget;
  mPosition.y = mTarget.y - nview2.y * mDistToTarget;
  mPosition.z = mTarget.z - nview2.z * mDistToTarget;
 
  // Normalize and restore the other vectors.
  mView.x  = nview.x;   mView.y = nview.y;   mView.z = nview.z;
  mUp.x    = nup.x;       mUp.y = nup.y;       mUp.z = nup.z; 
  mCross.x = ncross.x; mCross.y = ncross.y; mCross.z = ncross.z; 
  D3DXVec3Normalize(&mView,&mView);
  D3DXVec3Normalize(&mUp,&mUp);
  D3DXVec3Normalize(&mCross,&mCross);
}

/*  ________________________________________________________________________ */
void Camera::OrbitY(float deg)
/*! Orbits the camera in the Y direction.

    OrbitY() orbits the camera about the target point in such a way that it
    appears as though the camera is moving in a "up-down" aligned manner.
    
    @param deg  The amount to orbit, in degrees. Positive values orbit the
                the camera to the above the target.
*/
{
D3DXMATRIX   m;
D3DXVECTOR4  vp;
D3DXVECTOR4  nview;
D3DXVECTOR4  nup;

  // Clamp degrees if they'd put us beyond the limits.
  if(mAngleXZ + deg > 180.0f)
    deg = 180.0f - mAngleXZ;
  if(mAngleXZ + deg < 0.0f)
    deg = -mAngleXZ;
    
  // Rotate about the cross (X) vector.
  D3DXMatrixRotationAxis(&m,&mCross,deg * (D3DX_PI / 180.0f));
  D3DXVec3Transform(&nview,&mView,&m);
  D3DXVec3Transform(&nup,&mUp,&m);
D3DXVECTOR3  nview2(nview.x,nview.y,nview.z);

  D3DXVec3Normalize(&nview2,&nview2);

  // Move to the new position.
  mPosition.x = mTarget.x - nview2.x * mDistToTarget;
  mPosition.y = mTarget.y - nview2.y * mDistToTarget;
  mPosition.z = mTarget.z - nview2.z * mDistToTarget;
  
  // Normalize and store the other vectors. Note that
  // cross doesn't change because we moved around it.
  mView.x  = nview.x;   mView.y = nview.y;   mView.z = nview.z;
  mUp.x    = nup.x;       mUp.y = nup.y;       mUp.z = nup.z; 
  D3DXVec3Normalize(&mView,&mView);
  D3DXVec3Normalize(&mUp,&mUp);
  
  mAngleXZ += deg;
}

/*  ________________________________________________________________________ */
void Camera::TrackZ(float amt)
/*! Tracks the camera in the Z direction.

    TrackZ() moves the camera along its Z axis the specified amount, without
    changing its orientation. The effect is that the camera moves towards or
    away from the target point.
    
    The camera cannot track too far from or too near the target; these limits
    can be retrieved by the GetMaxTargetDistance() and GetMinTargetDistance()
    methods.
    
    @param amt  The amount to track. Positive values move towards the target.
*/
{
  mDistToTarget -= amt;
  if(mDistToTarget < kMinDistanceToTarget)
    mDistToTarget = kMinDistanceToTarget;
  else if(mDistToTarget > kMaxDistanceToTarget)
    mDistToTarget = kMaxDistanceToTarget;
}

/*  ________________________________________________________________________ */
float Camera::GetMinTargetDistance(void) const
/*! Returns the minimum distance allowed between the camera and its target.

    @return
    The minimum distance the camera is allowed to track in the Z direction
    towards the target point.
*/
{
  return (kMinDistanceToTarget);
}

/*  ________________________________________________________________________ */
float Camera::GetMaxTargetDistance(void) const
/*! Returns the maximum distance allowed between the camera and its target.

    @return
    The maximum distance the camera is allowed to track in the Z direction
    from the target point.
*/
{
  return (kMaxDistanceToTarget);
}

/*  ________________________________________________________________________ */
void Camera::nRecomputeBasis(void)
/*! Recomputes the camera basis vectors based on the target and position.
*/
{
  // Get the view vector.
  D3DXVec3Subtract(&mView,&mTarget,&mPosition);
  D3DXVec3Normalize(&mView,&mView);
  
D3DXVECTOR3  up(0.0f,1.0f,0.0f);
D3DXVECTOR3  plane(0.0f,1.0f,0.0f);

  D3DXVec3Cross(&mCross,&up,&mView);
  D3DXVec3Normalize(&mCross,&mCross);
  D3DXVec3Cross(&mUp,&mView,&mCross);
  D3DXVec3Normalize(&mUp,&mUp);
  
  // Compute the angle with the XZ plane.
  mAngleXZ = (180.0f / 3.14f) * ::acos(D3DXVec3Dot(&plane,&mView));
}