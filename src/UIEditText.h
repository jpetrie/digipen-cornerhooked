/*! ========================================================================

      @file    UIEditText.h
      @author  jmp
      @brief   Interface to UIEditText class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _UI_EDITTEXT_H_
#define _UI_EDITTEXT_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "UIElement.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class UIEditText : public UIElement
/*! 
*/
{
  public:
    // ct and dt
    UIEditText(int x,int y,int w,int h,const std::string &text,int textSize = kUIElem_DefaultTextSize);
    ~UIEditText(void);
    
    // accessors
    virtual Type GetType(void) const      { return (kEditText); }
	unsigned int MaxSize(void) const      { return (mMaxSize); }
    bool         IsReadOnly(void) const   { return (mIsReadOnly); }
    bool         IsSingleLine(void) const { return (mIsSingleLine); }
    bool         IsCentered(void) const   { return (mIsCentered); }
    
    // manipulators
	void MaxSize(unsigned int ms ) { mMaxSize = ms ; }
    void ReadOnly(bool ro)         { mIsReadOnly = ro; }
    void SingleLine(bool sl)       { mIsSingleLine = sl; }
    void Centered(bool c)          { mIsCentered = c; }
    
    // text
    std::string GetText(void) const           { return (mText); }
    void        SetText(const std::string &t) { mText = t; mCaretPos = 0; }
    void        InsertText(const std::string &t);
    void        RemoveText(void);
    
    // render
    virtual void Render(void);
    
    // events
    virtual bool CheckLeftClick(int x,int y);
    virtual bool CheckKeyDown(int key);
    
  
  private:
    // data members
    std::string  mText;
	unsigned int mMaxSize;
    
    bool  mIsReadOnly;    //!< Does not draw a background or label, cannot be changed.
    bool  mIsSingleLine;  //!< Takes returns, does not scroll.
    bool  mIsCentered;    //!< Text renders centered.
    
    DXRect *mCaret;
    int     mCaretPos;
};


#endif  /* _UI_GRAPHIC_H_ */