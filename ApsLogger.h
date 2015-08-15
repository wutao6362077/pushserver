#ifndef __APS_LOGGER__H
#define __APS_LOGGER__H
#include <stdarg.h>

#include <log4cxx/logger.h>   
#include <log4cxx/propertyconfigurator.h>   
#include <log4cxx/helpers/exception.h>  

#include "OSMutex.h"

#define LOGGER_MESSAGE __FILE__ __LINE__

class ApsLogger
{

#define msgFormat(buf,msg) \
	do{ \
	va_list ap; \
	va_start(ap,msg); \
	vsnprintf(buf, sizeof(buf), msg, ap); \
	va_end(ap); \
	} while (0)

public:

	static void SetName (std::string appName);

	static void Trace(const char *msg, ...)
	{
		char buf[bufferSize_] = {0};
		msgFormat(buf, msg);
		LOG4CXX_TRACE(GetInstance(),buf);
	}
		
	static void Debug(const char *msg, ...)
	{
		char buf[bufferSize_] = {0};
		msgFormat(buf, msg);
		LOG4CXX_DEBUG(GetInstance(),buf);
	}
		
	static void Info(const char *msg, ...)
	{
		char buf[bufferSize_] = {0};
		msgFormat(buf, msg);
		LOG4CXX_INFO(GetInstance(),buf);
	}
				
/*
	static void Warn(const char *msg, ...)
	{
		char buf[bufferSize_] = {0};
		msgFormat(buf, msg);
		LOG4CXX_WARN(Instance(),buf);
	}
*/
	static void Error(const char *msg, ...)
	{
		char buf[bufferSize_] = {0};
		msgFormat(buf, msg);
		LOG4CXX_ERROR(GetInstance(),buf);
	}
		
	static void Fatal(const char *msg, ...)
	{
		char buf[bufferSize_] = {0};
		msgFormat(buf, msg);
		LOG4CXX_FATAL(GetInstance(),buf);
	}
private:
	static log4cxx::LoggerPtr& GetInstance(void);

	static const unsigned short bufferSize_  = 4096;
	static std::string appName_;
	static OSMutex        m_Lock;   //lock   
	static log4cxx::LoggerPtr m_pushLogger;
};

#endif

