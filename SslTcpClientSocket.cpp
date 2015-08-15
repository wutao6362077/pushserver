#include "SslTcpClientSocket.h"
#include "ApsConfigParser.h"
#include "ZQDataconverter.h"
#include "ApsLogger.h"

#ifndef __Win32__
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif

//��APNs�������࣬�����������PushClientSocket������������ת����APNs��
//���SslTCPSocketʵ��
SslTCPClientSocket::SslTCPClientSocket(): TCPSocket(NULL, 0)
{

	ApsLogger::Debug("SslTCPClientSocket construct!");	
}

SslTCPClientSocket::~SslTCPClientSocket()
{

}

bool SslTCPClientSocket::Read(SSL *m_pssl,void *buffer, const int length, int *outRecvLenP)
{
	bool reconnect_flag = false;

    int ret = SSL_read(m_pssl, buffer, length);
	*outRecvLenP = ret;
	int nRes = SSL_get_error(m_pssl, ret);
	if(nRes == SSL_ERROR_NONE)//�޴���
    { 	   	
		ApsLogger::Debug("This = %x read ret = %d successful!",this,ret);
    }
    else if (nRes == SSL_ERROR_WANT_WRITE)//д����
    {
		// If we get this error, we are currently flow-controlled and should
		// wait for the socket to become writeable again
		this->RequestEvent(EV_WR);
		
		ApsLogger::Debug("This = %x read block, Add to list.",this);     	
		//printf("Add to list!\n");
    }
	else//���Ӵ���
	{
		ApsLogger::Debug("connect error will restart."); 
		reconnect_flag = true;
	}

	return reconnect_flag;

}

