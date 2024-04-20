/*!
	@file	Log.h
	@author	Scott Smith
	@date	May 05, 2004

	@brief	Event Log.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#ifndef	_EVENTLOG_H_
#define	_EVENTLOG_H_

#include "nsl_tools_string.h"
#include "nsl_singleton.h"
#include <iostream>

/*!
 @class		Log
 @ingroup	Miscellaneous Files
 @date		05-05-2004
 @author	Scott
*//*__________________________________________________________________________*/
class Log
{
public:
	/*!
	 @return 
	*//*__________________________________________________________________________*/
	Log():mFp(stdout), mEcho(true) {}
	/*!
	 @return 
	*//*__________________________________________________________________________*/
	virtual ~Log() { if(mFp && mFp != stdout && mFp != stderr)  fclose(mFp); }
	/*!
	 @param message 
	*//*__________________________________________________________________________*/
	virtual void Post(std::string message) 
	{ 
		mMessages.push_back(message + '\n'); 
		if(mEcho)
			std::cout << message << std::endl;
	}
	/*!
	 @param void 
	*//*__________________________________________________________________________*/
	virtual void Dump(void) 
	{
		if(mFp)
		{
			for(unsigned int i = 0; i < mMessages.size(); ++i)
				fprintf(mFp, mMessages[i].c_str());
		}
		mMessages.clear();
	}
	/*!
	 @param echo 
	*//*__________________________________________________________________________*/
	virtual void Echo(bool echo)	
	{ 
		// will not echo if output is stdout or stderr.
		if(mFp == stdout || mFp == stderr)
			mEcho = true;
		else
			mEcho = echo; 
	}
	/*!
	 @param void 
	 @return 
	*//*__________________________________________________________________________*/
	virtual bool Echo(void)			{ return mEcho; }
	/*!
	 @param *fp 
	*//*__________________________________________________________________________*/
	virtual void File(FILE *fp = stdout)
	{
		if(mFp && mFp != stdout && mFp != stderr)
			fclose(mFp);
		mFp = fp;
	}
	/*!
	 @param filename 
	*//*__________________________________________________________________________*/
	virtual void File(std::string filename)
	{
		if(mFp && mFp != stdout && mFp != stderr)
			fclose(mFp);
		mFp = fopen(filename.c_str(), "w");
	}

protected:
	nsl::stringlist	mMessages;
	bool mEcho;
	FILE*	mFp;
};

/*!
 @class		LogSingleton
 @ingroup	Miscellaneous Files
 @date		05-05-2004
 @author	Scott
*//*__________________________________________________________________________*/
class LogSingleton : public Log, public nsl::singleton< LogSingleton >
{
};

#define	LogS (LogSingleton::instance())

#endif