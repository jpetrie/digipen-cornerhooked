/*! ========================================================================

      @file    wrapdbghelp.cpp
      @author  jmp
      @brief   Implementation of dbghelp.dll wrapper.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "wrapdbghelp.h"

#include <cstdio>

namespace dbghelp
{


/*                                                                  typedefs
---------------------------------------------------------------------------- */

namespace
{
  // function pointers
  typedef BOOL     (__stdcall *ptr_SymInitialize)(HANDLE,PSTR,BOOL);
  typedef BOOL     (__stdcall *ptr_SymCleanup)(HANDLE);
  typedef BOOL     (__stdcall *ptr_SymEnumSymbols)(HANDLE,ULONG64,PCSTR,PSYM_ENUMERATESYMBOLS_CALLBACK,PVOID);
  typedef BOOL     (__stdcall *ptr_SymSetContext)(HANDLE,PIMAGEHLP_STACK_FRAME,PIMAGEHLP_CONTEXT);
  typedef DWORD    (__stdcall *ptr_SymSetOptions)(DWORD);
  typedef DWORD    (__stdcall *ptr_SymGetOptions)(void);
  typedef BOOL     (__stdcall *ptr_SymFromName)(HANDLE,LPSTR,PSYMBOL_INFO);
  typedef BOOL     (__stdcall *ptr_SymGetTypeInfo)(HANDLE,DWORD64,ULONG,IMAGEHLP_SYMBOL_TYPE_INFO,PVOID);
  typedef BOOL     (__stdcall *ptr_StackWalk)(DWORD,HANDLE,HANDLE,LPSTACKFRAME64 sf,PVOID,PREAD_PROCESS_MEMORY_ROUTINE64,PFUNCTION_TABLE_ACCESS_ROUTINE64,PGET_MODULE_BASE_ROUTINE64,PTRANSLATE_ADDRESS_ROUTINE64);
  typedef PVOID    (__stdcall *ptr_SymFunctionTableAccess)(HANDLE,DWORD64);
  typedef DWORD64  (__stdcall *ptr_SymGetModuleBase)(HANDLE,DWORD64);
  typedef BOOL     (__stdcall *ptr_SymGetSymFromAddr)(HANDLE,DWORD64,PDWORD64,PIMAGEHLP_SYMBOL64);
  typedef BOOL     (__stdcall *ptr_SymGetLineFromAddr)(HANDLE,DWORD64,PDWORD,PIMAGEHLP_LINE64);
  typedef DWORD    (__stdcall *ptr_UnDecorateSymbolName)(PCSTR,PSTR,DWORD,DWORD); 
}


/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  // dll strings
  const char  k_dll_dbghelp[]      = "dbghelp.dll";
  const char  k_dllf_syminit[]     = "SymInitialize";
  const char  k_dllf_symcleanup[]  = "SymCleanup";
  const char  k_dllf_symenum[]     = "SymEnumSymbols";
  const char  k_dllf_symsetctx[]   = "SymSetContext";
  const char  k_dllf_symsetopts[]  = "SymSetOptions";
  const char  k_dllf_symgetopts[]  = "SymGetOptions";
  const char  k_dllf_symfromname[] = "SymFromName";
  const char  k_dllf_symgetinfo[]  = "SymGetTypeInfo";
  const char  k_dllf_stackwalk[]   = "StackWalk64";
  const char  k_dllf_symfta[]      = "SymFunctionTableAccess64";
  const char  k_dllf_symgmb[]      = "SymGetModuleBase64";
  const char  k_dllf_symgsfa[]     = "SymGetSymFromAddr64";
  const char  k_dllf_symglfa[]     = "SymGetLineFromAddr64";
  const char  k_dllf_undname[]     = "UnDecorateSymbolName";
  
  // error strings
  const char  k_err_caption[]          = "Fatal Error";
  const char  k_err_dbghelp_missing[]  = "A required DLL (dbghlp.dll) was not found. The application cannot continue.";
  const char  k_err_dllfunc_missing[]  = "A required DLL function (%s) was not found. The application cannot continue.";
  const int   k_err_dllfunc_missing_sz = 512;
}


/*                                                                 variables
---------------------------------------------------------------------------- */

namespace
{
  // loaded flag
  bool  gLoaded = false;
  
  // dll
  HMODULE  gDbgHelpDLL;

