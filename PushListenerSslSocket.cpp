#include "PushListenerSslSocket.h"
#include "SslSvrSession.h"
#include "ApsLogger.h"
#include "ApsConfigParser.h"
#include "ZQDataconverter.h"

XMLPrefsParser* PushListenerSslSocket::sPrefsSource = NULL;

void PushListenerSslSocket::InitializeParam()
{
	sPrefsSource = ApsConfigParser::GetInstance();

	ContainerRef svrPref = sPrefsSource->GetRefForC2DMServer();
	ContainerRef pref = NULL;
    char* thePrefValue = NULL;
	char* thePrefTypeStr = NULL;
    char* thePrefName = NULL;
	ZQ_AttrDataType theType;
	UInt32 convertedBufSize = 0;
	//Get the APNs server ip
	pref = sPrefsSource->GetPrefRefByName( svrPref, "c2dm_server_ip" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strAddr.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}

	//Get the APNs server port
    pref = sPrefsSource->GetPrefRefByName( svrPref, "c2dm_server_port");
	if (pref != NULL)
	{
		if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName, (char**)&thePrefTypeStr))
		{
			theType = ZQDataConverter::TypeStringToType(thePrefTypeStr);
			convertedBufSize = sizeof(UInt16);
			ZQDataConverter::StringToValue(thePrefValue, theType, &m_nPort, &convertedBufSize);
		}
	}

	//Get the certification dir
	pref = sPrefsSource->GetPrefRefByName( svrPref, "c2dm_cert_path" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strPath.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the certification file path
	pref = sPrefsSource->GetPrefRefByName( svrPref, "c2dm_server_cert" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strCert.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the certification key path
	pref = sPrefsSource->GetPrefRefByName( svrPref, "c2dm_server_key" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strKey.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
}

OS_Error PushListenerSslSocket::Initialize()
{
	TCPListenerSocket::SetServerParam(SocketUtils::ConvertStringToAddr(m_strAddr.GetAsCString()), m_nPort);
	return TCPListenerSocket::Initialize();
}

PushListenerSslSocket::PushListenerSslSocket(void):m_pmeth(NULL),m_pctx(NULL)
{
	InitializeParam();

	// 设置客户端使用的SSL版本
	m_pmeth = SSLv23_method();
	
	// 创建SSL上下文环境
	// 每个进程只需维护一个SSL_CTX结构体
	m_pctx = SSL_CTX_new(m_pmeth);

	/* Load the CA from the Path */
//    if(SSL_CTX_load_verify_locations(m_pctx, "ca-cert.key", "/iospush") <= 0)
	if(SSL_CTX_load_verify_locations(m_pctx, NULL, m_strPath.GetAsCString()) <= 0)
    {
        /* Handle failed load here */
		ApsLogger::Debug("Failed to set CA location..."); 
        //printf("Failed to set CA location...\n");
        ERR_print_errors_fp(stderr);
        return;
    }
	
	// 读取证书文件
    /* Load the client certificate into the SSL_CTX structure */
	int err = SSL_CTX_use_certificate_file(m_pctx, m_strCert.GetAsCString(), SSL_FILETYPE_PEM);
    if (err <= 0)
    {
        ApsLogger::Debug("Cannot use Certificate File\n");
        ERR_print_errors_fp(stderr);
        return;
    }

    /* Load the private-key corresponding to the client certificate */
	err = SSL_CTX_use_PrivateKey_file(m_pctx, m_strKey.GetAsCString(), SSL_FILETYPE_PEM);
    if ( err<= 0)
    {
        ApsLogger::Debug("Cannot use Private Key\n");
        ERR_print_errors_fp(stderr);
        return;
    }

    /* Check if the client certificate and private-key matches */
	err = SSL_CTX_check_private_key(m_pctx);
    if (err == 0)
    {
        ApsLogger::Debug("Private key does not match the certificate public key\n");
        return;
    }

    /* Check if the client certificate and private-key matches */
	//SSL_CTX_set_verify(m_pctx,SSL_VERIFY_PEER,NULL);
}

PushListenerSslSocket::~PushListenerSslSocket()
{
	if(m_pmeth != NULL)
	{
		delete m_pmeth;
	}
	if(m_pctx != NULL)
	{
		delete m_pctx;
	}
}

//acceptor模块调用GetSessionTask函数，创建一个连接会话，处理该socket句柄上的所有任务。
Task*   PushListenerSslSocket::GetSessionTask(int osSocket, struct sockaddr_in* addr)
{
	Assert(osSocket != EventContext::kInvalidFileDesc);
 	TCPSocket* theSocket = NULL; 
	
	//new one web accept session
    SslSvrSession* theTask = new SslSvrSession(osSocket,addr,m_pctx);
	ApsLogger::Debug("New PushListenerSslSocket, socket fd = %d, thetask = %p!",osSocket,theTask); 
	if(NULL == theTask)
		return NULL;
    theSocket = theTask->GetSocket();  // out socket is not attached to a unix socket yet.

	//set options on the socket
	int sndBufSize = 96L * 1024L;
    theSocket->InitNonBlocking(osSocket);
	theSocket->NoDelay();
	theSocket->KeepAlive();
	theSocket->SetSocketBufSize(sndBufSize);
    theSocket->RequestEvent(EV_RE);

    this->RunNormal();
        
    return theTask;
}
