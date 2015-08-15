#include "PushC2DMClientSocket.h"
#include "ApsLogger.h"

PushC2DMClientSocket* PushC2DMClientSocket::m_APNsSocket = NULL;
OSMutex        PushC2DMClientSocket::m_Lock;   //lock

//具体的业务处理类，需要根据接收的话题等信息，查找redis中的deviceToken，然后发送给SslTcpSocket
PushC2DMClientSocket* PushC2DMClientSocket::GetInstance(void)
{
	if (m_APNsSocket == NULL)
    {
		OSMutexLocker locker(&m_Lock);
    	if (m_APNsSocket == NULL)
    	{ 	
			ApsLogger::Debug("New PushClientSocket!"); 
       		m_APNsSocket = new PushC2DMClientSocket();
    	}
    }

    return m_APNsSocket;
}

void PushC2DMClientSocket::Initialize()
{
	ApsLogger::Debug("New SslC2DMTCPSocket!"); 
	m_sslSocket = new AndroidSslTCPSocket();
}

PushC2DMClientSocket::PushC2DMClientSocket() 
{
    m_tokenLen = htons(32);
    memset((void*)&m_data,0,sizeof(m_data));
}

PushC2DMClientSocket::~PushC2DMClientSocket()
{
	if(m_sslSocket != NULL)
	{
		delete m_sslSocket;
	}
}

void PushC2DMClientSocket::PushNotification(const char *pToken,const char *pMsg)
{
	static bool init_flag = false;
	if(!init_flag)
	{
		Initialize();
		init_flag = true;
	}

	int paylen = 0;
	bool flag = false;
	OSMutexLocker locker(&m_Lock);
	//product message body
    paylen = GenPayloadData(1,pMsg);
	//product device token
    GenPushData(pToken);
	//send the message
	flag = m_sslSocket->Send(&m_data, 35 + paylen);
	if(flag)
	{	
		ApsLogger::Debug("Send error restart AndroidSslTCPSocket!"); 
		//delete m_sslSocket;
		m_sslSocket->Signal(Task::kKillEvent); // just clean up the task
		m_sslSocket = NULL;
		m_sslSocket = new AndroidSslTCPSocket();
		m_sslSocket->Send(&m_data, 35 + paylen);
	}
	else
	{
		ApsLogger::Debug("Send successful AndroidSslTCPSocket!"); 
	}
}

void PushC2DMClientSocket::GenPushData(const char *pToken)
{
    char *ptr = m_data.szToken;
    *ptr++ = 0;
    memcpy(ptr,&m_tokenLen,2);
    ptr += 2;
	::memcpy(ptr, pToken, ANDROID_DEVICE_BINARY_SIZE);
}

int PushC2DMClientSocket::GenPayloadData(int badgeNum,const char *pMsg)
{
    char buf[256] = {0};
    char badgeBuf[3] = {0};
	strcpy(&m_data.szPayload[2], "{\"aps\":");
    if(pMsg != NULL)
    {
        strcat(&m_data.szPayload[2],pMsg);
    }
	else
	{
		strcat(&m_data.szPayload[2], "{\"alert\":\"Hello World!\"}");
	}
 
    strcat(&m_data.szPayload[2],"}");

    int len = strlen(&m_data.szPayload[2]);
    assert(len <= 256);
    short payload_len = htons(len);
    memcpy(m_data.szPayload,&payload_len,sizeof(payload_len));
    return len + 2;
}
