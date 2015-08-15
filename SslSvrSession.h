#ifndef _HTTP_SESSION_H_
#define _HTTP_SESSION_H_

#include "LinkedList.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "TCPListenerSocket.h"
#include "TCPSslSessionInterface.h"
#include "TCPRequestStream.h"
#include "json/json.h"
#include "ProviderPacket.h"

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h> 

class SslSvrSession : public TCPSslSessionInterface
{
public:
	SslSvrSession(int fd,struct sockaddr_in *addr,SSL_CTX *m_pctx);
	virtual ~SslSvrSession(void);

	//Is this session alive? If this returns false, clean up and begone as
	//fast as possible
	Bool16 IsLiveSession()      { return fSocket.IsConnected(); }
	int PacketHandle(ProviderPacket &packet);

//	OS_Error		GenHttpRespMsg(StrPtrLen* cmdParams, const char* content, StrPtrLen *contentType, HTTPStatusCode status = httpOK);

	//Process the http request.
//	virtual void		OnHttpRequest(HTTPRequest* pRequest);
protected:
	virtual SInt64 Run();
    
    // test current connections handled by this object against server pref connection limit
    Bool16 OverMaxConnections(UInt32 buffer);

	UInt32				fState;
    HTTPRequest			fHttpReq;

	ZQ_Error	ProcessRecvMsg(char* pBuffer, UInt32& nLen);		// process Request
//	void		PushHttpMsg(const char* pData, int nLen);
//	void		SendHttpRespMsg(const char* data, int nLen);
//	void		PutResponse(const char* data, int nLen);
	void		CleanupRequest();	

private:
	List				fMsgList;
	OSMutex				fMutexList;

	SSL 			*m_pssl;
	X509			*m_pserver_cert;
	EVP_PKEY		*m_pkey;
	std::string appId;

	Json::Reader reader;
	Json::Value root;
};

#endif

