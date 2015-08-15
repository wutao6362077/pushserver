#include "MqttSessionHandler.h"
#include "ApsLogger.h"

MqttSessionHandler* MqttSessionHandler::m_mqttSessionHandler = NULL;
OSMutex        MqttSessionHandler::m_Lock;   //lock

MqttSessionHandler* MqttSessionHandler::GetInstance(void)
{
	if (m_mqttSessionHandler == NULL)
    {
		OSMutexLocker locker(&m_Lock);
    	if (m_mqttSessionHandler == NULL)
    	{	
			ApsLogger::Info("New MqttSessionHandler!");
       		m_mqttSessionHandler = new MqttSessionHandler();
    	}
    }

    return m_mqttSessionHandler;
}

MqttSessionHandler::MqttSessionHandler()
{
	ApsLogger::Debug("MqttSessionHandler");	
}

MqttSessionHandler::~MqttSessionHandler()
{
	ApsLogger::Debug("~MqttSessionHandler");  
}

bool MqttSessionHandler::AddMqttSession(std::string &uuid,MqttServer * session)
{
	std::pair<MapMqttSessionIter, bool> Insert_Pair;
	
	Insert_Pair = m_mapMqttSession.insert(std::pair<std::string, MqttServer *>(uuid,session));
	if(Insert_Pair.second == true)
	{
		ApsLogger::Debug("AddMqttSession token = %s, session addr = %p successful!",uuid.c_str(),session); 	
	}
	else
	{
		ApsLogger::Debug("AddMqttSession token = %s, session addr = %p error!",uuid.c_str(),session);	
	}
	return Insert_Pair.second;
}

uint32_t  MqttSessionHandler::DeleteMqttSession(std::string &uuid)
{
	uint32_t num;
	num = m_mapMqttSession.erase(uuid);
	if(num == 0)
	{
		ApsLogger::Debug("DeleteMqttSession token = %s false ret = %d!",uuid.c_str(),num); 	
	}
	
	return num;
}

MqttServer * MqttSessionHandler::FindMqttSession(std::string &uuid)
{
	MqttServer * ret = NULL;
	m_mapMqttSessionIter = m_mapMqttSession.find(uuid);
	if(m_mapMqttSessionIter == m_mapMqttSession.end())
	{
		ApsLogger::Debug("FindMqttSession token = %s false!",uuid.c_str()); 	
	}
	else
	{
		ApsLogger::Debug("FindMqttSession token = %s session addr = %p true!",uuid.c_str(),m_mapMqttSessionIter->second); 	
		ret = m_mapMqttSessionIter->second;
	}
	return ret;
}


