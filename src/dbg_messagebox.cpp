/*! ========================================================================
    
      @file    dbg_messagebox.cpp
      @author  jmp
      @brief   Implementation of debugging messagebox.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "dbg_messagebox.h"

namespace dbg
{


/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  // default values
  const LPTSTR        ikDefaultCaptionStr = "Error";
  const msgboxparams  ikDefaultParams =
  { 
    false,  // True for fixed font, false for regular.
    "OK",   // 1st button caption.
    "",     // 2nd button caption.
    "",     // 3rd button caption.
    ""      // 4th button caption.
  };
  
  // size and position
  const short  ikMinWidth         = 150;
  const short  ikMinHeight        = 75;
  const short  ikMaxWidth         = 300;
  const short  ikMaxHeight        = 200;
  const short  ikInteriorHSpacing = 5;
  const short  ikInteriorVSpacing = 5;
  const short  ikButtonWidth      = 45;
  const short  ikButtonHeight     = 12;
  
  // ids
  const short ikTextID = 1;
  const short ikBtn1ID = 2;
  const short ikBtn2ID = 3;
  const short ikBtn3ID = 4;
}


/*                                                                   classes
---------------------------------------------------------------------------- */

class msgbox
{
  public:
    // enums
    enum item_type
    //! Dialog item type codes.
    {
      button = 0x0080,  //!< A button.
      text   = 0x0082,  //!< A text object.
    };
    
    
    // ct and dt
     msgbox(const std::string &caption,short w,short h,const msgboxparams &p);
    ~msgbox(void); 
    
    // items
    void add(item_type type,short id,short x,short y,short w,short h,const std::string &name,int flags);

    // display
    int display(void);
    
    // accessors
    bool p_fixedfont(void) const { return (mParams.fixed_font); }
    
  
  private:
    // structs
    struct item_entry
    //! Encapsulates a dialog item.
    {
      short        x;      //!< X coordinate.
      short        y;      //!< Y coordinate.
      short        w;      //!< Width of item.
      short        h;      //!< Height of item.
      std::string  name;   //!< Item name.
      int          flags;  //!< Item flags.
      item_type    type;   //!< Item type (see item_type enum).
      short        id;     //!< Item ID.
    };
    
    
    // contract: noncopyable
    msgbox(const msgbox &m);
    msgbox& operator=(const msgbox &m);
    
    // alignment
    LPWORD n_align(LPWORD word);
    
    // dialog procedure
    static BOOL CALLBACK n_dialog_proc(HWND dlog,UINT msg,WPARAM wp,LPARAM lp);
    
    
    // data members
    std::string                mCaption;  // Messagebox title.
    msgboxparams               mParams;   // Messagebox parameters.
    short                      mWidth;
    short                      mHeight;
    std::vector< item_entry >  mItems;    // Items in the messagebox.
};


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
msgbox::msgbox(const std::string &caption,short w,short h,const msgboxparams &p)
/*! Constructor.

    @param caption  The messagebox caption.
    @param w        The width of the messagebox.
    @param h        The height of the messagebox.
    
    @internal
*/
: mCaption(caption),
  mWidth(w),mHeight(h),
  mParams(p)
{
}

/*  ________________________________________________________________________ */
msgbox::~msgbox(void)
/*! Destructor.

    @internal
*/
{
}
    
/*  ________________________________________________________________________ */
void msgbox::add(item_type type,short id,short x,short y,short w,short h,const std::string &name,int flags)
/*! Add an item to the message box.

    @param 
    
    @internal
*/
{
item_entry  item;

  // Fill out the item.
  item.x     = x;
  item.y     = y;
  item.w     = w;
  item.h     = h;
  item.name  = name;
  item.flags = flags;
  item.type  = type;
  item.id    = id;
  
  mItems.push_back(item);
}

