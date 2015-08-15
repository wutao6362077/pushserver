#include "ApsRedisLocation.h"
#include "RedisClient.h"
#include "RedisResult.h"
#include "ApsConfigParser.h"
#include "ZQDataconverter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ApsLogger.h"

#define SD_PUSH_USER_MSG			"Account:%s"

XMLPrefsParser* ApsRedisLocation::sPrefsSource = NULL;
ApsRedisLocation* ApsRedisLocation::m_redisClient = NULL;
OSMutex        ApsRedisLocation::m_Lock;   //lock

ApsRedisLocation* ApsRedisLocation::GetInstance(void)
{
	if (m_redisClient == NULL)
    {
		OSMutexLocker locker(&m_Lock);
    	if (m_redisClient == NULL)
    	{	
			ApsLogger::Info("New ApsRedisLocation!");
       		m_redisClient = new ApsRedisLocation();
    	}
    }

    return m_redisClient;
}

void ApsRedisLocation::Initialize()
{
	sPrefsSource = ApsConfigParser::GetInstance();

	ContainerRef svrPref = sPrefsSource->GetRefForRedis();
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
		ApsLogger::Info("Connect redis successful!");
	}
	else
	{
		ApsLogger::Info("Connect redis false!");
		exit(-1);
	}
}

void ApsRedisLocation::OndisConnectCallback(const std::string& name)
{
	RedisClient::GetInstance().NeedReConnect();
}

ApsRedisLocation::ApsRedisLocation(void)
{
	RedisClient::GetInstance().SetDisConnectCallback(&ApsRedisLocation::OndisConnectCallback);
}

ApsRedisLocation::~ApsRedisLocation(void)
{
   RedisClient::GetInstance().Quit();
}

bool ApsRedisLocation::ConnectSql(void)
{
	RedisClient::GetInstance().SetServerInfo("redis", m_strAddr.GetAsCString(), m_nPort, "");
    return RedisClient::GetInstance().Connect();
}

bool ApsRedisLocation::CheckConnection()
{
	return RedisClient::GetInstance().IsConnected();
}

bool ApsRedisLocation::GetDeviceToken(const char* userId, const char* device_id, MapDID &map)
{
	bool ret = false;
	if(userId == NULL || device_id == NULL)
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
	std::string strDeviceId(device_id);
	std::string message;
	char sdName[256];
	int nLen = sprintf(sdName, SD_PUSH_USER_MSG, userId);
	sdName[nLen] = '\0';
	DID did;
	ApsLogger::Debug("GetDeviceToken: sdName = %s, device_id = %s!",sdName, device_id);
	if(RedisClient::GetInstance().GetData(sdName, strDeviceId,RDS_HASH, message))
	{	
		did.strDType.assign(message,message.find('&')+1,message.length());
		std::string devToken;
		devToken.assign(message,0,message.find('&'));
		map[devToken] = did;
		ret = true;
    }
	return ret;
}

bool ApsRedisLocation::GetUserToken(const char* userId, MapDID &map)
{
	bool ret = false;
	if(userId == NULL)
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
	std::string type;
	char sdName[256];
	int nLen = sprintf(sdName, SD_PUSH_USER_MSG, userId);
	sdName[nLen] = '\0';
	DID did;
	ApsLogger::Debug("GetUserToken: sdName = %s!",sdName);
	if(RedisClient::GetInstance().GetData(sdName, type, resultUsers))
	{	
		for(int i = 0; i < resultUsers.RowSize(); i++)
		{
			std::string message = resultUsers.Value(i, 1);
			did.strDType.assign(message,message.find('&')+1,message.length());
			std::string devToken;
			devToken.assign(message,0,message.find('&'));
			map[devToken] = did;
			ret = true;
		}
    }
	return ret;
}
