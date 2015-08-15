#ifndef __MQTTLISTENERSOCKET_H__
#define __MQTTLISTENERSOCKET_H__
#include "XMLPrefsParser.h"
#include "SocketUtils.h"
#include "XMLPrefsParser.h"
#include "StringParser.h"

#include "TCPListenerSocket.h"
#include "Task.h"

class MqttListenerSocket : public TCPListenerSocket
{
public:
        MqttListenerSocket() {}
        virtual ~MqttListenerSocket() {}	
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
