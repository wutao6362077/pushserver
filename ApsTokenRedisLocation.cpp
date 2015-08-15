#include "ApsTokenRedisLocation.h"
#include "RedisClient.h"
#include "RedisResult.h"
#include "ApsConfigParser.h"
#include "ZQDataconverter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ApsLogger.h"

#define SD_TOKEN_VALID				"Account:%s:valid %s"
#define SD_TOKEN_INVALID			"Account:%s:invalid"


XMLPrefsParser* ApsTokenRedisLocation::sPrefsSource = NULL;
ApsTokenRedisLocation* ApsTokenRedisLocation::m_tokenRedisClient = NULL;
OSMutex        ApsTokenRedisLocation::m_Lock;   //lock

ApsTokenRedisLocation* ApsTokenRedisLocation::GetInstance(void)
{
	if (m_tokenRedisClient == NULL)
    {
		OSMutexLocker locker(&m_Lock);
    	if (m_tokenRedisClient == NULL)
    	{	
			ApsLogger::Info("New ApsTokenRedisLocation!");
       		m_tokenRedisClient = new ApsTokenRedisLocation();
    	}
    }

    return m_tokenRedisClient;
}

void ApsTokenRedisLocation::Initialize()
{
	sPrefsSource = ApsConfigParser::GetInstance();

	ContainerRef svrPref = sPrefsSource->GetRefForTokenRedis();
	ContainerRef pref = NULL;
    char* thePrefValue = NULL;
	char* thePrefTypeStr = NULL;
    char* thePrefName = NULL;
	ZQ_AttrDataType theType;
	UInt32 convertedBufSize = 0;
	//Get the redis server ip
	pref = sPrefsSource->GetPrefRefByName( svrPref, "redis_server_ip" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strAddr.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the redis user name
	pref = sPrefsSource->GetPrefRefByName( svrPref, "redis_usr_name" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strUsrName.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the redis password
	pref = sPrefsSource->GetPrefRefByName( svrPref, "redis_pwd" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strPwd.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the redis db name
	pref = sPrefsSource->GetPrefRefByName( svrPref, "redis_db_name" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strDbName.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the redis server port
    pref = sPrefsSource->GetPrefRefByName( svrPref, "redis_server_port");
	if (pref != NULL)
	{
		if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName, (char**)&thePrefTypeStr))
		{
			theType = ZQDataConverter::TypeStringToType(thePrefTypeStr);
			convertedBufSize = sizeof(UInt16);
			ZQDataConverter::StringToValue(thePrefValue, theType, &m_nPort, &convertedBufSize);
		}
	}

	if(ConnectSql())
	{
		ApsLogger::Info("Connect token redis successful!");
	}
	else
	{
		ApsLogger::Info("Connect token redis false!");
		exit(-1);
	}
}

void ApsTokenRedisLocation::OndisConnectCallback(const std::string& name)
{
	RedisClient::GetInstance().NeedReConnect();
}

ApsTokenRedisLocation::ApsTokenRedisLocation(void)
{
	RedisClient::GetInstance().SetDisConnectCallback(&ApsTokenRedisLocation::OndisConnectCallback);
}

ApsTokenRedisLocation::~ApsTokenRedisLocation(void)
{
   RedisClient::GetInstance().Quit();
}

bool ApsTokenRedisLocation::ConnectSql(void)
{
	RedisClient::GetInstance().SetServerInfo("redis", m_strAddr.GetAsCString(), m_nPort, "");
    return RedisClient::GetInstance().Connect();
}

bool ApsTokenRedisLocation::CheckConnection()
{
	return RedisClient::GetInstance().IsConnected();
}

bool ApsTokenRedisLocation::IsValidDeviceToken(const char* deviceID, const char* type)
{
	bool ret = false;
	if(deviceID == NULL||type == NULL)
	{
		return ret;	
	}
	if(!CheckConnection())
	{
		if(!ConnectSql())//reconnect error so return.
		{
			return ret;
		}
	}
	std::string result;
	std::string redisType = "set";
	std::string field = deviceID; 
	char sdName[256];
	int nLen = sprintf(sdName, SD_TOKEN_INVALID, type,deviceID);
	sdName[nLen] = '\0';
//	ApsLogger::Debug("IsValidDeviceToken: sdName = %s!",sdName);
	if(!RedisClient::GetInstance().GetData(sdName, field,redisType, result))
	{	
		ret = true;
	}
	return ret;
}

bool ApsTokenRedisLocation::SetValidDeviceToken(const char* deviceID, const char* type)
{
	bool ret = false;
	if(deviceID == NULL||type == NULL)
	{
		return ret;	
	}
	if(!CheckConnection())
	{
		if(!ConnectSql())//reconnect error so return.
		{
			return ret;
		}
	}
	RedisResult resultUsers;
	std::string redisType = "set";
	std::string value = deviceID;
	std::string field;
	char sdName[256];
	int nLen = sprintf(sdName, SD_TOKEN_INVALID, type);
	sdName[nLen] = '\0';
//	ApsLogger::Debug("IsValidDeviceToken: sdName = %s!",sdName);
	if(RedisClient::GetInstance().SetData(sdName,value, redisType,0, field))
	{	
		ret = true;
	}
	return ret;
}