/*  ________________________________________________________________________ */
int msgbox::display(void)
/*! Display the messagebox.

    @return
    0 if an error occured; otherwise a result code corresponding to the button
    hit to close the messagebox.
    
    @internal
*/
{
HGLOBAL            globalMem;
LPDLGTEMPLATE      dialogTmpl;
LPDLGITEMTEMPLATE  itemTmpl;
LPWORD             ptr;
LPWSTR             ptrStr;
LRESULT            ret;
int                cnt;
int                sz = sizeof(DLGTEMPLATE) + (2 * sizeof(WORD)) +  ((mCaption.size() + 1) * sizeof(WCHAR));

  // Adjust to account for alignment.
	sz = (sz + 3) & ~3;
	for(unsigned int i = 0; i < mItems.size(); ++i)
	{
	int  len = sizeof(DLGITEMTEMPLATE) + 3 * sizeof(WORD) + (((mItems[i].name.size() + 1)) * sizeof(WCHAR));
  
    // Alignment, again. But not for last item.
		if(i != mItems.size() - 1)
			len = (len + 3) & ~3;
	  sz += len;
	}

  // The dialog template must reside in "global memory".
  globalMem = ::GlobalAlloc(GMEM_ZEROINIT,2048);
  if(!globalMem)
      return (-1);
  dialogTmpl = reinterpret_cast< LPDLGTEMPLATE >(::GlobalLock(globalMem));

  // Define dialog box.
  dialogTmpl->style = (WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION);
  dialogTmpl->cdit  = static_cast< WORD >(mItems.size());
  dialogTmpl->x  = 0;
  dialogTmpl->y  = 0;
  dialogTmpl->cx = mWidth;
  dialogTmpl->cy = mHeight;
  
  // Move the pointer.
  ptr = reinterpret_cast< LPWORD >(dialogTmpl + 1);
  
  // No extra creation data.
  *ptr++ = 0;
  *ptr++ = 0;
  
  // Copy caption.
  ptrStr = reinterpret_cast< LPWSTR >(ptr);
  cnt    = ::MultiByteToWideChar(CP_ACP,0,mCaption.c_str(),-1,ptrStr,static_cast< int >(mCaption.size()) + 1);
  ptr    += cnt;
  
  // Copy items (the first item is field for the dialog text).
  for(unsigned int i = 0; i < mItems.size(); ++i)
  {
  const char *t = mItems[i].name.c_str();
  
    ptr = n_align(ptr);
    itemTmpl        = (LPDLGITEMTEMPLATE) ptr;
    itemTmpl->x     = mItems[i].x;
    itemTmpl->y     = mItems[i].y;
    itemTmpl->cx    = mItems[i].w;
    itemTmpl->cy    = mItems[i].h;
    itemTmpl->id    = mItems[i].id;
    itemTmpl->style = mItems[i].flags;
    
    // Item type.
    ptr    = reinterpret_cast< LPWORD >(itemTmpl + 1);
    *ptr++ = 0xFFFF;
    *ptr++ = static_cast< WORD >(mItems[i].type);
  
    // Copy text. 
#pragma warning(push, 1)
#pragma warning(disable: 4706)
    for(ptrStr = reinterpret_cast< LPWSTR >(ptr); *ptrStr++ = static_cast< WCHAR >(*t++); )
      ;
#pragma warning(default: 4706)
#pragma warning(pop)

    ptr    = reinterpret_cast< LPWORD >(ptrStr);
    *ptr++ = 0;
  }

  // Unlock memory.
  ::GlobalUnlock(globalMem);
  
  // DialogBoxIndirect() does not return until something interesting has happened.
  ::MessageBeep(MB_OK);
  ret = ::DialogBoxIndirectParam(0,
                                 reinterpret_cast< LPDLGTEMPLATE >(globalMem),
                                 0,
                                 reinterpret_cast< DLGPROC >(n_dialog_proc),
                                 reinterpret_cast< LPARAM >(this)); 
  ::GlobalFree(globalMem); 
  return (static_cast< int >(ret));
}

