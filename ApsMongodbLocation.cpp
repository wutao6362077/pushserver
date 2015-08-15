#include "ApsMongodbLocation.h"
#include "ApsConfigParser.h"
#include "ZQDataconverter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ApsLogger.h"

#define SD_PUSH_USER_MSG			"Account:%s"

XMLPrefsParser* ApsMongodbLocation::sPrefsSource = NULL;
ApsMongodbLocation* ApsMongodbLocation::m_mongodbClient = NULL;
OSMutex        ApsMongodbLocation::m_Lock;   //lock

ApsMongodbLocation* ApsMongodbLocation::GetInstance(void)
{
	if (m_mongodbClient == NULL)
    {
		OSMutexLocker locker(&m_Lock);
    	if (m_mongodbClient == NULL)
    	{	
			ApsLogger::Info("New ApsMongodbLocation!");
       		m_mongodbClient = new ApsMongodbLocation();
    	}
    }

    return m_mongodbClient;
}

void ApsMongodbLocation::Initialize()
{
	sPrefsSource = ApsConfigParser::GetInstance();

	ContainerRef svrPref = sPrefsSource->GetRefForMongodb();
	ContainerRef pref = NULL;
    char* thePrefValue = NULL;
	char* thePrefTypeStr = NULL;
    char* thePrefName = NULL;
	ZQ_AttrDataType theType;
	UInt32 convertedBufSize = 0;
	//Get the redis server ip
	pref = sPrefsSource->GetPrefRefByName( svrPref, "mongodb_server_ip" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strAddr.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the redis user name
	pref = sPrefsSource->GetPrefRefByName( svrPref, "mongodb_usr_name" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strUsrName.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the redis password
	pref = sPrefsSource->GetPrefRefByName( svrPref, "mongodb_pwd" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strPwd.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the redis db name
	pref = sPrefsSource->GetPrefRefByName( svrPref, "mongodb_db_name" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strDbName.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the redis server port
    pref = sPrefsSource->GetPrefRefByName( svrPref, "mongodb_server_port");
	if (pref != NULL)
	{
		if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName, (char**)&thePrefTypeStr))
		{
			theType = ZQDataConverter::TypeStringToType(thePrefTypeStr);
			convertedBufSize = sizeof(UInt16);
			ZQDataConverter::StringToValue(thePrefValue, theType, &m_nPort, &convertedBufSize);
		}
	}

/*	if(ConnectSql())
	{
		ApsLogger::Info("Connect mongodb successful!");
	}
	else
	{
		ApsLogger::Info("Connect mongodb false!");
		exit(-1);
	}*/
}

void ApsMongodbLocation::OndisConnectCallback(const std::string& name)
{
	//RedisClient::GetInstance().NeedReConnect();
}

ApsMongodbLocation::ApsMongodbLocation(void)
{

}

ApsMongodbLocation::~ApsMongodbLocation(void)
{
   //RedisClient::GetInstance().Quit();
}

bool ApsMongodbLocation::ConnectSql()
{	
	Initialize();
	char server[256] = {0};
	snprintf(server,sizeof(server),"%s:%d",m_strAddr.GetAsCString(),m_nPort);
	m_clientConnection.connect(server);//connect right or quit process
//	mongo::BSONObj p = BSON( "name" << "Joe" << "age" << 33 );  
//    m_clientConnection.insert("tutorial.persons", p); 
	return true;
}

bool ApsMongodbLocation::CheckConnection()
{
	//return RedisClient::GetInstance().IsConnected();
}

void ApsMongodbLocation::Add(const char* collection, mongo::BSONObj &p)
{
	m_clientConnection.insert(collection, p);
}

std::auto_ptr<mongo::DBClientCursor> ApsMongodbLocation::Find(const char* collection, mongo::Query *q)
{
	if(q == NULL)
		return m_clientConnection.query(collection);
	else
		return m_clientConnection.query(collection,*q);
}

mongo::BSONObj ApsMongodbLocation::Remove(const char* collection, mongo::BSONObj &p)
{
	return m_clientConnection.findAndRemove(collection,p);
}

void ApsMongodbLocation::Update(const char* collection,mongo::Query & query,mongo::BSONObj& update)
{
	m_clientConnection.update(collection,query,update);
}

