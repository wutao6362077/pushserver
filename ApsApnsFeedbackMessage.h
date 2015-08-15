#ifndef __APS_APNS_FEEDBACK_MESSAGE_H__
#define __APS_APNS_FEEDBACK_MESSAGE_H__
#include <stdint.h>
#include <string>
#include<netinet/in.h>
#include "json/json.h"
#include "StrPtrLen.h"
#include "ApsLogger.h"
#include "ApsApnsPrimeryRequestMessage.h"

class ApsApnsFeedbackMessage
{
public:
        ApsApnsFeedbackMessage();
        virtual ~ApsApnsFeedbackMessage();

		void SetTime(uint32_t time);

		uint32_t GetTime();

		void SetDeviceToken(const char *deviceToken);
		
		std::string GetDeviceToken();
		
private:
		uint32_t m_time;
		std::string m_deviceToken;
		static uint16_t m_tokenLength;
};

#endif
