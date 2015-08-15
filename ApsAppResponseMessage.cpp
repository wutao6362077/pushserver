#include "ApsAppResponseMessage.h"

//error message
StrPtrLen ApsAppResponseMessage::sStatusCodeStrings[] =
{
	StrPtrLen("success"),
    StrPtrLen("unacceptable protocol version"),
    StrPtrLen("identifier rejected"),
    StrPtrLen("server unavailable"),
    StrPtrLen("bad user name or password"),
    StrPtrLen("not authorized"),
    StrPtrLen("undefined command"),
    StrPtrLen("parse request message error"),
    StrPtrLen("receive publish while not connected"),
    StrPtrLen("receive pingreq while not connected"),
    StrPtrLen("receive connect while not initiate state"),
    StrPtrLen("receive disconnect while not connected"),
    StrPtrLen("publish with all deviceToken is NULL"),
};
//error code
uint32_t ApsAppResponseMessage::sStatusCodes[] =
{
	0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
};

//RTK连接接收报文类
ApsAppResponseMessage::ApsAppResponseMessage()
{
	m_successFlag = false;
	m_mid = 0;
	m_errorCode = 0; 
}

ApsAppResponseMessage::~ApsAppResponseMessage()
{

}

void ApsAppResponseMessage::SetSuccessFlag(bool flag)
{
	m_successFlag = flag;
}

void ApsAppResponseMessage::SetMessageID(uint16_t mid)
{
	m_mid = mid;
}

void ApsAppResponseMessage::SetCommand(std::string &cmd)
{
	m_command = cmd;
}

void ApsAppResponseMessage::SetErrorCode(uint32_t errorCode)
{
	m_errorCode= errorCode;
}

const char* ApsAppResponseMessage::GetResponseMessage()
{
	Json::Value response;

	const char* message;
	bool ret = false;
	if(m_errorCode == 0)//deal message right
	{
		response["name"] = m_command;
		const char * cmd = m_command.c_str();
		if(!strcmp(cmd,"connack"))
		{
			response["success"] = m_successFlag;
		}
		else if(!strcmp(cmd,"puback"))
		{
			response["success"] = m_successFlag;
			response["msgid"] = m_mid;
		}
	}
	else if(m_errorCode == 7)//parse request json message error
	{
		response["errno"] = m_errorCode;
		response["message"] = sStatusCodeStrings[m_errorCode].GetAsCString();
	}
	else//deal message error
	{
		response["name"] = m_command;
		response["errno"] = m_errorCode;
		response["message"] = sStatusCodeStrings[m_errorCode].GetAsCString();
	}

	return m_jsonWrite.write(response).c_str();
}

