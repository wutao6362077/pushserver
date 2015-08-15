#include "ProviderPacket.h"

ProviderPacket::ProviderPacket()
{
    Reset();
}

ProviderPacket::~ProviderPacket()
{

}

void ProviderPacket::Reset()
{
	m_command = 0xff;
	m_token.clear();
	m_payload.clear();
	m_tokenLength = 0;
	m_payloadLength = 0;
}

uint8_t ProviderPacket::SetCommand(uint8_t cmd)
{
	unsigned char old = m_command;
	m_command = cmd;
	return old;
}

uint8_t ProviderPacket::GetCommand()
{
	return m_command;
}

uint16_t ProviderPacket::SetTokenLength(uint16_t length)
{
	uint8_t old = m_tokenLength;
	m_tokenLength = length;
	return old;
}

uint16_t ProviderPacket::GetTokenLength()
{
	return m_tokenLength;
}

void ProviderPacket::SetToken(const char *buf, uint16_t length)
{
	m_token.assign(buf,length);
}

std::string ProviderPacket::GetToken()
{
	return m_token;
}

uint16_t ProviderPacket::SetPayloadLength(uint16_t length)
{
	uint8_t old = m_payloadLength;
	m_payloadLength = length;
	return old;
}

uint16_t ProviderPacket::GetPayloadLength()
{
	return m_payloadLength;
}

void ProviderPacket::SetPayload(const char *buf, uint16_t length)
{
	m_payload.assign(buf,length);
}

std::string ProviderPacket::GetPayload()
{
	return m_payload;
}

