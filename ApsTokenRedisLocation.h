#ifndef __APS_TOKEN_REDIS_LOCATION_H__
#define __APS_TOKEN_REDIS_LOCATION_H__
#include "XMLPrefsParser.h"
#include "StringParser.h"
#include "ApsLocation.h"
#include "OSMutex.h"

class ApsTokenRedisLocation
	:	public ApsLocation
{
public:
    //call this before calling anything else
	static ApsTokenRedisLocation* GetInstance(void);

	virtual ~ApsTokenRedisLocation(void);

    void Initialize();

	static void OndisConnectCallback(const std::string& name);

	virtual bool	ConnectSql(void);

	virtual bool	CheckConnection();
	
	// 查找userid和deviceid对应的deviceToken
	virtual bool IsValidDeviceToken(const char* deviceID, const char* type);

	virtual bool SetValidDeviceToken(const char* deviceID, const char* type);

private:
	ApsTokenRedisLocation(void);
	StrPtrLen		m_strAddr;
	StrPtrLen		m_strUsrName;
	StrPtrLen		m_strPwd;
	StrPtrLen       m_strDbName;
	short			m_nPort;
	static ApsTokenRedisLocation* m_tokenRedisClient;
	static OSMutex        m_Lock;   //lock   
	static XMLPrefsParser*	sPrefsSource;
};

#endif	// __HI_REDIS_LOCATION_H__
