#include "ApsApnsPrimeryRequestMessage.h"


uint16_t ApsApnsPrimeryRequestMessage::m_tokenLength = htons(32);
#define DEVICE_BINARY_SIZE 32

//used for change the string of device token to binary token
SInt32 Chr2Hex(char c)
{
	switch(c)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return c - '0';
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
		return c - 'a' + 10;
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
		return c - 'A' + 10;
	default:
		return 0;
	}
	return 0;
}


SInt32 Hex2Chr(char *dstStr, const char *hexStr, int hexLen)
{
	SInt32 desIndex = 0;
	for(SInt32 srcIndex=0;srcIndex<hexLen;)
	{
		if(*(hexStr+srcIndex) < '0'
		||(*(hexStr+srcIndex) > '9' && *(hexStr+srcIndex) < 'A')
		||(*(hexStr+srcIndex) > 'Z' && *(hexStr+srcIndex) < 'a')
		|| *(hexStr+srcIndex) > 'z')
		{
			srcIndex++;
			continue;
		}
		SInt32 ibig   = Chr2Hex(*(hexStr+srcIndex++));
		SInt32 ismall = Chr2Hex(*(hexStr+srcIndex++));
		*(dstStr+desIndex++) = ibig*16 + ismall;
	}
	return desIndex;
}

//RTK连接接收报文类
ApsApnsPrimeryRequestMessage::ApsApnsPrimeryRequestMessage()
{

}

ApsApnsPrimeryRequestMessage::~ApsApnsPrimeryRequestMessage()
{

}

void ApsApnsPrimeryRequestMessage::SetCommand(uint8_t cmd)
{
	m_command = cmd;
}

void ApsApnsPrimeryRequestMessage::SetDeviceToken(const char *deviceToken)
{
	char mDeviceToken[32];
	Hex2Chr(mDeviceToken, deviceToken, strlen(deviceToken));
	m_deviceToken.assign(mDeviceToken,DEVICE_BINARY_SIZE);
}

void ApsApnsPrimeryRequestMessage::SetPayload(const char *payLoad)
{
    char buf[256] = {0};
	strcpy(buf, "{\"aps\":");
    if(strncmp(payLoad,"null",4))
    {
        strcat(buf,payLoad);
    }
	else
	{
		strcat(buf, "{\"alert\":\" AVCON Push Sever \"}");
	}
 
    strcat(buf,"}");

    short payload_len = htons(strlen(buf));
	m_payload.assign(buf,strlen(buf));
}
void ApsApnsPrimeryRequestMessage::SetExpriy(uint32_t expriy)
{
	m_expriy = expriy;
}

void ApsApnsPrimeryRequestMessage::SetId(uint32_t id)
{
	m_id = id;
}

bool ApsApnsPrimeryRequestMessage::GetRequestMessage(char *buf, uint32_t &length)
{
	bool ret = false;
	uint32_t len = 0;
	if(buf == NULL)
	{
		return ret;
	}
	if(m_command == 0)
	{
		*buf = m_command;
		len = 1;
		memcpy(buf+len,&m_tokenLength,2);
		len += 2;
		memcpy(buf+len, m_deviceToken.c_str(), DEVICE_BINARY_SIZE);
		len += DEVICE_BINARY_SIZE;
		uint16_t strLen = htons(m_payload.length());
		memcpy(buf+len,&strLen,2);
		len += 2;
		memcpy(buf+len,m_payload.c_str(),m_payload.length());
		len += m_payload.length();
		length = len;
	}
	else if(m_command == 1)
	{
		*buf = m_command;
		len = 1;
		uint32_t id = htonl(m_id);
		memcpy(buf+len,&id,4);
		len += 4;
		uint32_t expriy = htonl(m_expriy);
		memcpy(buf+len,&expriy,4);
		len += 4;	
		memcpy(buf+len,&m_tokenLength,2);
		len += 2;
		memcpy(buf+len, m_deviceToken.c_str(), DEVICE_BINARY_SIZE);
		len += DEVICE_BINARY_SIZE;
		uint16_t strLen = htons(m_payload.length());
		memcpy(buf+len,&strLen,2);
		len += 2;
		memcpy(buf+len,m_payload.c_str(),m_payload.length());
		len += m_payload.length();
		length = len;
	}
	else
	{
		ApsLogger::Debug("GetRequestMessage command = %d not valid.",m_command);		
	}
	
	return true;
}