  // functions
  ptr_SymInitialize           gSymInitialize;
  ptr_SymCleanup              gSymCleanup;
  ptr_SymEnumSymbols          gSymEnumSymbols;
  ptr_SymSetContext           gSymSetContext;
  ptr_SymSetOptions           gSymSetOptions;
  ptr_SymGetOptions           gSymGetOptions;
  ptr_SymFromName             gSymFromName;
  ptr_SymGetTypeInfo          gSymGetTypeInfo;
  ptr_StackWalk               gStackWalk;
  ptr_SymFunctionTableAccess  gSymFunctionTableAccess;
  ptr_SymGetModuleBase        gSymGetModuleBase;
  ptr_SymGetSymFromAddr       gSymGetSymFromAddr;
  ptr_SymGetLineFromAddr      gSymGetLineFromAddr;
  ptr_UnDecorateSymbolName    gUnDecorateSymbolName;
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
void load(void)
/*! Loads the dbghelp.dll wrapper.
    
    It is safe to call this function multiple times; the library will only be
    loaded once. If loading fails, a message is displayed and load() calls
    abort() -- throwing an exception at this point might not be sane.
*/
{
  if(gLoaded)
    return;
  gLoaded = true;
  
  // Load DbgHelp; if it isn't there, fail miserably.
  gDbgHelpDLL = ::LoadLibrary(k_dll_dbghelp);
  if(gDbgHelpDLL == 0)
  {
    ::MessageBox(0,k_err_dbghelp_missing,k_err_caption,MB_OK);
    ::abort();
  }
  
  // Set up function pointers.
  try
  {
    if(0 == (gSymInitialize = (ptr_SymInitialize)::GetProcAddress(gDbgHelpDLL,k_dllf_syminit)))
      throw k_dllf_syminit;
    if(0 == (gSymCleanup = (ptr_SymCleanup)::GetProcAddress(gDbgHelpDLL,k_dllf_symcleanup)))
      throw k_dllf_symcleanup;
    if(0 == (gSymEnumSymbols = (ptr_SymEnumSymbols)::GetProcAddress(gDbgHelpDLL,k_dllf_symenum)))
      throw k_dllf_symenum;
    if(0 == (gSymSetContext = (ptr_SymSetContext)::GetProcAddress(gDbgHelpDLL,k_dllf_symsetctx)))
      throw k_dllf_symsetctx;
    if(0 == (gSymSetOptions = (ptr_SymSetOptions)::GetProcAddress(gDbgHelpDLL,k_dllf_symsetopts)))
      throw k_dllf_symsetopts;
    if(0 == (gSymGetOptions = (ptr_SymGetOptions)::GetProcAddress(gDbgHelpDLL,k_dllf_symgetopts)))
      throw k_dllf_symgetopts;
    if(0 == (gSymFromName = (ptr_SymFromName)::GetProcAddress(gDbgHelpDLL,k_dllf_symfromname)))
      throw k_dllf_symfromname;
    if(0 == (gSymGetTypeInfo = (ptr_SymGetTypeInfo)::GetProcAddress(gDbgHelpDLL,k_dllf_symgetinfo)))
      throw k_dllf_symgetinfo;
    if(0 == (gStackWalk = (ptr_StackWalk)::GetProcAddress(gDbgHelpDLL,k_dllf_stackwalk)))
      throw k_dllf_stackwalk;
    if(0 == (gSymFunctionTableAccess = (ptr_SymFunctionTableAccess)::GetProcAddress(gDbgHelpDLL,k_dllf_symfta)))
      throw k_dllf_symfta;
    if(0 == (gSymGetModuleBase = (ptr_SymGetModuleBase)::GetProcAddress(gDbgHelpDLL,k_dllf_symgmb)))
      throw k_dllf_symgmb;
    if(0 == (gSymGetSymFromAddr = (ptr_SymGetSymFromAddr)::GetProcAddress(gDbgHelpDLL,k_dllf_symgsfa)))
      throw k_dllf_symgsfa;
    if(0 == (gSymGetLineFromAddr = (ptr_SymGetLineFromAddr)::GetProcAddress(gDbgHelpDLL,k_dllf_symglfa)))
      throw k_dllf_symglfa;
    if(0 == (gUnDecorateSymbolName = (ptr_UnDecorateSymbolName)::GetProcAddress(gDbgHelpDLL,k_dllf_undname)))
      throw k_dllf_undname;
  }
  catch(const char *e)
  {
  char  buffer[k_err_dllfunc_missing_sz];
  
    ::sprintf(buffer,k_err_dllfunc_missing,e);
    ::MessageBox(0,buffer,k_err_caption,MB_OK);
    ::abort();
  }
}

/*  ________________________________________________________________________ */
void free(void)
/*! Frees the dbghelp.dll wrapper.
*/
{
  ::FreeLibrary(gDbgHelpDLL);
  gLoaded = false;
}

void* get(const std::string &f)
{
  if(!gLoaded)
    load();
  return (::GetProcAddress(gDbgHelpDLL,f.c_str()));
}

/*  ________________________________________________________________________ */
BOOL SymInitialize(HANDLE h,PSTR p,BOOL i)
/*! Call SymInitialize().
*/
{
  if(!gLoaded)
    load();
  return (gSymInitialize(h,p,i));
}

/*  ________________________________________________________________________ */
BOOL SymCleanup(HANDLE h)
/*! Call SymCleanup().
*/
{
  if(!gLoaded)
    load();
  return (gSymCleanup(h));
}

/*  ________________________________________________________________________ */
BOOL SymEnumSymbols(HANDLE h,ULONG64 b,PCSTR m,PSYM_ENUMERATESYMBOLS_CALLBACK c,PVOID u)
/*! Call SymEnumSymbols().
*/
{
  if(!gLoaded)
    load();
  return (gSymEnumSymbols(h,b,m,c,u));
}

/*  ________________________________________________________________________ */
BOOL SymSetContext(HANDLE h,PIMAGEHLP_STACK_FRAME s,PIMAGEHLP_CONTEXT c)
/*! Call SymSetContext().
*/
{
  if(!gLoaded)
    load();
  return (gSymSetContext(h,s,c));
}

/*  ________________________________________________________________________ */
DWORD SymSetOptions(DWORD o)
/*! Call SymSetOptions().

    @param o  Symbol options.
    
    @return
    The options previous in effect.
*/
{
  if(!gLoaded)
    load();
  return (gSymSetOptions(o));
}

/*  ________________________________________________________________________ */
DWORD SymGetOptions(void)
/*! Call SymGetOptions().
    
    @return
    The options in effect.
*/
{
  if(!gLoaded)
    load();
  return (gSymGetOptions());
}

/*  ________________________________________________________________________ */
BOOL SymFromName(HANDLE h,LPSTR n,PSYMBOL_INFO s)
/*! Call SymFromName().
*/
{
  if(!gLoaded)
    load();
  return (gSymFromName(h,n,s));
}

/*  ________________________________________________________________________ */
BOOL SymGetTypeInfo(HANDLE h,DWORD64 m,ULONG t,IMAGEHLP_SYMBOL_TYPE_INFO g,PVOID i)
/*! Call SymGetTypeInfo().
*/
{
  if(!gLoaded)
    load();
  return (gSymGetTypeInfo(h,m,t,g,i));
}

/*  ________________________________________________________________________ */
BOOL StackWalk(DWORD mt,HANDLE p,HANDLE t,LPSTACKFRAME64 sf,PVOID ctx,PREAD_PROCESS_MEMORY_ROUTINE64 rmr,PFUNCTION_TABLE_ACCESS_ROUTINE64 ftar,PGET_MODULE_BASE_ROUTINE64 gmbr,PTRANSLATE_ADDRESS_ROUTINE64 tar)
/*! Call StackWalk().
*/
{
  if(!gLoaded)
    load();
  return (gStackWalk(mt,p,t,sf,ctx,rmr,ftar,gmbr,tar));
}

/*  ________________________________________________________________________ */
PVOID SymFunctionTableAccess(HANDLE h,DWORD64 ab)
/*! Call SymFunctionTableAccess().
*/
{
  if(!gLoaded)
    load();
  return (gSymFunctionTableAccess(h,ab));
}

/*  ________________________________________________________________________ */
DWORD64 SymGetModuleBase(HANDLE h,DWORD64 a)
/*! Call SymGetModuleBase().
*/
{
  if(!gLoaded)
    load();
  return (gSymGetModuleBase(h,a));
}

/*  ________________________________________________________________________ */
BOOL SymGetSymFromAddr(HANDLE h,DWORD64 a,PDWORD64 d,PIMAGEHLP_SYMBOL64 s)
/*! Call SymGetSymFromAddr().
*/
{
  if(!gLoaded)
    load();
  return (gSymGetSymFromAddr(h,a,d,s));
}

/*  ________________________________________________________________________ */
BOOL SymGetLineFromAddr(HANDLE h,DWORD64 a,PDWORD d,PIMAGEHLP_LINE64 l)
/*! Call SymGetLineFromAddr().
*/
{
  if(!gLoaded)
    load();
  return (gSymGetLineFromAddr(h,a,d,l));
}

/*  ________________________________________________________________________ */
DWORD UnDecorateSymbolName(PCSTR d,PSTR u,DWORD l,DWORD f)
/*! Call UnDecorateSymbolName().
*/
{
  if(!gLoaded)
    load();
  return (gUnDecorateSymbolName(d,u,l,f));
}

}  /* namespace dbghelp */