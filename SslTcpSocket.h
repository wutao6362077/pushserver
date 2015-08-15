#ifndef SSLTCPSOCKET_H
#define SSLTCPSOCKET_H
#include <stdlib.h>
#include <assert.h>
#include <string>
#include "TCPSocket.h"
#include "IdleTask.h"
#include "StrPtrLen.h"
#include "OSMutex.h"
#include "MyAssert.h"
#include "SocketUtils.h"
#include "XMLPrefsParser.h"
#include "StringParser.h"
#include "LinkedList.h"

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h> 


class SslTcpSocket : public TCPSocket
{
public:
    SslTcpSocket();
    ~SslTcpSocket();
	OS_Error Initialize();
	void SetAddr(StrPtrLen addr);
	void SetPort(uint16_t port);
	void SetPassword(StrPtrLen password);
	void SetPath(StrPtrLen path);
	void SetKey(StrPtrLen key);
	void SetCert(StrPtrLen cert);
	virtual OS_Error	Connect(UInt32 inRemoteAddr, UInt16 inRemotePort);
    bool ssl_connect(const char *host, int port,const char *password, const char *certfile, const char *keyfile, const char* capath);
	virtual OS_Error	Send(const char* inData, const UInt32 inLength, UInt32* outLengthSent);
	virtual OS_Error Read(void *buffer, const UInt32 length, UInt32 *rcvLen);
private:
    void Reset();

private:

    SSL_CTX         *m_pctx;
    SSL             *m_pssl;
    const SSL_METHOD      *m_pmeth;
    X509            *m_pserver_cert;
    EVP_PKEY        *m_pkey;

    /* Socket Communications */
    struct sockaddr_in   m_server_addr;
    struct hostent      *m_phost_info;

	StrPtrLen		m_strAddr;
	short			m_nPort;
	StrPtrLen		m_strPassword;
	StrPtrLen		m_strPath;
	StrPtrLen		m_strCert;
	StrPtrLen       m_strKey;
	List				fMsgList;
	OSMutex				fMutexList;

	
	OSMutex        m_OperationLock;   //per thread one opration
};
#endif
