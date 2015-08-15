#include "ApsApnsAdvanceResponseMessage.h"

//RTK连接接收报文类
ApsApnsAdvanceResponseMessage::ApsApnsAdvanceResponseMessage()
{

}

ApsApnsAdvanceResponseMessage::~ApsApnsAdvanceResponseMessage()
{

}

void ApsApnsAdvanceResponseMessage::SetCommand(uint8_t cmd)
{
	m_command = cmd;
}

void ApsApnsAdvanceResponseMessage::SetStatus(uint8_t status)
{
	m_status = status;
}

uint8_t ApsApnsAdvanceResponseMessage::GetStatus()
{
	return m_status;
}

void ApsApnsAdvanceResponseMessage::SetId(uint32_t id)
{
    m_id = id;
}

uint32_t ApsApnsAdvanceResponseMessage::GetId()
{
    return m_id;
}

