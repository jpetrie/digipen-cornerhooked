/*!
 @file		profiler.h
 @ingroup	profiler
 @date		05-13-2004
 @author	Scott
*//*__________________________________________________________________________*/

/*
Guard
*/
#pragma once

#ifndef __PROFILER_H__
#define __PROFILER_H__

//#define NL_PROFILE

#if defined(NL_PROFILE)

#define VC_EXTRALEAN

/*
Includes
*/
#include <windows.h>
#include <dbghelp.h>
#include <limits>

#pragma warning(push, 3)
#pragma warning(disable: 4702)
#include <map>
#include <string>
#include <iostream>
#include <algorithm>
//#include <queue>
#include <stack>
#pragma warning(pop)
#include "hlog.h"
#include "nsl_singleton.h"

#include "wrapdbghelp.h"

/*!
 @struct	ProfFnData
 @ingroup	profiler
 @date		05-14-2004
 @author	Scott
*//*__________________________________________________________________________*/
struct ProfFnData
{
	ProfFnData(void):mStackDepth(0), mHitCount(0), mBestTime(std::numeric_limits< double >::infinity()), 
		mWorstTime(0.0), mCurrTime(0.0), mTotalTime(0.0), mAveTime(0.0)  
	{
		
	}
	bool operator<(const ProfFnData& rhs)const { return (this->mCurrTime < rhs.mCurrTime); }
	
	int			mStackDepth;	///< How deep into the function call stack are we ?
	int			mHitCount;		///< How many times was the function called during program execution?
    double		mBestTime;		///< The function's best performance time.
	double		mWorstTime;		///< The function's worst performance time.
	double		mCurrTime;		///< The function's performance time for this call.
	double		mTotalTime;		///< The total time spent in the function for the life of the program.
	double		mAveTime;		///< The time spent in the function on a single call, on average.
	std::string mParentFn;		///< The name of the function that called this function.
	std::string mFnName;		///< The name of the function.
	std::map< std::string, ProfFnData >		mChildren;		///< The names of the functions called from this function.
};
typedef float FLOAT;
typedef std::map< std::string, ProfFnData > FnMap;
class DXFont;
/*!
 @class		Profiler
 @ingroup	profiler
 @date		05-14-2004
 @author	Scott
*//*__________________________________________________________________________*/
class Profiler : public nsl::singleton< Profiler >
{
	friend class ProfileAux;
public:
	inline Profiler();
	inline virtual ~Profiler();
	inline void ShowSignature(bool show);
	inline void PushFn(ProfFnData&);
	inline void PopFn(void);
	inline void Profiler::DumpStats(int x, int y, DXFont *font);
private:
	Log mLog;
	::LARGE_INTEGER mStartTime, mEndTime, mFreq;
	std::string mCurrFnName;

	std::stack< ProfFnData >	mCallStack;
	FnMap mRTList;
	FnMap mFnList;

	int mStackDepth;
	double mTotalTime;
	bool mShowSig;

	inline void StartTimer(void);
	inline double EndTimer(void);
	inline std::vector< std::string > Evaluate(const ProfFnData&);
};
class ProfileAux
{
public:
	inline ProfileAux(std::string fn, Profiler* profiler);
	inline ~ProfileAux();
private:
	::LARGE_INTEGER miStartTime, miEndTime, miFreq;
	std::string mFnName;
	Profiler* mProfiler;
	inline void iStartTimer(void);
	inline double iEndTimer(void);
};

#include "Profiler.inl"

#define ProfilerShowSig(Bool)	(Profiler::instance())->ShowSignature(Bool)
#define ProfileFn	volatile ProfileAux Prof##__COUNTER__(__FUNCDNAME__, Profiler::instance())
#define ProfilerS	(Profiler::instance())
#else
class Profiler
{
};
#define ProfileFn
#define ProfilerShowSig(Bool)
#define ProfilerS
#endif

#endif

