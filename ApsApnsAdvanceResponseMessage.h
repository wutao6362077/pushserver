#ifndef __APS_APNS_ADVANCE_RESPONSE_MESSAGE_H__
#define __APS_APNS_ADVANCE_RESPONSE_MESSAGE_H__
#include <stdint.h>
#include <string>
#include<netinet/in.h>
#include "json/json.h"
#include "StrPtrLen.h"
#include "ApsLogger.h"
class ApsApnsAdvanceResponseMessage
{
public:
        ApsApnsAdvanceResponseMessage();
		
        virtual ~ApsApnsAdvanceResponseMessage();

		void SetCommand(uint8_t cmd);

		void SetStatus(uint8_t status);
		
		uint8_t GetStatus();

		void SetId(uint32_t id);
		
		uint32_t GetId();
		
private:
		uint8_t m_command;
		uint8_t m_status;
		uint32_t m_id;
};

#endif