/*  ________________________________________________________________________ */
BOOL CALLBACK msgbox::n_dialog_proc(HWND dlog,UINT msg,WPARAM wp,LPARAM lp)
/*! Messagebox dialog procedure.

    @param dlog  Handle to the dialog window.
    @param msg   Message code.
    @param wp    Message payload data.
    @param lp    Message payload data.
    
    @return
    True if the dialog should close, false otherwise.
    
    @internal
*/
{
msgbox *self = reinterpret_cast< msgbox* >(::GetWindowLongPtr(dlog,GWLP_USERDATA));

  switch(msg)
  {
    case WM_INITDIALOG:
    // Dialog initialization.
    {
    HFONT  font  = static_cast< HFONT >(::GetStockObject(DEFAULT_GUI_FONT));
    HFONT  ffont = static_cast< HFONT >(::GetStockObject(ANSI_FIXED_FONT));
    
      // Store the pointer in the window's userdata.
      ::SetWindowLongPtr(dlog,GWLP_USERDATA,lp);
			self = reinterpret_cast< msgbox* >(lp);
			
			// The message is displayed in a fixed width font.
			if(self->p_fixedfont())
			  ::SendMessage(::GetDlgItem(dlog,ikTextID),WM_SETFONT,reinterpret_cast< WPARAM >(ffont),0);
			else
			  ::SendMessage(::GetDlgItem(dlog,ikTextID),WM_SETFONT,reinterpret_cast< WPARAM >(font),0);
			  
			// Use default GUI font.
			for(unsigned int i = 1; i < self->mItems.size(); ++i)
			  ::SendMessage(::GetDlgItem(dlog,self->mItems[i].id),WM_SETFONT,reinterpret_cast< WPARAM >(font),0);
			  
			// Center the dialog on the main monitor.
	  RECT  workRect;
	  int   wx,wy,ww,wh;
	  
	    ::GetClientRect(dlog,&workRect);
	    ww = (workRect.right - workRect.left);
	    wh = (workRect.bottom - workRect.top);
			::SystemParametersInfo(SPI_GETWORKAREA,0,&workRect,0);
			wx = ((workRect.right - workRect.left) >> 1) - (ww >> 1);
			wy = ((workRect.bottom - workRect.top) >> 1) - (wh >> 1);
			::SetWindowPos(dlog,HWND_TOPMOST,wx,wy,ww,wh,0);
			
			// Done.
			return (false);
    }
    break;
    case WM_COMMAND:
    {
      switch(LOWORD(wp))
      {
        case ikBtn1ID:
          ::EndDialog(dlog,mbr_btn1);
        break;
        case ikBtn2ID:
          ::EndDialog(dlog,mbr_btn2);
        break;
        case ikBtn3ID:
          ::EndDialog(dlog,mbr_btn3);
        break;
        default:
        {
          // Do nothing.
        }
      }
      return (true);
    }
    break;
    default:
    {
      // Do nothing.
    }
  }
  
  return (false);
}

/*  ________________________________________________________________________ */
LPWORD msgbox::n_align(LPWORD word)
/*! @brief Align a word pointer to a word boundary.

    @param word  The pointer to word-align.
    
    @return
    The word-aligned pointer.

    @internal
*/
{
ULONG  aligned;

  aligned = reinterpret_cast< ULONG >(word);
  aligned += 3;
  aligned >>= 2;
  aligned <<= 2;
  return (reinterpret_cast< LPWORD >(aligned));
}

