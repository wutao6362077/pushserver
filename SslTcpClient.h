#ifndef _SSL_TCP_CLENT_H_
#define _SSL_TCP_CLENT_H_

#include "TcpSslClientSocket.h"
#include "TcpClient.h"
class ApsApnsClientPool;

class SslTcpClient : public TcpClient
{
public:
	SslTcpClient(ApsApnsClientPool* pool);
	virtual ~SslTcpClient(void);

public:
	void				SetUserInfo(const char*type, const char*username, const char*password, const char* appName, const char* devId, const char*realm);
	char*				GetUserType(){return fUserTypeBuf;};
	char*				GetUserName(){return fUserNameBuf;};
	void 				InitializeParam(StrPtrLen	addr,short port,StrPtrLen password,StrPtrLen	path,StrPtrLen	cert,StrPtrLen	key);
	SslTcpClientSocket&			GetClientSocket();

	OSQueueElem 	fTaskQueueElem;
	virtual	ZQ_Error		OnTrySendMsg();
	ApsApnsClientPool* m_clientPool;
	virtual void            OnTimeout();
	void				Online();
	void				Offline();

protected:
	

	ZQ_Error				OnSendMessage(char* pBuffer, UInt32& nLen);
	
protected:

	virtual void			OnNetConnecting();
	virtual void			OnNetConnectFailed();
	virtual void			OnNetConnectted();
	virtual void            OnNetDisconnect();
	virtual ZQ_Error		OnProcessRecvMsg(char* pBuffer, UInt32& nLen);

protected:
	SslTcpClientSocket			*fTcpSocket;
	char				fUserTypeBuf[kMaxUserTypeLen];
	char                fUserNameBuf[kMaxUserNameLen];
    char                fUserPasswordBuf[kMaxUserPasswordLen];
	char				fUserAppName[kMaxUserParamLen];
	char				fUserDevId[kMaxUserParamLen];
    char                fUserRealmBuf[kMaxUserParamLen];

private:
	List				fMsgList;
	OSMutex				fMutexList;

};

#endif

