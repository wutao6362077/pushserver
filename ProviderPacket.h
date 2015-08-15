#ifndef _PROVIDER_PACKET_H_
#define _PROVIDER_PACKET_H_
#include <stdint.h>
#include <string.h>
#include <string>

class ProviderPacket
{
public:
	ProviderPacket();
	virtual ~ProviderPacket();
	void Reset();
	uint8_t SetCommand(uint8_t cmd);
	uint8_t GetCommand();
	uint16_t SetTokenLength(uint16_t length);
	uint16_t GetTokenLength();
	void SetToken(const char *buf,uint16_t length);
	std::string GetToken();
	uint16_t SetPayloadLength(uint16_t length);
	uint16_t GetPayloadLength();
	void SetPayload(const char *buf,uint16_t length);
	std::string GetPayload();

private:
	uint8_t m_command;
	uint16_t m_tokenLength;
	std::string m_token;
	uint16_t m_payloadLength;
	std::string m_payload;
};

#endif

