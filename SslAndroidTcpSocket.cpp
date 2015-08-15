#include "SslAndroidTcpSocket.h"
#include "ApsConfigParser.h"
#include "ZQDataconverter.h"
#include "ApsLogger.h"

#ifndef __Win32__
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif

XMLPrefsParser* AndroidSslTCPSocket::sPrefsSource = NULL;

//与APNs的连接类，负责接收来自PushClientSocket类推送来数据转发到APNs。
//多个SslTCPSocket实例
AndroidSslTCPSocket::AndroidSslTCPSocket(): TCPSocket(NULL, 0), IdleTask()
{
    //ctor
    m_pctx = NULL;
    m_pssl = NULL;
    m_pmeth = NULL;
    m_pserver_cert = NULL;
    m_pkey = NULL;
	connected = false;

	if(Initialize()!=1)
	{
		ApsLogger::Fatal("AndroidSslTCPSocket init error!");
		exit(-1);
	}
	ApsLogger::Debug("AndroidSslTCPSocket init successful!");	
	connected = true;
	ListZero(&fMsgList);
}

AndroidSslTCPSocket::~AndroidSslTCPSocket()
{
    //dtor
    Reset();
	OSMutexLocker locker(&fMutexList);
	ListEmpty(&fMsgList);
}
void AndroidSslTCPSocket::Reset()
{
    if(m_pssl)
    {
        SSL_shutdown(m_pssl);
        SSL_free(m_pssl);
        m_pssl = NULL;
    }
    if(m_pctx)
    {
        SSL_CTX_free(m_pctx);
        m_pctx = NULL;
    }
	connected = false;
}

void AndroidSslTCPSocket::InitializeParam()
{
	sPrefsSource = ApsConfigParser::GetInstance();

	ContainerRef svrPref = sPrefsSource->GetRefForC2DMClient();
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

	//Get the APNs feedback server ip
	pref = sPrefsSource->GetPrefRefByName( svrPref, "c2dm_feedback_server_ip" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strFeedbackAddr.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}

	//Get the APNs feedback server port
    pref = sPrefsSource->GetPrefRefByName( svrPref, "c2dm_feedback_server_port");
	if (pref != NULL)
	{
		if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName, (char**)&thePrefTypeStr))
		{
			theType = ZQDataConverter::TypeStringToType(thePrefTypeStr);
			convertedBufSize = sizeof(UInt16);
			ZQDataConverter::StringToValue(thePrefValue, theType, &m_nFeedbackPort, &convertedBufSize);
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
	pref = sPrefsSource->GetPrefRefByName( svrPref, "c2dm_client_cert" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strCert.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the certification key path
	pref = sPrefsSource->GetPrefRefByName( svrPref, "c2dm_client_key" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strKey.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
}

OS_Error AndroidSslTCPSocket::Initialize()
{
	int num = 0;
	InitializeParam();
	//创建并连接SSL，阻塞模式。如果非阻塞会导致连接建立不成功
    OS_Error err = this->TCPSocket::Open();
    if (0 == err) do
    {
       err = ssl_connect(m_strAddr.GetAsCString(), m_nPort, m_strCert.GetAsCString(), m_strKey.GetAsCString(), m_strPath.GetAsCString());
    } while (!err&&(num++<5));//reconnect 5 times
	//连接成功后才能变为非阻塞模式
    this->SetNonBlocking();
	//设置Task，响应系统的读写事件
	this->SetTask(this);
    return err;
}

OS_Error AndroidSslTCPSocket::ssl_connect(const char *host, int port, const char *certfile, const char *keyfile, const char* capath)
{
    Reset();

    int err = 0;

    /* Create an SSL_METHOD structure (choose an SSL/TLS protocol version) */
    m_pmeth = SSLv23_method();

    /* Create an SSL_CTX structure */
    m_pctx = SSL_CTX_new(m_pmeth);
    if(!m_pctx)
    {
    	
		ApsLogger::Debug("Could not get SSL Context!"); 
        //printf("Could not get SSL Context\n");
        return false;
    }

    /* Load the CA from the Path */
    if(SSL_CTX_load_verify_locations(m_pctx, NULL, capath) <= 0)
    {
        /* Handle failed load here */
		ApsLogger::Debug("Failed to set CA location..."); 
        //printf("Failed to set CA location...\n");
        ERR_print_errors_fp(stderr);
        return false;
    }

    /* Load the client certificate into the SSL_CTX structure */
    if (SSL_CTX_use_certificate_file(m_pctx, certfile, SSL_FILETYPE_PEM) <= 0)
    {
		ApsLogger::Debug("Cannot use Certificate File."); 
        //printf("Cannot use Certificate File\n");
        ERR_print_errors_fp(stderr);
        return false;
    }

    /* Load the private-key corresponding to the client certificate */
	err = SSL_CTX_use_PrivateKey_file(m_pctx, keyfile, SSL_FILETYPE_PEM);
    if ( err<= 0)
    {
    	
		ApsLogger::Debug("Cannot use Private Key."); 
      //  printf("Cannot use Private Key\n");
        ERR_print_errors_fp(stderr);
        return false;
    }

    /* Check if the client certificate and private-key matches */
	err = SSL_CTX_check_private_key(m_pctx);
    if (err == 0)
    {
		ApsLogger::Debug("Private key does not match the certificate public key."); 
      //  printf("Private key does not match the certificate public key\n");
        return false;
    }

    memset (&m_server_addr, '\0', sizeof(m_server_addr));
    m_server_addr.sin_family      = AF_INET;
    m_server_addr.sin_port        = htons(port);       /* Server Port number */
    m_phost_info = gethostbyname(host);
    if(m_phost_info)
    {
        /* Take the first IP */
        struct in_addr *address = (struct in_addr*)m_phost_info->h_addr_list[0];
        m_server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*address)); /* Server IP */

    }
    else
    {
    	
		ApsLogger::Debug("Could not resolve hostname %s.",host); 
        printf("Could not resolve hostname %s\n", host);
        return false;
    }

	err = TCPSocket::Connect(ntohl(m_server_addr.sin_addr.s_addr),port);
    /* Establish a TCP/IP connection to the SSL client */
    //err = connect(fFileDesc, (struct sockaddr*) &m_server_addr, sizeof(m_server_addr));
    if(err == -1)
    {
		ApsLogger::Debug("Could not connect."); 
       // printf("Could not connect\n");
        return false;
    }

    /* An SSL structure is created */
    m_pssl = SSL_new(m_pctx);
    if(!m_pssl)
    {
		ApsLogger::Debug("Could not get SSL Socket.");     	
        //printf("Could not get SSL Socket\n");
        return false;
    }

    /* Assign the socket into the SSL structure (SSL and socket without BIO) */
    SSL_set_fd(m_pssl, fFileDesc);

    /* Perform SSL Handshake on the SSL client */
	err = ::SSL_connect(m_pssl);
    if(err != 1)
    {
	    const char *errbuf;
		err = SSL_get_error(m_pssl,err);
        errbuf = ERR_reason_error_string(err);

        char ErrBuff[1024];
        ERR_load_crypto_strings();   
        ERR_error_string(ERR_get_error(),ErrBuff);    	
        ApsLogger::Debug("AndroidSslTCPSocket Could not connect to SSL Server error code is %d,error string is %s!",err,errbuf);
        return false;
    }
    return true;
}

