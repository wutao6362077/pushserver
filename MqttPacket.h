#ifndef _MQTT_PACKET_H_
#define _MQTT_PACKET_H_
#include <stdint.h>
#include <string.h>
#include <string>

/* Error values */
enum EMqttErrorCode {
	MQTT_ERR_CONN_PENDING = -1,
	MQTT_ERR_SUCCESS = 0,
	MQTT_ERR_NOMEM = 1,
	MQTT_ERR_PROTOCOL = 2,
	MQTT_ERR_INVAL = 3,
	MQTT_ERR_NO_CONN = 4,
	MQTT_ERR_CONN_REFUSED = 5,
	MQTT_ERR_NOT_FOUND = 6,
	MQTT_ERR_CONN_LOST = 7,
	MQTT_ERR_TLS = 8,
	MQTT_ERR_PAYLOAD_SIZE = 9,
	MQTT_ERR_NOT_SUPPORTED = 10,
	MQTT_ERR_AUTH = 11,
	MQTT_ERR_ACL_DENIED = 12,
	MQTT_ERR_UNKNOWN = 13,
	MQTT_ERR_ERRNO = 14,
	MQTT_ERR_EAI = 15
};

/* Message types */
#define CONNECT 0x10
#define CONNACK 0x20
#define PUBLISH 0x30
#define PUBACK 0x40
#define PUBREC 0x50
#define PUBREL 0x60
#define PUBCOMP 0x70
#define SUBSCRIBE 0x80
#define SUBACK 0x90
#define UNSUBSCRIBE 0xA0
#define UNSUBACK 0xB0
#define PINGREQ 0xC0
#define PINGRESP 0xD0
#define DISCONNECT 0xE0

enum EMqttServerStatus {
	MQTT_INIT_NEW = 0,
	MQTT_CONNECTED = 1,
	MQTT_DISCONNECTING = 2,
};

class MqttPacket
{
public:
	MqttPacket();
	virtual ~MqttPacket();
	void Reset();
	uint8_t SetCommand(uint8_t cmd);
	uint8_t GetCommand();
	//unsigned char SetHaveRemaining(unsigned char remain);
	//unsigned char SetRemainingCount(unsigned char count);
	uint8_t SetRemainingLength(uint8_t length);
	uint8_t GetRemainingLength();
	void SetReceivePayload(const char *buf,uint16_t length);
	
	//void SetSendPayload(const char *buf,unsigned short int length);
	std::string GetPayload();
	std::string GetJsonMsg();
	//int GetTelegram(char *buf,unsigned short int length);
	uint16_t SetMid(uint16_t mid);
	uint16_t GetMid();
	enum EMqttErrorCode ReadString(std::string &str);
	enum EMqttErrorCode ReadUint16(uint16_t &word);
	enum EMqttErrorCode ReadUint8(uint8_t &num);
private:
	uint8_t m_command;
	uint8_t m_haveRemaining;
	uint8_t m_remainingCount;
	uint16_t m_mid;
	uint32_t  m_remainingLength;
	uint32_t  m_packetLength;
	uint32_t  m_toProcess;
	uint32_t  m_pos;
	char *m_payload;
};

#endif

