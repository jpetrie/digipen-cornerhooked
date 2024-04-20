/* =========================================================================
   
    @file    Camera.h
    @author  jmp
    @brief   Interface to camera class.
    
   ========================================================================= */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _CAMERA_H_
#define _CAMERA_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include <d3dx9math.h>


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class Camera
/*! A 3D camera. 
*/
{ 
  public:
    // ct and dt
    Camera(void);
    ~Camera(void);

    // view transformation
	  void GetViewTransform(D3DXMATRIX *viewMatrix) const;
	  void GetViewVector(D3DXVECTOR3 *viewVector) const;
	  void GetPosVector(D3DXVECTOR3 *posVector) const;
	  void GetTargetVector(D3DXVECTOR3 *targetVector) const;
	  void GetUpVector(D3DXVECTOR3 *upVector) const;
	  
	  // distance to target
	  float GetDistToTarget(void) const { return (mDistToTarget); }

    // absolute position and orientation
    void SetTarget(float x,float y,float z,bool update_postion = false);
    void SetPosition(float x,float y,float z,bool update_target = false);
  
    // relative position and orientation
    void OrbitX(float deg = 1.0f);
    void OrbitY(float deg = 1.0f);
    void TrackZ(float amt = 1.0f);
    
    // constraints
    float GetMinTargetDistance(void) const;
    float GetMaxTargetDistance(void) const;
    

  private:
    // basis
    void nRecomputeBasis(void);
    
    // data members
    D3DXVECTOR3  mTarget;    //!< Target point.
	  D3DXVECTOR3  mPosition;  //!< Eye point.
    D3DXVECTOR3  mUp;        //!< Up vector.
    D3DXVECTOR3  mView;      //!< View vector.
	  D3DXVECTOR3  mCross;     //!< Up cross view vector.
	  
	  float  mAngleXZ;       //!< Angle of the view vector with the XZ plane.
    float  mDistToTarget;  //!< Distance to target point.
};

#endif  /* _CAMERA_H_ */