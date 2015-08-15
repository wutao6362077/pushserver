#ifndef _MQTT_SESSION_HANDLE_H_
#define _MQTT_SESSION_HANDLE_H_

#include <map>
#include <string>
#include <vector>
#include "MqttPacket.h"
#include "MqttServer.h"

typedef std::map<std::string,MqttServer*>	MapMqttSession;
typedef std::map<std::string,MqttServer*>::iterator	MapMqttSessionIter;

//MQTT会话管理
class MqttSessionHandler
{
public:
	//call this before calling anything else
	static MqttSessionHandler* GetInstance();

	virtual ~MqttSessionHandler();

	bool AddMqttSession(std::string &uuid,MqttServer * session);

	uint32_t  DeleteMqttSession(std::string &uuid);

	MqttServer * FindMqttSession(std::string &uuid);

	
private:
	
	MqttSessionHandler();
	static MqttSessionHandler* m_mqttSessionHandler;
	MapMqttSession m_mapMqttSession;
	MapMqttSessionIter m_mapMqttSessionIter;
	static OSMutex        m_Lock;   //lock  
};
#endif

