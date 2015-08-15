#ifndef _MQTT_SERVER_H_
#define _MQTT_SERVER_H_

#include "MqttPacket.h"
#include "TCPSessionInterface.h"
#include "LinkedList.h"
#include "json/json.h"

//MQTT·þÎñ¶ËÀà
class MqttServer : public TCPSessionInterface
{
public:
	//call this before calling anything else
	MqttServer();

	virtual ~MqttServer();

	SInt64 Run();

	int mqtt_packet_handle(class MqttPacket &packet);

	bool IsLiveSession()		{ return fSocket.IsConnected(); }

	ZQ_Error ProcessRecvMsg(char* pBuffer, int& nLen);

	int mqtt_handle_connect(class MqttPacket &packet);

	int mqtt_handle_disconnect(class MqttPacket &packet);

	int mqtt_handle_pingreq(class MqttPacket &packet);

	int mqtt_handle_puback(class MqttPacket &packet);

	int mqtt_send_connack(int result);

    int mqtt_send_pingresp();

	int mqtt_send_publish(uint16_t mid,uint32_t payloadlen, const void *payload, int qos, bool retain, bool dup);

	int mqtt_send_command(uint8_t command);

	void mqtt_server_disconnect();
	
	void CleanupRequest();

private:
	
	enum EMqttServerStatus connectStatus;
	std::string protocolName;
	uint8_t protocolVersion;
	short unsigned int  keepAlive;
	uint8_t flag;
	std::string clientId;
	List				fMsgList;
	OSMutex				fMutexList;

	Json::Reader reader;
	Json::Value root;
};
#endif

