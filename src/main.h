/*! ========================================================================
    
      @file    main.h
      @author  jmp
      @brief   Corner Hooked main header.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _CH_MAIN_H_
#define _CH_MAIN_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include <windows.h>

#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9math.h>
#include <dxerr9.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma warning (disable:4702)  // unreachable code
#include <algorithm>
#include <cmath>
#include <ctime>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#pragma warning (default:4702)

#include "lexical_cast.h"

#include "Geometry.hpp"

#include "asserter.h"
#include "enforcer.h"
#include "DXEnforcer.h"

#include "dbg_stacktrace.h"
#include "dbg_messagebox.h"
#include "tracker.h"

#include "wrapdbghelp.h"

#undef max
#undef min


/*                                                                    macros
---------------------------------------------------------------------------- */

// safe destruction macros
#define SAFE_DELETE(x)         { if(x) { delete (x); (x) = 0; } }
#define SAFE_DELETE_ARRAY(x)   { if(x) { delete[] (x); (x) = 0; } }
#define SAFE_RELEASE(x)        { if(x) { (x)->Release(); (x) = 0; } }

// directx shader macros
#if (D3D_SDK_VERSION <= 31)
  // Summer Update 2k3 and earlier
  #define  D3D_SHADERPASS_BEGIN(s,p)  (s)->Pass(p)
  #define  D3D_SHADERPASS_END(s)
  #define  D3D_SHADERPASS_COMMIT(s)
#else
  // Summer Update 2k4 and later
  #define  D3D_SHADERPASS_BEGIN(s,p)  (s)->BeginPass(p)
  #define  D3D_SHADERPASS_END(s)      (s)->EndPass()
  #define  D3D_SHADERPASS_COMMIT(s)   (s)->CommitChanges()
#endif

#endif  /* _CH_MAIN_H_ */