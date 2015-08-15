#ifndef __APS_APP_TCP_LISTENER_SOCKET_H__
#define __APS_APP_TCP_LISTENER_SOCKET_H__
#include "SocketUtils.h"
#include "XMLPrefsParser.h"
#include "StringParser.h"

#include "TCPListenerSocket.h"
#include "Task.h"

class ApsAppTcpListenerSocket : public TCPListenerSocket
{
public:
        ApsAppTcpListenerSocket() {}
        virtual ~ApsAppTcpListenerSocket() {}
		void InitializeParam();
        OS_Error        Initialize();   
        //sole job of this object is to implement this function
        virtual Task*   GetSessionTask(int osSocket, struct sockaddr_in* addr);
private:
	static XMLPrefsParser*	sPrefsSource;
	StrPtrLen		m_strAddr;
	short			m_nPort;
};

#endif
