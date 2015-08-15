#ifndef SSLTCPCLIENTSOCKET_H
#define SSLTCPCLIENTSOCKET_H
#include <stdlib.h>
#include <assert.h>
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
#define DEVICE_BINARY_SIZE 32

class SslTCPClientSocket : public TCPSocket
{
public:
    SslTCPClientSocket();
    ~SslTCPClientSocket();
	//void InitializeParam();
	//OS_Error Initialize();
    //OS_Error ssl_connect(const char *host, int port, const char *certfile, const char *keyfile, const char* capath);
	//bool Send(const void *data, int length, int *outRecvLenP=NULL);	
	bool Read(SSL *m_pssl,void *buffer, const int length, int *outRecvLenP);
private:
    //void Reset();

private:

    /* Socket Communications */
    struct sockaddr_in   m_server_addr;
    struct hostent      *m_phost_info;

	bool connected;
	static XMLPrefsParser*	sPrefsSource;
	StrPtrLen		m_strAddr;
	short			m_nPort;
	StrPtrLen		m_strPath;
	StrPtrLen		m_strCert;
	StrPtrLen       m_strKey;
	StrPtrLen		m_strFeedbackAddr;
	short			m_nFeedbackPort;
	List				fMsgList;
	OSMutex				fMutexList;
};
#endif
