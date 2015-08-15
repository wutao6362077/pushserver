#include "AvconPushServer.h"

#include "OS.h"
#include "OSArrayObjectDeleter.h"
#include "SocketUtils.h"
#include "ZQDataconverter.h"
#include "ApsAppTcpListenerSocket.h"
#include "ApsConfigParser.h"
#include "ApsLogger.h"
#include "PushListenerSslSocket.h"
#include "MqttListenerSocket.h"

AvconPushServer* AvconPushServer::m_pushServer = NULL;
OSMutex        AvconPushServer::m_Lock;   //lock

AvconPushServer* AvconPushServer::GetInstance(void)
{
	if (m_pushServer == NULL)
    {
		OSMutexLocker locker(&m_Lock);
    	if (m_pushServer == NULL)
    	{
    	    ApsLogger::Debug("New ZQPushSvr!");
       		m_pushServer = new AvconPushServer();
    	}
    }

    return m_pushServer;
}

AvconPushServer::AvconPushServer(void)
{

}

AvconPushServer::~AvconPushServer(void)
{

}

bool AvconPushServer::Initialize()
{
    // BEGIN LISTENING
    if ( !this->CreateListeners(false) )
	{
        ApsLogger::Debug("4: ZQPushSvr create listeners error!");
		return false;
	}
	else
	{
		ApsLogger::Debug("4: ZQPushSvr create listeners successful!");
	}
	return true;;
}

void AvconPushServer::StartTasks()
{
    // Start listening
    for (UInt32 x = 0; x < fNumListeners; x++)
	{
        fListeners[x]->RequestEvent(EV_RE);
		ApsLogger::Debug("5: Start listening server ip:%s port:%d!", fListeners[x]->GetLocalAddrStr()->Ptr, fListeners[x]->GetLocalPort());
	}
}

bool AvconPushServer::CreateListeners(Bool16 startListeningNow)
{
    // Now figure out which of these ports we are *already* listening on.
    // If we already are listening on that port, just move the pointer to the
    // listener over to the new array
    TCPListenerSocket** newListenerArray = new TCPListenerSocket*[3];
    UInt32 curPortIndex = 0;
	newListenerArray[curPortIndex] = new ApsAppTcpListenerSocket();
	//UInt32 svrAddr = fWebSvrIP.Len==0?INADDR_ANY:SocketUtils::ConvertStringToAddr(fWebSvrIP.Ptr);
    ZQ_Error err = (newListenerArray[curPortIndex])->Initialize();
    
    // If there was an error creating this listener, destroy it and log an error
    if ((startListeningNow) && (err != ZQ_NoErr))
        delete newListenerArray[curPortIndex];
	if(err == ZQ_NoErr)
    {
        // This listener was successfully created.
        if (startListeningNow)
            newListenerArray[curPortIndex]->RequestEvent(EV_RE);
        curPortIndex++;
    }

	newListenerArray[curPortIndex] = new PushListenerSslSocket();
	err = (newListenerArray[curPortIndex])->Initialize();
	if(err == ZQ_NoErr)
    {
        // This listener was successfully created.
        if (startListeningNow)
            newListenerArray[curPortIndex]->RequestEvent(EV_RE);
        curPortIndex++;
    }

	newListenerArray[curPortIndex] = new MqttListenerSocket();
	err = (newListenerArray[curPortIndex])->Initialize();
	if(err == ZQ_NoErr)
    {
        // This listener was successfully created.
        if (startListeningNow)
            newListenerArray[curPortIndex]->RequestEvent(EV_RE);
        curPortIndex++;
    }

    // Finally, make our server attributes and fListener privy to the new...
    fListeners = newListenerArray;
    fNumListeners = curPortIndex;

    return (fNumListeners > 0);
}

bool AvconPushServer::ProcessMsg()
{
	return true;
}
