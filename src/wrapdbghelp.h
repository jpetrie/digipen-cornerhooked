/*! ========================================================================

      @file    wrapdbghelp.h
      @author  jmp
      @brief   Interface to dbghelp.dll wrapper.
      
    ======================================================================== 
 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _WRAPDBGHELP_H_
#define _WRAPDBGHELP_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include <windows.h>
#include <dbghelp.h>

#include <string>

namespace dbghelp
{


/*                                                                prototypes
---------------------------------------------------------------------------- */

// init and free
void load(void);
void free(void);

void* get(const std::string &f);

// function access
BOOL     SymInitialize(HANDLE h,PSTR p,BOOL i);
BOOL     SymCleanup(HANDLE h);
BOOL     SymEnumSymbols(HANDLE h,ULONG64 b,PCSTR m,PSYM_ENUMERATESYMBOLS_CALLBACK c,PVOID u);
BOOL     SymSetContext(HANDLE h,PIMAGEHLP_STACK_FRAME s,PIMAGEHLP_CONTEXT c);
DWORD    SymSetOptions(DWORD o);
DWORD    SymGetOptions(void);
BOOL     SymFromName(HANDLE h,LPSTR n,PSYMBOL_INFO s);
BOOL     SymGetTypeInfo(HANDLE h,DWORD64 m,ULONG t,IMAGEHLP_SYMBOL_TYPE_INFO g,PVOID i);
BOOL     StackWalk(DWORD mt,HANDLE p,HANDLE t,LPSTACKFRAME64 sf,PVOID ctx,PREAD_PROCESS_MEMORY_ROUTINE64 rmr,PFUNCTION_TABLE_ACCESS_ROUTINE64 ftar,PGET_MODULE_BASE_ROUTINE64 gmbr,PTRANSLATE_ADDRESS_ROUTINE64 tar);
PVOID    SymFunctionTableAccess(HANDLE h,DWORD64 ab);
DWORD64  SymGetModuleBase(HANDLE h,DWORD64 a);
BOOL     SymGetSymFromAddr(HANDLE h,DWORD64 a,PDWORD64 d,PIMAGEHLP_SYMBOL64 s);
BOOL     SymGetLineFromAddr(HANDLE h,DWORD64 a,PDWORD d,PIMAGEHLP_LINE64 l);
DWORD    UnDecorateSymbolName(PCSTR d,PSTR u,DWORD l,DWORD f);


}       /* namespace dbghelp */
#endif  /* _WRAPDBGHELP_H_ */