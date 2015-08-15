#include "ApsAppRequestMessage.h"
#include "ApsMongodbLocation.h"
#include "mongo/db/json.h"
#include <time.h> 

//RTK连接接收报文类
ApsAppRequestMessage::ApsAppRequestMessage()
{

}

ApsAppRequestMessage::~ApsAppRequestMessage()
{

}

const char* ApsAppRequestMessage::GetCommand()
{
	return m_command.c_str();
}

std::string ApsAppRequestMessage::GetDeviceID()
{
	return m_deviceID;
}

std::string ApsAppRequestMessage::GetUserID()
{
	return m_userID;
}

std::string ApsAppRequestMessage::GetAppID()
{
	return m_appID;
}

bool ApsAppRequestMessage::GetCleanFlag()
{
	return m_cleanFlag;
}

uint32_t ApsAppRequestMessage::GetKeepalive()
{
	return m_keepalive;
}

uint16_t ApsAppRequestMessage::GetMessageID()
{
	return m_mid;
}
Json::Value& ApsAppRequestMessage::GetMessage()
{
	return m_root;
}
Json::Value& ApsAppRequestMessage::GetIOSMessage()
{
	return m_iOS;
}

Json::Value& ApsAppRequestMessage::GetAndroidMessage()
{
	return m_android;
}

//RTK连接接收报文解析
bool ApsAppRequestMessage::ParseRequestMessage(std::string &message)
{
	//
	bool ret = m_reader.parse(message, m_root);
	if (!ret)//The message is not json
	{
		ApsLogger::Debug("Failed to parse request message!"); 
	}
	else//pares json message ok
	{
		//

		m_command = m_root["name"].asString();
		const char * cmd = m_command.c_str();
		if(!strcmp(cmd,"connect"))
		{
			m_appID= m_root["appid"].asString();
			m_cleanFlag= m_root["cleansession"].asBool();
			m_keepalive = m_root["keepalive"].asInt();
		}
		else if(!strcmp(cmd,"publish"))
		{
			m_mid= m_root["msgid"].asInt();
			m_qos= m_root["Qos"].asInt();
			m_remainFlag= m_root["remain"].asBool();
			Json::Value options = m_root["options"];
			m_timeOnLive= options["time_to_live"].asInt();
		
			Json::Value audience = m_root["audience"];
			Json::Value::Members members(audience.getMemberNames());
        	//std::sort( members.begin(), members.end() );
        	for (Json::Value::Members::iterator it = members.begin();it != members.end(); ++it)
        	{
        		if(strcmp(it->c_str(), "usrid")==0)
        		{
					m_userID= audience["usrid"].asCString();
				}
				else if(strcmp(it->c_str(), "devid")==0)
				{
					m_deviceID= audience["devid"].asCString();
				}
       		}

			Json::Value alert;
			bool alert_flag = false;
			bool ios_flag = false;
			bool android_flag = false;
			Json::Value notification = m_root["notification"];
			members = notification.getMemberNames();
        	//std::sort( members.begin(), members.end() );
        	for ( Json::Value::Members::iterator it = members.begin();it != members.end(); ++it )
        	{
        		if(strcmp(it->c_str(), "alert")==0)
        		{
					alert = notification["alert"];
					alert_flag = true;
				}
				else if(strcmp(it->c_str(), "ios")==0 )
				{
					ios_flag = true;
					m_iOS = notification["ios"];
					bool ios_flag = false;
					Json::Value::Members ios_members = m_iOS.getMemberNames();
        			//std::sort( members.begin(), members.end() );
        			for (Json::Value::Members::iterator it_ios = ios_members.begin();it_ios != ios_members.end(); ++it_ios )
        			{
        				if(strcmp(it_ios->c_str(), "alert")==0 )
        				{
							ios_flag = true;
							break;
						}
        			}
					if(!ios_flag)
					{
						if(alert_flag)
						{
							m_iOS["alert"]=alert;
						}
					}
				}
				else if(strcmp(it->c_str(), "android")==0)
				{
					android_flag = true;
					m_android= notification["android"];
					bool android_flag = false;
					Json::Value::Members android_members = m_android.getMemberNames();
        			for ( Json::Value::Members::iterator it_android = android_members.begin();it_android != android_members.end(); ++it_android )
        			{
        				if( strcmp(it_android->c_str(), "alert")==0 )
        				{
							android_flag = true;
							break;
						}
        			}
					if(!android_flag)
					{
						if(alert_flag)
						{
							m_android["alert"]=alert;
						}
					}
				}
        	}
			if(alert_flag)
			{
				if(!android_flag)
				{
					m_android["alert"]=alert;
				}
				if(!ios_flag)
				{
					m_iOS["alert"]=alert;
				}
			}
		} 
	}
	return ret;
}
