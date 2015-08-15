#include "ApsApnsFeedbackMessage.h"

uint16_t ApsApnsFeedbackMessage::m_tokenLength = 32;

//RTK连接接收报文类
ApsApnsFeedbackMessage::ApsApnsFeedbackMessage()
{

}

ApsApnsFeedbackMessage::~ApsApnsFeedbackMessage()
{

}

void ApsApnsFeedbackMessage::SetTime(uint32_t time)
{
	m_time= time;
}

uint32_t ApsApnsFeedbackMessage::GetTime()
{
	return m_time;
}

void ApsApnsFeedbackMessage::SetDeviceToken(const char *deviceToken)
{
	char mDeviceToken[32];
	Hex2Chr(mDeviceToken, deviceToken, strlen(deviceToken));
	m_deviceToken.assign(mDeviceToken);
}

std::string ApsApnsFeedbackMessage::GetDeviceToken()
{
	return m_deviceToken;
}

