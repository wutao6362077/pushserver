#include "SslTcpSocket.h"
#include "ApsConfigParser.h"
#include "ZQDataconverter.h"
#include "ApsLogger.h"

#ifndef __Win32__
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif

//与APNs的连接类，负责接收来自PushClientSocket类推送来数据转发到APNs。
//多个SslTCPSocket实例
SslTcpSocket::SslTcpSocket(): TCPSocket(NULL, 0)
{
    //ctor
    m_pctx = NULL;
    m_pssl = NULL;
    m_pmeth = NULL;
    m_pserver_cert = NULL;
    m_pkey = NULL;

	ListZero(&fMsgList);
}

SslTcpSocket::~SslTcpSocket()
{
    //dtor
    Reset();
	OSMutexLocker locker(&fMutexList);
	ListEmpty(&fMsgList);
}
void SslTcpSocket::Reset()
{
    if(m_pssl)
    {
//        SSL_shutdown(m_pssl);
        SSL_free(m_pssl);
        m_pssl = NULL;
    }
    if(m_pctx)
    {
        SSL_CTX_free(m_pctx);
        m_pctx = NULL;
    }
}

OS_Error	SslTcpSocket::Connect(UInt32 inRemoteAddr, UInt16 inRemotePort)
{
	if(!Initialize())
	{
		ApsLogger::Fatal("0X%x:SslTCPSocket init error!");
		//exit(-1);
		return OS_NotEnoughSpace;
	}
	ApsLogger::Debug("0X%x:SslTCPSocket init successful!",this);	
	fState &= kConnected;

	return OS_NoErr;
}

void SslTcpSocket::SetAddr(StrPtrLen addr)
{
	m_strAddr = addr;
}
void SslTcpSocket::SetPort(uint16_t port)
{
	m_nPort = port;
}
void SslTcpSocket::SetPassword(StrPtrLen password)
{
	m_strPassword = password;
}

void SslTcpSocket::SetPath(StrPtrLen path)
{
	m_strPath = path;
}
void SslTcpSocket::SetKey(StrPtrLen key)
{
	m_strKey = key;
}
void SslTcpSocket::SetCert(StrPtrLen cert)
{
	m_strCert = cert;
}

OS_Error SslTcpSocket::Initialize()
{
	//创建并连接SSL，阻塞模式。如果非阻塞会导致连接建立不成功
    bool err ;//= this->TCPSocket::Open();
    err = ssl_connect(m_strAddr.GetAsCString(), m_nPort,m_strPassword.GetAsCString(), m_strCert.GetAsCString(), m_strKey.GetAsCString(), m_strPath.GetAsCString());
	if (err)
    {
		//连接成功后才能变为非阻塞模式
	    this->SetNonBlocking();
	}
    return err;
}

bool SslTcpSocket::ssl_connect(const char *host, int port,const char *password, const char *certfile, const char *keyfile, const char* capath)
{
	OSMutexLocker locker(&m_OperationLock);

    Reset();

    int err = 0;

    /* Create an SSL_METHOD structure (choose an SSL/TLS protocol version) */
    //m_pmeth = SSLv23_method();
	m_pmeth = TLSv1_method();

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
	SSL_CTX_set_default_passwd_cb_userdata(m_pctx, (void*)password);
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
        ApsLogger::Debug("Could not connect to SSL Server error code is %d,error string is %s!",err,errbuf);
        return false;
    }
    return true;
}

OS_Error SslTcpSocket::Send(const char* inData, const UInt32 inLength, UInt32* outLengthSent=NULL)
{	
	if (!(fState & kConnected))
		return (OS_Error)ENOTCONN;
	OSMutexLocker locker(&m_OperationLock);		
	int err;
	err = SSL_write(m_pssl, inData, inLength);

	int nRes = SSL_get_error(m_pssl, err);
	if(nRes == SSL_ERROR_NONE)//无错误
    { 	   	
		ApsLogger::Debug("This = %x send ret = %d successful!",this,err);
		//*sendLength = err;
    }
    else if (nRes == SSL_ERROR_WANT_WRITE)//写阻塞
    {		
		ApsLogger::Debug("This = %x Send block, Add to list.",this);     	
		//printf("Add to list!\n");
    }
	else//连接错误
	{
		ApsLogger::Debug("send connect error will restart."); 
		fState ^= kConnected;//turn off connected state flag
		return (OS_Error)ENOTCONN;
	}

	return OS_NoErr;
}

OS_Error SslTcpSocket::Read(void *buffer, const UInt32 length, UInt32 *rcvLen)
{
		
	if (!(fState & kConnected))
		return (OS_Error)ENOTCONN;
	
	OSMutexLocker locker(&m_OperationLock);			
	//int theRecvLen = ::recv(fFileDesc, buffer, length, 0);//flags??
	int theRecvLen;
	theRecvLen = SSL_read(m_pssl, (char*)buffer, 7/*length*/);//flags??
	
	int nRes = SSL_get_error(m_pssl, theRecvLen);
	if(nRes == SSL_ERROR_NONE)//无错误
    { 	   	
		ApsLogger::Debug("This = %x read ret = %d successful!",this,theRecvLen);
		*rcvLen = (UInt32)theRecvLen;
    }
    else if (nRes == SSL_ERROR_WANT_READ)//写阻塞
    {
		// If we get this error, we are currently flow-controlled and should
		// wait for the socket to become writeable again
		//this->RequestEvent(EV_WR);
		
		ApsLogger::Debug("This = %x read block, Add to list.",this); 
		return (OS_Error)EINPROGRESS;
		//printf("Add to list!\n");
    }
	else//连接错误
	{
		ApsLogger::Debug("read connect error will restart."); 
		fState ^= kConnected;
		return (OS_Error)ENOTCONN;
	}

	return OS_NoErr;
}

