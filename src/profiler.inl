/*!
 @file		profiler.inl
 @ingroup	profiler
 @date		05-15-2004
 @author	Scott
*//*__________________________________________________________________________*/

#include "DXFont.h"

inline std::vector< std::string > Profiler::Evaluate(const ProfFnData& p)
{
	std::vector< std::string > ret;
	
	std::string t;
	for(int i = 1; i < p.mStackDepth; ++i)
			t += "\t";
	std::string s;
	{ // display total time spent in the function
		char buf[255] = {0};
		char name[255] = {0};
		dbghelp::UnDecorateSymbolName(p.mFnName.c_str(), name, 255, UNDNAME_NAME_ONLY);
		sprintf(buf, "%s: %f", name,(mFnList[p.mFnName].mTotalTime / mTotalTime) * 100.0);
		s = t + buf;
		ret.push_back(s);
	}	
	{	// display % of parent function time
		if(p.mParentFn.length() > 0)
		{	
			char buf[255] = {0};
			char name[255] = {0};
			dbghelp::UnDecorateSymbolName(mFnList[p.mParentFn].mFnName.c_str(), name, 255, UNDNAME_NAME_ONLY);
			sprintf(buf, "percent in %s: %f:", name, (p.mTotalTime / mFnList[p.mParentFn].mTotalTime) * 100.0 );
			s = t + buf;
			ret.push_back(s);
		}
	}
	{	// display average time per call
		char buf[255] = {0};
		sprintf(buf, "Average Time: %f", p.mAveTime);
		s = t + buf;
		ret.push_back(s);
	}
	{	// dispaly max call time
		char buf[255] = {0};
		sprintf(buf, "Worst Time:%f", p.mWorstTime);
		s = t + buf;
		ret.push_back(s);
	}
	{	// diaplay min call time
		char buf[255] = {0};
		sprintf(buf, "Best Time: %f", p.mBestTime);
		s = t + buf;
		ret.push_back(s);
	}
	{	// display time called (hit count)
		char buf[255] = {0};
		sprintf(buf, "Hits: %d", mFnList[p.mFnName].mHitCount);
		s = t + buf;
		ret.push_back(s);
	}
	if(!p.mChildren.empty())
	{
		FnMap::const_iterator it = p.mChildren.begin();
		while(it != p.mChildren.end())
		{
			std::vector< std::string > temp = Evaluate(it->second);
			ret.insert(ret.end(), temp.begin(), temp.end());
			
			++it;
		}
	}
	return ret;
}
/*!
 @return 
*//*__________________________________________________________________________*/
inline Profiler::Profiler():mShowSig(false), mStackDepth(0) 
{ 
	mLog.File("profile.txt"); 

	StartTimer(); 
}
/*!
 @return 
*//*__________________________________________________________________________*/
inline Profiler::~Profiler()
{
	mTotalTime = EndTimer();
	
	std::vector< std::string > stlist;
	std::vector< std::string > t;
	if(!mCallStack.empty())
	{
		t = Evaluate(/*mFnList[unwind.top()]*/mCallStack.top());
		//t = Evaluate(mFnList[mCallStack.top().mFnName]);
		stlist.insert(stlist.end(), t.begin(), t.end());
	
		for(unsigned int i = 0; i < stlist.size(); ++i)
			mLog.Post(stlist[i]);
	
		mLog.Dump();
	}
}
/*!
 @param show 
*//*__________________________________________________________________________*/
void Profiler::ShowSignature(bool show)	
{ 
	mShowSig = show; 
}
	
/*!
 @param void 
*//*__________________________________________________________________________*/
inline void Profiler::StartTimer(void) 
{ 
	::QueryPerformanceCounter(&mStartTime); 
}
/*!
 @param void 
 @return 
*//*__________________________________________________________________________*/
inline double Profiler::EndTimer(void)
{
	double ret = std::numeric_limits< double >::infinity();

	::QueryPerformanceCounter(&mEndTime);
	if(::QueryPerformanceFrequency(&mFreq))
	{
		mEndTime.QuadPart -= mStartTime.QuadPart;
		ret = static_cast< double >(mEndTime.QuadPart)/static_cast< double >(mFreq.QuadPart);
	}
	return ret;
}


