#include "ApsLogger.h"

std::string ApsLogger::appName_ = std::string("/home/running/log4cxx.properties");
OSMutex		  ApsLogger::m_Lock;	//lock	 
log4cxx::LoggerPtr ApsLogger::m_pushLogger;

void ApsLogger::SetName (std::string appName)
{
	appName_ = appName;
}

log4cxx::LoggerPtr& ApsLogger::GetInstance(void)
{
	if (m_pushLogger == NULL)
	{
		OSMutexLocker locker(&m_Lock);
		if (m_pushLogger == NULL)
		{
			m_pushLogger = log4cxx::Logger::getRootLogger();
			log4cxx::PropertyConfigurator::configure(appName_);
		}
	}

	return m_pushLogger;
}

