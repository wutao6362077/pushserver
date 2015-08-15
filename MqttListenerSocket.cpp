#include "MqttListenerSocket.h"
#include "MqttServer.h"
#include "ApsConfigParser.h"
#include "ZQDataconverter.h"

#include "ApsLogger.h"

XMLPrefsParser* MqttListenerSocket::sPrefsSource = NULL;

void MqttListenerSocket::InitializeParam()
{
	sPrefsSource = ApsConfigParser::GetInstance();

	ContainerRef svrPref = sPrefsSource->GetRefForMQTTServer();
	ContainerRef pref = NULL;
    char* thePrefValue = NULL;
	char* thePrefTypeStr = NULL;
    char* thePrefName = NULL;
	ZQ_AttrDataType theType;
	UInt32 convertedBufSize = 0;
	//Get the APNs server ip
	pref = sPrefsSource->GetPrefRefByName( svrPref, "mqtt_server_ip" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strAddr.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}

	//Get the APNs server port
    pref = sPrefsSource->GetPrefRefByName( svrPref, "mqtt_server_port");
	if (pref != NULL)
	{
		if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName, (char**)&thePrefTypeStr))
		{
			theType = ZQDataConverter::TypeStringToType(thePrefTypeStr);
			convertedBufSize = sizeof(UInt16);
			ZQDataConverter::StringToValue(thePrefValue, theType, &m_nPort, &convertedBufSize);
		}
	}
}

OS_Error MqttListenerSocket::Initialize()
{
	InitializeParam();
	TCPListenerSocket::SetServerParam(SocketUtils::ConvertStringToAddr(m_strAddr.GetAsCString()), m_nPort);
	return TCPListenerSocket::Initialize();
}

//acceptor模块调用GetSessionTask函数，创建一个连接会话，处理该socket句柄上的所有任务。
Task*   MqttListenerSocket::GetSessionTask(int osSocket, struct sockaddr_in* addr)
{
	Assert(osSocket != EventContext::kInvalidFileDesc);
 	TCPSocket* theSocket = NULL; 
	
	//new one web accept session
    MqttServer* theTask = new MqttServer();
	ApsLogger::Debug("New MqttServer, socket fd = %d!",osSocket); 
	if(NULL == theTask)
		return NULL;
    theSocket = theTask->GetSocket();  // out socket is not attached to a unix socket yet.

	//set options on the socket
	int sndBufSize = 96L * 1024L;
    theSocket->Set(osSocket, addr);
    theSocket->InitNonBlocking(osSocket);
	theSocket->NoDelay();
	theSocket->KeepAlive();
	theSocket->SetSocketBufSize(sndBufSize);
    theSocket->RequestEvent(EV_RE);

    this->RunNormal();
        
    return theTask;
}
