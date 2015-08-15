#include "MqttPacket.h"

MqttPacket::MqttPacket()
{
	m_payload = NULL;
    Reset();
}

MqttPacket::~MqttPacket()
{
	/* Free data */
	if(m_payload != NULL)
	{
		delete[] m_payload;
	}
}

void MqttPacket::Reset()
{
	/* Free data and reset values */
	m_command = 0;
	m_haveRemaining = 0;
	m_remainingCount = 0;
	m_remainingLength = 0;
	if(m_payload != NULL)
	{
		delete[] m_payload;
		m_payload = NULL;
	}
	m_payload = NULL;
	m_toProcess = 0;
	m_pos = 0;
}

uint8_t MqttPacket::SetCommand(uint8_t cmd)
{
	unsigned char old = m_command;
	m_command = cmd;
	return old;
}

uint8_t MqttPacket::GetCommand()
{
	return m_command;
}
/*
unsigned char MqttPacket::SetHaveRemaining(unsigned char remain)
{
	unsigned char old = m_haveRemaining;
	m_haveRemaining = remain;
	return old;
}

unsigned char MqttPacket::SetRemainingCount(unsigned char count)
{
	unsigned char old = m_remainingCount;
	m_remainingCount = count;
	return old;
}
*/
uint8_t MqttPacket::SetRemainingLength(unsigned char length)
{
	uint8_t old = m_remainingLength;
	m_remainingLength = length;
	return old;
}

uint8_t MqttPacket::GetRemainingLength()
{
	return m_remainingLength;
}

void MqttPacket::SetReceivePayload(const char *buf, uint16_t length)
{
	if(m_payload)
	{
		delete[] m_payload;
	}
	m_payload = new char[length];
	memcpy(m_payload,buf,length);
	m_remainingLength = length;
}
/*
void MqttPacket::SetSendPayload(const char *buf,unsigned short int length)
{
	if(m_payload)
	{
		delete[] m_payload;
	}
	m_payload = new char[length];
	memcpy(m_payload,buf,length);
	m_packetLength = length;
}
*/
std::string MqttPacket::GetPayload()
{
	std::string msg;
	//The first 2 byte is mid
	msg.assign(m_payload+2,m_remainingLength-2);
	return msg;
}

std::string MqttPacket::GetJsonMsg()
{
	std::string msg;
	msg.assign(&m_payload[m_pos],m_remainingLength-m_pos);
	return msg;
}
/*
int MqttPacket::GetTelegram(char *buf,unsigned short int length)
{
	int pos = 0;
	buf[pos++] = m_command;
	buf[pos++] = m_remainingLength;
	buf[pos++] = m_mid>>8;
	buf[pos++] = m_mid&0xff;
	memcpy((void *)buf[pos],(const void *)m_payload,m_packetLength);
	return pos+m_packetLength;
}
*/
uint16_t MqttPacket::SetMid(uint16_t mid)
{
	unsigned short int old = m_mid;
	m_mid = mid;
	return old;
}

uint16_t MqttPacket::GetMid()
{
	return m_mid;
}

enum EMqttErrorCode MqttPacket::ReadString(std::string &str)
{
	uint16_t len;
	enum EMqttErrorCode rc;

	rc = ReadUint16(len);
	if(rc != MQTT_ERR_SUCCESS)
	{
		return rc;
	}
	if(m_pos+len > m_remainingLength)
	{
		return MQTT_ERR_PROTOCOL;
	}
	
	str.assign(&m_payload[m_pos],len);
	m_pos += len;

	return MQTT_ERR_SUCCESS;
}

enum EMqttErrorCode MqttPacket::ReadUint16(uint16_t &word)
{
	uint8_t msb, lsb;

	if(m_pos+2 > m_remainingLength)
	{
		return MQTT_ERR_PROTOCOL;
	}
	
	msb = m_payload[m_pos++];
	lsb = m_payload[m_pos++];

	word = (msb<<8) + lsb;

	return MQTT_ERR_SUCCESS;
}

enum EMqttErrorCode MqttPacket::ReadUint8(uint8_t &num)
{
	if(m_pos+1 > m_remainingLength)
	{
		return MQTT_ERR_PROTOCOL;
	}
	
	num = m_payload[m_pos++];
	return MQTT_ERR_SUCCESS;
}

