/*! ========================================================================

      @file    UIScreen.h
      @author  jmp
      @brief   Interface to UIScreen.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _UI_SCREEN_H_
#define _UI_SCREEN_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIElement.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class UIScreen
/*! Represents a set of user interface elements.
*/
{
  public:
    typedef void (*ExitFinishedCB)(void);
    
    // ct and dt
    UIScreen(void);
    ~UIScreen(void);
  
    // element insertion
    void AddElement(UIElement *e);
    void AddElement(const std::string &name,UIElement *e);
    
    // element retrieval
    size_t     NumElements(void) const { return (mAllElements.size()); }
    UIElement* GetElement(const std::string &name);
    
    // query
    bool CheckLeftClick(int x,int y);
    bool CheckKeyDown(int key);

	// manipulators
	void Focus(UIElement *pFocused);

    // update
    void Update(int x,int y,float elapsed);
    void Render(void);
    void Reset(void);
    void Exit(ExitFinishedCB cb);
    
    
  private:
    // typedefs
    typedef std::vector< UIElement* >           TElemList;  //!< List of unnamed elements.
    typedef std::map< std::string,UIElement* >  TElemMap;   //!< List of named elements.
  
    // data members
    TElemList  mAllElements;  //!< All elements (master list).
    TElemMap   mNamedElem;    //!< For elements with names.
    TElemList  mUnnamedElem;  //!< For elements without names.
    
    UIElement *mFocused;  //!< The element with the focus.
    
    ExitFinishedCB  mExitCallback;  //! Callback to invoke when a screen is done its exit state.
};


/*                                                                prototypes
---------------------------------------------------------------------------- */


#endif  /* _UI_SCREEN_H_ */