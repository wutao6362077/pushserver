#ifndef __APS_REDIS_LOCATION_H__
#define __APS_REDIS_LOCATION_H__
#include "XMLPrefsParser.h"
#include "StringParser.h"
#include "ApsLocation.h"
#include "OSMutex.h"

class ApsRedisLocation
	:	public ApsLocation
{
public:
    //call this before calling anything else
	static ApsRedisLocation* GetInstance(void);

	virtual ~ApsRedisLocation(void);

    void Initialize();

	static void OndisConnectCallback(const std::string& name);

	virtual bool	ConnectSql(void);

	virtual bool	CheckConnection();
	
	// 查找userid和deviceid对应的deviceToken
	virtual bool GetDeviceToken(const char* userId, const char* device_id, MapDID &map);

	// 查找userid包含的所有deviceToken
	virtual bool GetUserToken(const char* userId, MapDID &map);

private:
	ApsRedisLocation(void);
	StrPtrLen		m_strAddr;
	StrPtrLen		m_strUsrName;
	StrPtrLen		m_strPwd;
	StrPtrLen       m_strDbName;
	short			m_nPort;
	static ApsRedisLocation* m_redisClient;
	static OSMutex        m_Lock;   //lock   
	static XMLPrefsParser*	sPrefsSource;
};

#endif	// __HI_REDIS_LOCATION_H__
