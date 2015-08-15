#include "SslTcpClient.h"
#include "HttpMsg.h"
#include "SocketUtils.h"
#include "ApsApnsClientPool.h"
SslTcpClient::SslTcpClient(ApsApnsClientPool* pool)
:	TcpClient(NULL)
,	fTcpSocket(new SslTcpClientSocket(Socket::kNonBlockingSocketType))
,	m_clientPool(pool)
{
	memset(fUserTypeBuf, 0, kMaxUserTypeLen);
	memset(fUserNameBuf, 0, kMaxUserNameLen);
	memset(fUserPasswordBuf, 0, kMaxUserPasswordLen);
	memset(fUserAppName, 0, kMaxUserParamLen);
	memset(fUserDevId, 0, kMaxUserParamLen);
	memset(fUserRealmBuf, 0, kMaxUserParamLen);
	fTaskQueueElem.SetEnclosingObject(this);
	ListZero(&fMsgList);
	SetClientSocket(fTcpSocket);
//	fTcpSocket = new SslTcpClientSocket(Socket::kNonBlockingSocketType);
	ApsLogger::Debug("SslTcpClient This = %x!",this);
	SetTimeout(30*60*1000);
}

void	SslTcpClient::InitializeParam(StrPtrLen	addr,short port,StrPtrLen	password,StrPtrLen	path,StrPtrLen	cert,StrPtrLen	key)
{
	fTcpSocket->InitializeParam(addr,port,password,path,cert,key);
}

SslTcpClient::~SslTcpClient()
{
	OSMutexLocker locker(&fMutexList);
	ListEmpty(&fMsgList);
}

void SslTcpClient::SetUserInfo(const char*userType, const char*userName, const char*password, const char* appName, const char* devId, const char*realm)
{
	Assert(NULL != userType);
	Assert(kMaxUserTypeLen>=strlen(userType));
	memcpy(fUserTypeBuf, userType, strlen(userType));

	Assert(NULL != userName);
	Assert(kMaxUserNameLen>=strlen(userName));
	memcpy(fUserNameBuf, userName, strlen(userName));

	Assert(NULL != password);
	Assert(kMaxUserPasswordLen>=strlen(password));
	memcpy(fUserPasswordBuf, password, strlen(password));
	
	Assert(NULL != appName);
	Assert(kMaxUserParamLen>=strlen(appName));
	memcpy(fUserAppName, appName, strlen(appName));

	Assert(NULL != devId);
	Assert(kMaxUserParamLen>=strlen(devId));
	memcpy(fUserDevId, devId, strlen(devId));

	Assert(NULL != realm);
	Assert(kMaxUserParamLen>=strlen(realm));
	memcpy(fUserRealmBuf, realm, strlen(realm));
}

void SslTcpClient::Online()
{
	//fStatus = kOnline;
	SetConnectStatus(kOnline);
	SetTimeout(kTimeKeepAlive*1000);
}

void SslTcpClient::Offline()
{
	//fStatus = kOffline;
	SetConnectStatus(kOffline);
	SetTimeout(0);

	//Clear msg in list.
	OSMutexLocker locker(&fMutexList);
	
//	ListEmpty(&fMsgList);
}

void SslTcpClient::OnTimeout()
{
	OSMutexLocker locker(&fMutexList);
	SetTimeout(kTimeKeepAlive*1000);
	SetConnectStatus(kOffline);
	if(fTcpSocket != NULL)
	{
		delete fTcpSocket;
	}
	fTcpSocket = new SslTcpClientSocket(Socket::kNonBlockingSocketType);
	SetClientSocket(fTcpSocket);
}

SslTcpClientSocket& 	SslTcpClient::GetClientSocket()
{
	return *fTcpSocket;
}

void SslTcpClient::OnNetConnecting()
{
	if(GetConnectStatus() != kConnecting)
	{
		//fStatus = kConnecting;
		SetConnectStatus(kConnecting);
		SetTimeout(kTimeReconnect*1000);
	}
}

void SslTcpClient::OnNetConnectFailed()
{
	Offline();
}

void SslTcpClient::OnNetConnectted()
{
	SetTimeout(kTimeKeepAlive*1000);
}

void SslTcpClient::OnNetDisconnect()
{
	//m_clientPool->fQueue.EnQueue(&fTaskQueueElem);
	//m_clientPool->Signal(Task::kConnectEvent);
	Offline();
}

ZQ_Error	SslTcpClient::OnSendMessage(char* pBuffer, UInt32& nLen)
{
	ZQ_Error err = OS_NoErr;


	OSMutexLocker locker(&fMutexList);
	char* pData = new char[nLen];
	memcpy(pData, pBuffer, nLen);
	ListAppend(&fMsgList, pData, nLen);

	NotifySendMsg();

	return OS_NoErr;
}

ZQ_Error SslTcpClient::OnTrySendMsg()
{
	ZQ_Error err = OS_NoErr;
	OSMutexLocker locker(&fMutexList);
	ListElement *elem = fMsgList.first;
	if(NULL != elem)
	{
		err = SendData((char *)elem->content, elem->size);
		if(err == OS_NoErr)
		{
			RefreshTimeout();
			ListRemoveHead(&fMsgList);
			if(fMsgList.first != NULL)
				NotifySendMsg();
		}
		else//add to connect list
		{
			m_clientPool->fQueue.EnQueue(&fTaskQueueElem);
			m_clientPool->Signal(Task::kConnectEvent);
			err = OS_NotEnoughSpace;
			qtss_printf("Try to send msg.\n");
		}
	}
	return err;
}

ZQ_Error	SslTcpClient::OnProcessRecvMsg(char* pBuffer, UInt32& nLen)
{
	return OS_NoErr;
}

