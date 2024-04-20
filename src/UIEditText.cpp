/*! ========================================================================

      @file    UIEditText.cpp
      @author  jmp
      @brief   Implementation of UIEditText.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"
#include "game.h"

#include "UIEditText.h"

#include "Clock.h"
#include "Input.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  const float  kCaretBlinkTime = 0.4f;  // In seconds.
  const int    kCaretWidth     = 1;     // In pixels.
  const int    kCaretHeight    = 13;    // In pixels.
  const int    kCaretVBias     = 4;     // In pixels.
}


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
UIEditText::UIEditText(int x,int y,int w,int h,const std::string &text,int textSize)
/*! Constructor.
*/
: UIElement(x,y,w,h,"","",textSize),mText(text),mMaxSize(2000000),
  mIsReadOnly(false),mIsSingleLine(false),mIsCentered(false),mCaretPos(0)
{
  mCaret = new DXRect(mRenderer,D3DCOLOR_ARGB(255,255,255,255),
                                D3DCOLOR_ARGB(255,255,255,255),
                                D3DCOLOR_ARGB(255,255,255,255),
                                D3DCOLOR_ARGB(255,255,255,255));
}

/*  ________________________________________________________________________ */
UIEditText::~UIEditText(void)
/*! Destructor.
*/
{
  SAFE_DELETE(mCaret);
}

/*  ________________________________________________________________________ */
void UIEditText::InsertText(const std::string &t)
/*! Inserts a string into the element at the current edit position.

    @param t  The string to insert.
*/
{
  std::stringstream  fmt;

  if(mText.length() >= mMaxSize)
    return ;

  if(mCaretPos > static_cast<int>(mText.length()))
    mCaretPos = mText.length();

  fmt << mText.substr(0,mCaretPos) << t << mText.substr(mCaretPos);
  mText = fmt.str();
  mCaretPos += static_cast< int >(t.length());
}

/*  ________________________________________________________________________ */
void UIEditText::RemoveText(void)
/*! Removes a character from the current edit position.
*/
{
std::stringstream  fmt;

  // Can't delete if we're at the beginning.
  if(mCaretPos == 0)
    return;
  
  if(mCaretPos > static_cast<int>(mText.length()))
    mCaretPos = static_cast<int>(mText.length());
  
  // Otherwise, take the substrings such that the doomed character goes
  // away.
  fmt << mText.substr(0,mCaretPos - 1) << mText.substr(mCaretPos);
  mText = fmt.str();
  mCaretPos--;
}


/*  ________________________________________________________________________ */
void UIEditText::Render(void)
/*! Render the element.
*/
{
static Clock  clock;             // For controlling caret blinking.
static bool   caret  = true;     // Is the caret rendered?
int           flags  = 0;        // For text rendering.
SIZE          offset = { 0,0 };  // For offsetting due to side caption.
  
  clock.Update();

  // If we have a side caption, figure out how much to
  // offset the rest of the element, and then render the caption.
  if(mIsSideCaption)
  {
    offset = mLabelR->GetStringWidth(mCaption);
    mLabelR->DrawText(mX,mY,offset.cx,kUIElem_LabelHeight,kUIElem_LabelColor,DT_CENTER | DT_VCENTER,mCaption);
  }

  // Draw the background.
  if(mIsEnabled)
    mRectR->DrawRect(mX + offset.cx,mY,mW,mH);
  
  // Control caret blinking.
  if(clock.Accumulated() >= kCaretBlinkTime)
  {
    caret = !caret;
    clock.Reset();
  }
  
  // Set text render flags as appropriate.
  if(mIsCentered)
    flags |= DT_CENTER;
  else
    flags |= DT_LEFT;
  if(!mIsSingleLine)
    flags |= DT_WORDBREAK;
  else
    flags |= DT_VCENTER;
  
  // Render the text. If disabled, we're "read-only", so render the color of
  // label text.
  if(mIsEnabled)
    mFontR->DrawText(mX + offset.cx,mY,mW,mH,kUIElem_TextColor,flags,mText.c_str());
  else
    mFontR->DrawText(mX + offset.cx,mY,mW,mH,kUIElem_LabelColor,flags,mText.c_str());
    
  // Render the caret.
  if(!mIsReadOnly && mIsEnabled && mIsFocused && caret)
  {
  SIZE  caretPos = mFontR->GetCharPosition(mText,mW,mCaretPos);
  
    mCaret->DrawRect(mX + caretPos.cx + offset.cx,mY + caretPos.cy + kCaretVBias,kCaretWidth,kCaretHeight);
  }
}

