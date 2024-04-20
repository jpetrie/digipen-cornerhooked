/*! ========================================================================

      @file    UIPanel.h
      @author  jmp
      @brief   Interface to UIPanel class.
      
    ======================================================================== 
 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _UI_PANEL_H_
#define _UI_PANEL_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "UIElement.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class UIPanel : public UIElement
/*! 
*/
{
  public:
    // ct and dt
    UIPanel(int x,int y,int w,int h);
    ~UIPanel(void);
    
    // accessors
    virtual Type GetType(void) const { return (kPanel); }
    
    // element insertion
    void AddElement(UIElement *e);
    void AddElement(const std::string &name,UIElement *e);
    
    // element retrieval
    size_t     NumElements(void) const { return (mAllElements.size()); }
    UIElement* GetElement(const std::string &name);
  
    // render
    virtual void Render(void);
    
    // query
    bool CheckLeftClick(int x,int y,UIElement *&hit);
    
    
  private:
    // typedefs
    typedef std::vector< UIElement* >           TElemList;  //!< 
    typedef std::map< std::string,UIElement* >  TElemMap;   //!< 
  
    // data members
    TElemList  mAllElements;  //!< All elements (master list).
    TElemMap   mNamedElem;    //!< For elements with names.
    TElemList  mUnnamedElem;  //!< For elements without names.
};


#endif  /* _UI_PANEL_H_ */