bool AndroidSslTCPSocket::Send(const void *data, int length,int *sendLength)
{
	bool reconnect_flag = false;

    int ret = SSL_write(m_pssl, data, length);
	int nRes = SSL_get_error(m_pssl, ret);
	if(nRes == SSL_ERROR_NONE)//无错误
    { 	   	
		ApsLogger::Debug("AndroidSslTCPSocket This = %x send ret = %d successful!",this,ret);
    }
    else if (nRes == SSL_ERROR_WANT_WRITE)//写阻塞
    {
       	char* pData = new char[length];
		memcpy(pData, data, length);

		if(pData)
		{
			OSMutexLocker locker(&fMutexList); 
			ListAppend(&fMsgList, pData, length);
		}
		// If we get this error, we are currently flow-controlled and should
		// wait for the socket to become writeable again
		this->RequestEvent(EV_WR);
		
		ApsLogger::Debug("AndroidSslTCPSocket This = %x Send block, Add to list.",this);     	
		//printf("Add to list!\n");
    }
	else//连接错误
	{
		ApsLogger::Debug("AndroidSslTCPSocket connect error will restart."); 
		reconnect_flag = true;
	}

	return reconnect_flag;
}

bool AndroidSslTCPSocket::Read(void *buffer, const int length, int *outRecvLenP)
{
	bool reconnect_flag = false;

    int ret = SSL_read(m_pssl, buffer, length);
	*outRecvLenP = ret;
	int nRes = SSL_get_error(m_pssl, ret);
	if(nRes == SSL_ERROR_NONE)//无错误
    { 	   	
		ApsLogger::Debug("AndroidSslTCPSocket This = %x read ret = %d successful!",this,ret);
    }
    else if (nRes == SSL_ERROR_WANT_WRITE)//写阻塞
    {
		// If we get this error, we are currently flow-controlled and should
		// wait for the socket to become writeable again
		this->RequestEvent(EV_WR);
		
		ApsLogger::Debug("AndroidSslTCPSocket This = %x read block, Add to list.",this);     	
		//printf("Add to list!\n");
    }
	else//连接错误
	{
		ApsLogger::Debug("AndroidSslTCPSocket connect error will restart."); 
		reconnect_flag = true;
	}

	return reconnect_flag;

}

SInt64 AndroidSslTCPSocket::Run()
{
    EventFlags events = this->GetEvents();
    
    //
    // ProcessEvent cannot be going on when this object gets deleted, because
    // the resolve / release mechanism of EventContext will ensure this thread
    // will block before destructing stuff.
    if (events & Task::kKillEvent)
        return -1;
           
    //This function will get called when we have run out of file descriptors.
    //All we need to do is check the listen queue to see if the situation has
    //cleared up.
    //(void)this->GetEvents();
    //this->ProcessEvent(Task::kWriteEvent);
	if(events&Task::kWriteEvent)
	{
			OSMutexLocker locker(&fMutexList); 
			ListElement *elem = NULL; 
			UInt32 theLengthSent = 0;
			if((elem = fMsgList.first) != NULL) 
			{ 
				int err = SSL_write(m_pssl,(char*)elem->content, elem->size);
				ApsLogger::Debug("AndroidSslTCPSocket this = %x run send ret = %d!",this,err);
				//err = fSocket.Send((char*)elem->content, elem->size, &theLengthSent);
				if (err <= 0)//直接推出有问题，导致PushClientSocket中的指针无效，后续修改。
				{		
					ApsLogger::Debug("AndroidSslTCPSocket Resend message error.");		
					return 0;
				}
				else if (err != elem->size)
				{
					ApsLogger::Debug("AndroidSslTCPSocket Resend part of message, May muddle the message.");
					this->RequestEvent(EV_WR);
				}
				else
				{
					ApsLogger::Debug("AndroidSslTCPSocket Resend message successful.");	
					ListRemoveHead(&fMsgList); 
					if(NULL != fMsgList.first)
						this->Signal(kWriteEvent);
				}	
			}
	}
    return 0;
}
