/*! ========================================================================

      @file    UIMenu.h
      @author  jmp
      @brief   Interface to UIMenu class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _UI_MENU_H_
#define _UI_MENU_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIElement.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class UIMenu : public UIElement
/*! 
*/
{
  public:
    // ct and dt
    UIMenu(int x,int y,int w,int h);
    ~UIMenu(void);
    
    // accessors
    virtual Type GetType(void) const { return (kMenu); }
    
    // items
    int  AddItem(const std::string &text);
    void ClearItems(void);
    
    // selected
    int GetSelected(void) const { return (mSelectedItem); }
    void SetSelected(int s);
    
    // events
    virtual bool CheckLeftClick(int x,int y);
    
    // render
    virtual void Render(void);
  
  private:
    // typedefs
    typedef std::vector< std::string >  ItemList;
    
    ItemList  mItems;         //!< Items in the list.
    int       mSelectedItem;  //!< Index of selected item (or -1 for none).
};


#endif  /* _UI_MENU_H_ */