/*!
 @param fn 
 @param profiler 
 @return 
*//*__________________________________________________________________________*/
inline ProfileAux::ProfileAux(std::string fn, Profiler* profiler):mFnName(fn), mProfiler(profiler)
{
	char bf[255] = {0};
	sprintf(bf, "%d", mProfiler->mStackDepth);
	//mFnName += bf;
	//mProfiler->mFnList[mFnName].mParentFn = mProfiler->mLastFnName;
	mProfiler->mFnList[mFnName].mFnName = mFnName;
	mProfiler->mCurrFnName = mFnName;
	mProfiler->mFnList[mFnName].mHitCount++;
	mProfiler->mRTList[mFnName].mFnName = mFnName;
	mProfiler->mRTList[mFnName].mHitCount++;
	mProfiler->mStackDepth++;
	ProfFnData pdf;
	pdf.mFnName = mFnName;
	pdf.mStackDepth = mProfiler->mStackDepth;
	mProfiler->PushFn(pdf);
	iStartTimer();
}
/*!
 @return 
*//*__________________________________________________________________________*/
inline ProfileAux::~ProfileAux()
{
	ProfFnData pfd = mProfiler->mFnList[mFnName];
	double time_spent = iEndTimer();
	mProfiler->mStackDepth--;
	if(mProfiler->mFnList.count(mFnName) == 0)
	{
		mProfiler->mFnList[mFnName].mTotalTime = 0.0;
	}
	if(mProfiler->mRTList.count(mFnName) == 0)
	{
		mProfiler->mRTList[mFnName].mTotalTime = 0.0;
	}
	mProfiler->mFnList[mFnName].mTotalTime += time_spent;
	mProfiler->mFnList[mFnName].mCurrTime = time_spent;
	if(time_spent > mProfiler->mFnList[mFnName].mWorstTime)
		mProfiler->mFnList[mFnName].mWorstTime = time_spent;
	if(time_spent < mProfiler->mFnList[mFnName].mBestTime)
	mProfiler->mFnList[mFnName].mBestTime = time_spent;
	mProfiler->mFnList[mFnName].mAveTime = mProfiler->mFnList[mFnName].mTotalTime / mProfiler->mFnList[mFnName].mHitCount;

	mProfiler->mRTList[mFnName].mTotalTime += time_spent;
	mProfiler->mRTList[mFnName].mCurrTime = time_spent;
	if(time_spent > mProfiler->mRTList[mFnName].mWorstTime)
		mProfiler->mRTList[mFnName].mWorstTime = time_spent;
	if(time_spent < mProfiler->mRTList[mFnName].mBestTime)
	mProfiler->mRTList[mFnName].mBestTime = time_spent;
	mProfiler->mRTList[mFnName].mAveTime = mProfiler->mRTList[mFnName].mTotalTime / mProfiler->mRTList[mFnName].mHitCount;
	
	mProfiler->mCallStack.top().mAveTime = mProfiler->mFnList[mFnName].mAveTime;
	mProfiler->mCallStack.top().mBestTime = mProfiler->mFnList[mFnName].mBestTime;
	mProfiler->mCallStack.top().mFnName = mProfiler->mFnList[mFnName].mFnName;
	mProfiler->mCallStack.top().mHitCount = mProfiler->mFnList[mFnName].mHitCount;
	mProfiler->mCallStack.top().mTotalTime = mProfiler->mFnList[mFnName].mTotalTime;
	mProfiler->mCallStack.top().mWorstTime = mProfiler->mFnList[mFnName].mWorstTime;
	
	mProfiler->PopFn();
}
/*!
 @param void 
*//*__________________________________________________________________________*/
inline void ProfileAux::iStartTimer(void) { ::QueryPerformanceCounter(&miStartTime); }
/*!
 @param void 
 @return 
*//*__________________________________________________________________________*/
inline double ProfileAux::iEndTimer(void)
{
	double ret = std::numeric_limits< double >::infinity();

	::QueryPerformanceCounter(&miEndTime);
	if(::QueryPerformanceFrequency(&miFreq))
	{
		miEndTime.QuadPart -= miStartTime.QuadPart;
		ret = static_cast< double >(miEndTime.QuadPart)/static_cast< double >(miFreq.QuadPart);
	}
	return ret;
}


/*!
 @param fn 
*//*__________________________________________________________________________*/
inline void Profiler::PushFn(ProfFnData& fn)
{
	if(!mCallStack.empty())
	{
		fn.mParentFn = mCallStack.top().mFnName;
		mFnList[fn.mFnName].mParentFn = mCallStack.top().mFnName;

	}
	//mFnList[mCallStack.top().mFnName].mChildren[fn.mFnName] = fn;
	//mCallStack.top().mChildren[fn.mFnName] = fn;
	mCallStack.push(fn);

}
/*!
 @param void 
*//*__________________________________________________________________________*/
inline void Profiler::PopFn(void)
{
	if(mCallStack.size() > 1)
	{
		ProfFnData p = mCallStack.top();
		mCallStack.pop();
		mCallStack.top().mChildren[p.mFnName] = p;
		//fn.mParentFn = mCallStack.top().mFnName;
		//mCallStack.pop();
		//mCallStack.push(p);
	}
}
static ProfFnData LeafFn(const ProfFnData& pfd)
{
	if(pfd.mChildren.size() == 0)
	{
		return pfd;
	}
	else
	{
		FnMap::const_iterator it = pfd.mChildren.begin();
		while(it != pfd.mChildren.end())
		{
			LeafFn(it->second);
			++it;
		}
	}
}
inline void Profiler::DumpStats(int x, int y, DXFont *font)
{
	/*::LARGE_INTEGER start, end, freq;
	::QueryPerformanceCounter(&start);
	double ret = std::numeric_limits< double >::infinity();*/
	
	ProfFnData worst_time, time;
	worst_time.mAveTime = 0.0;
	// dump stats here
	FnMap::iterator it = mRTList.begin();
	while(it != mRTList.end())
	{
		time = LeafFn(it->second);
        if(time.mAveTime > worst_time.mAveTime)
			worst_time = time;
		++it;
	}
	//std::cout << worst_time.first << std::endl;
	//char name[255] = {0};
	//::UnDecorateSymbolName(worst_time.first.c_str(), name, 255, UNDNAME_NAME_ONLY);
	std::vector< std::string > output = Evaluate(worst_time);
	for(unsigned int i = 0; i < output.size(); ++i)
	{
		font->DrawText(x, y, 256,256,0xffffffff, DT_LEFT, output[i].c_str());
		y += 15;
	}

	/*::QueryPerformanceCounter(&end);
	if(::QueryPerformanceFrequency(&freq))
	{
		end.QuadPart -= start.QuadPart;
		ret = static_cast< double >(end.QuadPart)/static_cast< double >(freq.QuadPart);
	}*/
	
	//mTotalTime -= ret;
	//mFnList[mCurrFnName].mTotalTime -= ret;
	//return output;
}

