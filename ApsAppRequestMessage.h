#ifndef __APS_APP_REQUEST_MESSAGE_H__
#define __APS_APP_REQUEST_MESSAGE_H__
#include <stdint.h>
#include <string>
#include "json/json.h"
#include "StrPtrLen.h"
#include "ApsLogger.h"

class ApsAppRequestMessage
{
public:
        ApsAppRequestMessage();
        virtual ~ApsAppRequestMessage();

		const char* GetCommand();

		std::string GetDeviceID();
		
		std::string GetUserID();

		std::string GetAppID();

		bool GetCleanFlag();

		uint32_t GetKeepalive();

		uint16_t GetMessageID();

		Json::Value& GetMessage();

		Json::Value& GetIOSMessage();

		Json::Value& GetAndroidMessage();
		
		bool ParseRequestMessage(std::string &message);
		
private:
		Json::Reader m_reader;
		Json::Value m_root;
		std::string m_command;
		std::string m_appID;
		bool m_cleanFlag;
		uint32_t m_keepalive;
		uint16_t m_mid;
		uint32_t m_qos;
		bool m_remainFlag;
		uint32_t m_timeOnLive;
		std::string m_userID;
		std::string m_deviceID;
		Json::Value m_android;
		Json::Value m_iOS;
};

#endif