/*  ________________________________________________________________________ */
bool UIEditText::CheckLeftClick(int x,int y)
/*! Handle left click event.

    @param x  The X coordinate of the click.
    @param y  The Y coordinate of the click.
    
    @return
    True if the click was handled, false otherwise.
*/
{
SIZE  offset = { 0,0 };  // For dealing with a side caption.

  if(mIsReadOnly || !mIsEnabled)
    return (false);
  
  if(mIsSideCaption)
    offset = mLabelR->GetStringWidth(mCaption);
    
  // First we get the index of the character clicked on (zero based).
  mCaretPos = mFontR->GetCharIndex(mText,mW,x - (mX + offset.cx),y - mY);
  
  // Now we adjust the index slightly; if the user clicked more than
  // halfway through the character (on the right side), we put the cursor
  // after the character; otherwise it stays before.
SIZE  charSz  = mFontR->GetCharWidth(mText[mCaretPos]);
SIZE  charPos = mFontR->GetCharPosition(mText,mW,mCaretPos);
  
  if(x >= (mX + charPos.cx + offset.cx) + (charSz.cx / 2))
    mCaretPos++;
  
  // Make sure we don't put the position out of bounds.
  if(mCaretPos > static_cast<int>(mText.length()))
    mCaretPos = static_cast<int>(mText.length());
  
  return (true);
}

/*  ________________________________________________________________________ */
bool UIEditText::CheckKeyDown(int key)
/*! Handle key down event.

    @param key  The scancode of the key.
    
    @return
    True if the keystroke was handled, false otherwise.
*/
{
char        c[2] = { 0,0 }; 
std::string s;
  
  if(mIsReadOnly || !mIsEnabled)
    return (false);
    
  c[0] = Input::GetKeyASCII(key);
  if(c[0] == 0)
  {
    // Zero return from GetKeyASCII() means there is no ASCII representation.
    // It must be some other key.
    if(key == DIK_LEFT)
      if(--mCaretPos < 0)
        mCaretPos = 0;
    if(key == DIK_RIGHT)
      if(++mCaretPos > static_cast<int>(mText.length()))
        mCaretPos = static_cast< unsigned int >(mText.length());
	if(key == DIK_HOME)
		mCaretPos = 0 ;
	if(key == DIK_END)
		mCaretPos = static_cast< unsigned int >(mText.length());
	if(key == DIK_DELETE)
	{
	  if ( mCaretPos < static_cast< int >(mText.length()) )
	  {
		  ++mCaretPos ;
		  RemoveText() ;
	  }
	}

  }
  else
  {
    // A few other special cases here, like backspace,
    // and returns (which may not get printed if we're a single
    // line edit box).
    if(c[0] == '\b')
      RemoveText();
    else if((c[0] == '\n' || c[0] == '\r') && mIsSingleLine)
    {
      InvokeCallback(UIElement::kLoseFocus);
	  Game::Get()->GetScreen()->Focus( this ) ;
    }
	// Ensure it is a valid printable ASCII character (cull tab, escape, etcetera)
	// (0x00-0x19 and 0x7f are control characters; 0x80+ are not on normal US keyboards)
    else if (c[0] >= 0x20 && c[0] < 0x7f)
    {
      // Stick it in.
      s = c;
      InsertText(s);
    }
  }

  return (true);
}