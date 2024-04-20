/*! ========================================================================
    
      @file    dbg_stacktrace.cpp
      @author  jmp
      @brief   Implementation of stack trace tools.  
    
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "dbg_stacktrace.h"

#include <sstream>
#include <iostream>

#include "dbg_messagebox.h"

namespace dbg
{


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
static void pathtail(const char *in,char *out,unsigned int sz)
/*! Get the tail of a path string (the last element).

    @param in   The input string.
    @param out  The output string.
    @param sz   The size of the output string.

    @internal
*/
{
unsigned int  len = 0;
unsigned int  idx = 0;

  // Idiot-proofing.
  if(0 == in || 0 == out)
    return;

  // Walk the string.
  while(in[len] != 0)
  {
    // Save the position of a path delimiter.
    if(in[len] == '\\')
      idx = len;
    ++len;
  }
  
  // Copy tail.
  if(len - idx < sz)
    sz = len - idx;
  for(unsigned int i = 0; i <= sz; ++i)
    out[i] = in[idx + 1 + i];
  out[sz] = 0;
}

/*  ________________________________________________________________________ */
void stacktrace(CONTEXT& c )
/*! Display a message box with a trace of the stack of the given context.

    @param c  The context to recover the stack trace for.
*/
{
bool               done = false;
std::stringstream  out;

  // Initialize the symbol handler.
  // We must do this invasively in order to get the
  // result we want (therefor, this can take some time).
  dbghelp::SymInitialize(::GetCurrentProcess(),0,true);

DWORD  symOptions;

  // Set up symbol options. We want to make sure we get the line
  // numbers out of the trace, because they're damn useful.
	symOptions = dbghelp::SymGetOptions();
	symOptions |= SYMOPT_LOAD_LINES;
	dbghelp::SymSetOptions(symOptions);


STACKFRAME64 sf;

  // Initialize the STACKFRAME structure; this only 
  // needs to be done on x86, for the record.
  ::memset( &sf, 0, sizeof(sf) ); 
  sf.AddrPC.Offset       = c.Eip;
  sf.AddrPC.Mode         = AddrModeFlat;
  sf.AddrStack.Offset    = c.Esp;
  sf.AddrStack.Mode      = AddrModeFlat;
  sf.AddrFrame.Offset    = c.Ebp;
  sf.AddrFrame.Mode      = AddrModeFlat;
 
  while(!done)
  {
    if(!dbghelp::StackWalk(IMAGE_FILE_MACHINE_I386,
                           ::GetCurrentProcess(),
                           ::GetCurrentThread(),
                           &sf,
                           &c,
                           0,
                           reinterpret_cast< PFUNCTION_TABLE_ACCESS_ROUTINE64 >(dbghelp::get("SymFunctionTableAccess64")),
                           reinterpret_cast< PGET_MODULE_BASE_ROUTINE64 >(dbghelp::get("SymGetModuleBase64")),
                           0))
    {
      done = true;
      continue;
    }

    // Sanity check the frame.
    if(0 == sf.AddrFrame.Offset)
    {
      done = false;
      continue;
    }
    out << sf.AddrPC.Offset << " " << sf.AddrFrame.Offset << " : ";
 
    // The symbol buffer needs to be bigger than the symbol
    // structure (dbghelp is weird that way).
  BYTE                symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 512];
  PIMAGEHLP_SYMBOL64  symbol = reinterpret_cast< PIMAGEHLP_SYMBOL64 >(symbolBuffer);
    
    symbol->SizeOfStruct  = sizeof(symbolBuffer);
    symbol->MaxNameLength = 512;
                         
  DWORD64  disp = 0;

    // Try and recover symbol information (name, line info, et cetera).
    if(dbghelp::SymGetSymFromAddr(::GetCurrentProcess(),sf.AddrPC.Offset,&disp,symbol))
    {
    IMAGEHLP_LINE64  line;
    DWORD            linedisp;

      out << symbol->Name << "+" << disp;
      
      ::memset(&line,0,sizeof(line));
      line.SizeOfStruct = sizeof(line);
      if(dbghelp::SymGetLineFromAddr(::GetCurrentProcess(),sf.AddrPC.Offset,&linedisp,&line))
      {
      char  l[256];

        pathtail(line.FileName,l,256);
        out << "(" << l << "," << line.LineNumber << "\n";
      }
      else
      {
        //err cant get line
      }
    }
    else
    {
      //err sym not found
    }
  }
  
  dbghelp::SymCleanup(::GetCurrentProcess());
     
TCHAR *array = new TCHAR[out.str().size() + 1];

  for(unsigned int i = 0; i < out.str().size() + 1; ++i)
    array[i] = out.str().c_str()[i];
  dbg::messagebox(0,array,"Stack Trace",0);
  
}

}  /* namespace dbg */
