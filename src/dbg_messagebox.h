/*! ========================================================================
    
      @file    dbg_messagebox.h
      @author  jmp
      @brief   Interface to debugging messagebox.
   
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.  
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _DBG_MESSAGEBOX_H_
#define _DBG_MESSAGEBOX_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

namespace dbg
{


/*                                                                 constants
---------------------------------------------------------------------------- */

// result codes
const int  mbr_btn1 = 1;
const int  mbr_btn2 = 2;
const int  mbr_btn3 = 3;


/*                                                                   structs
---------------------------------------------------------------------------- */

struct msgboxparams
//! Advanced messagebox options.
{
  bool         fixed_font;    //!< If true, use a fixed-width font for message display.
  std::string  btn1_caption;  //!< Caption for the first button.
  std::string  btn2_caption;  //!< Caption for the second button.
  std::string  btn3_caption;  //!< Caption for the third button.
  std::string  btn4_caption;  //!< Caption for the fourth button.
};


/*                                                                prototypes
---------------------------------------------------------------------------- */

// messagebox
int messagebox(HWND owner,const LPTSTR text,LPTSTR caption,UINT type,const msgboxparams *params = 0);


}       /* namespace dbg */
#endif  /* _DBG_MESSAGEBOX_H_ */