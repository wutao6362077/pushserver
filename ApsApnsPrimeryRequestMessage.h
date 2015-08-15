#ifndef __APS_APNS_PRIMERY_MESSAGE_H__
#define __APS_APNS_PRIMERY_MESSAGE_H__
#include <stdint.h>
#include <string>
#include<netinet/in.h>
#include "json/json.h"
#include "StrPtrLen.h"
#include "ApsLogger.h"
SInt32 Chr2Hex(char c);
SInt32 Hex2Chr(char *dstStr, const char *hexStr, int hexLen);

class ApsApnsPrimeryRequestMessage
{
public:
        ApsApnsPrimeryRequestMessage();
        virtual ~ApsApnsPrimeryRequestMessage();

		void SetCommand(uint8_t cmd);

		void SetDeviceToken(const char *deviceToken);

		void SetPayload(const char *payLoad);

		void SetExpriy(uint32_t expriy);

		void SetId(uint32_t id);

		bool GetRequestMessage(char *buf, uint32_t &length);
		
private:
		uint8_t m_command;
		uint32_t m_id;
		uint32_t m_expriy;
		std::string m_deviceToken;
		std::string m_payload;

		static uint16_t m_tokenLength;
};

#endif
