#ifndef __APS_MONGODB_LOCATION_H__
#define __APS_MONGODB_LOCATION_H__
#include "XMLPrefsParser.h"
#include "StringParser.h"
#include "ApsLocation.h"
#include "OSMutex.h"
#include "mongo/client/dbclient.h"

class ApsMongodbLocation
	:	public ApsLocation
{
public:
    //call this before calling anything else
	static ApsMongodbLocation* GetInstance(void);

	virtual ~ApsMongodbLocation(void);

    void Initialize();

	static void OndisConnectCallback(const std::string& name);

	virtual bool	ConnectSql();
	
	virtual bool	CheckConnection();

	void 			Add(const char* collection, mongo::BSONObj &p);

	std::auto_ptr<mongo::DBClientCursor>  	Find(const char* collection, mongo::Query *q);
	
	mongo::BSONObj 			Remove(const char* collection, mongo::BSONObj &p);

	void  			Update(const char* collection,mongo::Query& query,mongo::BSONObj& update);

private:
	ApsMongodbLocation(void);
	StrPtrLen		m_strAddr;
	StrPtrLen		m_strUsrName;
	StrPtrLen		m_strPwd;
	StrPtrLen       m_strDbName;
	short			m_nPort;
	mongo::DBClientConnection m_clientConnection;
	static ApsMongodbLocation* m_mongodbClient;
	static OSMutex        m_Lock;   //lock   
	static XMLPrefsParser*	sPrefsSource;
};

#endif	// __HI_REDIS_LOCATION_H__
