#ifndef __APS_APP_RESPONSE_MESSAGE_H__
#define __APS_APP_RESPONSE_MESSAGE_H__
#include <stdint.h>
#include <string>
#include "json/json.h"
#include "StrPtrLen.h"
#include "ApsLogger.h"
class ApsAppResponseMessage
{
public:
        ApsAppResponseMessage();
        virtual ~ApsAppResponseMessage();

		void SetSuccessFlag(bool flag);

		void SetMessageID(uint16_t mid);

		void SetErrorCode(uint32_t errorCode);

		void SetCommand(std::string &cmd);

		const char* GetResponseMessage();
		
private:
		std::string m_command;
		bool m_successFlag;
		uint16_t m_mid;
		uint32_t m_errorCode; 
		Json::FastWriter m_jsonWrite;

		static uint32_t   sStatusCodes[];
		static StrPtrLen   sStatusCodeStrings[];
};

#endif