/*  ________________________________________________________________________ */
int messagebox(HWND /*owner*/,const LPTSTR text,LPTSTR caption,UINT type,const msgboxparams *params)
/*! Present a messagebox.

    The first three parameters of the function work exactly as those to the
    Windows API MessageBox() function; the type parameter is the same except
    for the following differences:
    
    - MB_ABORTRETRYIGNORE operates as MB_CANCELTRYCONTINUE.
    - The MB_ICON* and MB_*MODAL types are ignored.
    - The MB_DEFBUTTON* types are ignored.
    - The MB_HELP type is ignored.
    
    @param owner    The owning window; may be 0.
    @param text     The text of the message box.
    @param caption  The caption (title) of the message box.
    @param type     Basic messagebox type options.
    @param params   Optional pointer to advanced messagebox options. If not null,
                    all fields of the msgboxparams structure must be filled out.
                    The options specified in the parameter structure override
                    those specified by the type parameter.

    @return
    -1 on failure, otherwise 
*/
{
msgbox       *mb;
msgboxparams  p;

  // No display if no message.
  if(0 == text)
    return (-1);

  // Default parameter values.
  if(0 == caption || 0 == caption[0])
    caption = ikDefaultCaptionStr;
  if(0 != params)
    p = *params;
  else
  {
    // Initially defaults.
    p = ikDefaultParams;
  
    // Apply adjustments based on type parameter.
    if(!!(type & MB_CANCELTRYCONTINUE) ||
      !!(type & MB_ABORTRETRYIGNORE))
    {
      p.btn1_caption = "Cancel";
      p.btn2_caption = "Try Again";
      p.btn3_caption = "Continue";
    }
    if(!!(type & MB_OK))
    {
      p.btn1_caption = "OK";
      p.btn2_caption = "";
      p.btn3_caption = "";
    }
    if(!!(type & MB_OKCANCEL))
    {
      p.btn1_caption = "OK";
      p.btn2_caption = "Cancel";
      p.btn3_caption = "";
    }
    if(!!(type & MB_RETRYCANCEL))
    {
      p.btn1_caption = "Retry";
      p.btn2_caption = "Cancel";
      p.btn3_caption = "";
    }
    if(!!(type & MB_YESNO))
    {
      p.btn1_caption = "Yes";
      p.btn2_caption = "No";
      p.btn3_caption = "";
    }
    if(!!(type & MB_YESNOCANCEL))
    {
      p.btn1_caption = "Yes";
      p.btn2_caption = "No";
      p.btn3_caption = "Cancel";
    }
  }
  
  // Must have at least one button.
  if(p.btn1_caption.empty() && p.btn2_caption.empty() && p.btn3_caption.empty() && p.btn4_caption.empty())
    p.btn1_caption = "OK";

  // Calculate the length of the text.
HDC    dc   = ::CreateDC("DISPLAY",0,0,0);
HFONT  font = static_cast< HFONT >(::GetStockObject(ANSI_FIXED_FONT));
HFONT  old  = static_cast< HFONT >(::SelectObject(dc,font));
RECT   rect;

  ::SetRect(&rect,0,0,ikMaxWidth,0);
  ::DrawText(dc,text,-1,&rect,DT_WORDBREAK | DT_CALCRECT);

  font = static_cast< HFONT >(::SelectObject(dc,old));
  ::DeleteObject(font);
  ::DeleteDC(dc);
  
  // Build message box.
short  width  = static_cast< short >(rect.right);
short  height = static_cast< short >(rect.bottom);

  if(width < ikMinWidth)
    width = ikMinWidth;
  if(width > ikMaxWidth)
    width = ikMaxWidth;
  if(height < ikMinHeight)
    height = ikMinHeight;
  if(height > ikMaxHeight)
    height = ikMaxHeight;
  
  mb = new msgbox(caption,
                  width + (3 * ikInteriorHSpacing) + ikButtonWidth,
                  height + (2 * ikInteriorVSpacing),p);
  
  mb->add(msgbox::text,ikTextID,ikInteriorHSpacing,ikInteriorVSpacing,width,height,text,(WS_CHILD | WS_VISIBLE));
  if(!p.btn1_caption.empty())
    mb->add(msgbox::button,ikBtn1ID,width + (2 * ikInteriorHSpacing),ikInteriorVSpacing,ikButtonWidth,ikButtonHeight,p.btn1_caption,(WS_CHILD | WS_VISIBLE));
  if(!p.btn2_caption.empty())
    mb->add(msgbox::button,ikBtn2ID,width + (2 * ikInteriorHSpacing),ikInteriorVSpacing + ikButtonHeight + 3,ikButtonWidth,ikButtonHeight,p.btn2_caption,(WS_CHILD | WS_VISIBLE));
  if(!p.btn3_caption.empty())
    mb->add(msgbox::button,ikBtn3ID,width + (2 * ikInteriorHSpacing),ikInteriorVSpacing + (2 * ikButtonHeight) + 6,ikButtonWidth,ikButtonHeight,p.btn3_caption,(WS_CHILD | WS_VISIBLE));
  
  return (mb->display()); 
}
  
}  /* namespace dbg */