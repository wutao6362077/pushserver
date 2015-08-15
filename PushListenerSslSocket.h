#ifndef __PUSHLISTENERSSLSOCKET_H__
#define __PUSHLISTENERSSLSOCKET_H__
#include "SocketUtils.h"
#include "XMLPrefsParser.h"
#include "StringParser.h"

#include "TCPListenerSocket.h"
#include "Task.h"
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h> 

class PushListenerSslSocket : public TCPListenerSocket
{
public:
        PushListenerSslSocket();
        virtual ~PushListenerSslSocket();	
		void InitializeParam();
        OS_Error        Initialize(); 
        //sole job of this object is to implement this function
        virtual Task*   GetSessionTask(int osSocket, struct sockaddr_in* addr);
private:
		SSL_CTX 		*m_pctx;
		SSL 			*m_pssl;
		const SSL_METHOD	  *m_pmeth;
		X509			*m_pserver_cert;
		EVP_PKEY		*m_pkey;

		static XMLPrefsParser*	sPrefsSource;
		StrPtrLen		m_strAddr;
		short			m_nPort;
		StrPtrLen		m_strPath;
		StrPtrLen		m_strCert;
		StrPtrLen       m_strKey;
};

#endif
