#ifndef PUSHC2DMCLIENTSOCKET_H
#define PUSHC2DMCLIENTSOCKET_H
#include <stdio.h>
#include "SslAndroidTcpSocket.h"
#include "ApsRedisLocation.h"
#include "ApsTokenRedisLocation.h"
#include "ApsMongodbLocation.h"

class PushC2DMClientSocket
{
public:
	//call this before calling anything else
	static PushC2DMClientSocket* GetInstance(void);
	void Initialize();
    ~PushC2DMClientSocket();

    void PushNotification(const char *pToken,const char *pMsg);
    void GenPushData(const char *pToken);
    int GenPayloadData(int badgeNum,const char *pMsg = NULL);

private:
	PushC2DMClientSocket();
    void Reset();

private:
	static PushC2DMClientSocket* m_APNsSocket;
	static OSMutex        m_Lock;   //lock  
    AndroidSslTCPSocket * m_sslSocket;

    int m_tokenLen;
    struct PUSHDATA
    {
        char szToken[1+2+32];
        char szPayload[2+256];
    }m_data;
};
#endif /* PUSHCLIENTSOCKET_H */
