/*!
	@file	HLog.h
	@author	Scott Smith
	@date	May 07, 2004

	@brief	Event Log.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#ifndef	__HTML_LOGGER_H__
#define	__HTML_LOGGER_H__

#include "Log.h"

class HLog : public LogSingleton
{
public:
	/*!
	 @param void 
	*//*__________________________________________________________________________*/
	virtual void Dump(void) 
	{
		if(mFp)
		{
			fprintf(mFp, "<HTML>");
			fprintf(mFp, "<BODY>");
			
			for(unsigned int i = 0; i < mMessages.size(); ++i)
				fprintf(mFp, mMessages[i].c_str());
			
			fprintf(mFp, "</BODY>");
			fprintf(mFp, "</HTML>");
			
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
	 @param message 
	*//*__________________________________________________________________________*/
	virtual void Post(std::string message) 
	{ 
		std::string t = "<P>" + message;
		mMessages.push_back(t + '\n'); 
		if(mEcho)
			std::cout << message << std::endl;
	}
	

private:
	HLog(const HLog &rhs);
	HLog & operator=(const HLog &rhs);
};
#define	HLogS (HLog::instance()) 
#endif