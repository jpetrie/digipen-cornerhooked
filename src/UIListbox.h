/*! ========================================================================

      @file    UIListbox.h
      @author  jmp
      @brief   Interface to UIListbox class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _UI_LISTBOX_H_
#define _UI_LISTBOX_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIElement.h"

#include "UIScrollbar.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class UIListbox : public UIElement
/*! 
*/
{
  public:
    // ct and dt
    UIListbox(int x,int y,int w,int h);
    ~UIListbox(void);
    
    // accessors
    virtual Type GetType(void) const { return (kListbox); }
    
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
	void         Render(int oldX,int oldY);
  
  private:
    // typedefs
    typedef std::vector< std::string >  ItemList;
    
    ItemList  mItems;         //!< Items in the list.
    int       mSelectedItem;  //!< Index of selected item (or -1 for none).
    
    UIScrollbar  *mScrollbar;
};


#endif  /* _UI_LISTBOX_H_ */