#ifndef _AVCON_PUSH_SERVER_H_
#define _AVCON_PUSH_SERVER_H_
#include "XMLPrefsParser.h"
#include "StringParser.h"
#include "ZQServer.h"

//服务器类，只有一个实例，单例模式。
class AvconPushServer : public ZQServer
{
public:
	//call this before calling anything else
	static AvconPushServer* GetInstance(void);

	virtual ~AvconPushServer(void);

	//
    // Initialize
    //
    // This function starts the server. If it returns true, the server has
    // started up sucessfully. If it returns false, a fatal error occurred
    // while attempting to start the server.
    //
    // This function *must* be called before the server creates any threads,
    // because one of its actions is to change the server to the right UID / GID.
    // Threads will only inherit these if they are created afterwards.
    bool Initialize(void);

	//
    // StartTasks
    //
    // The server has certain global tasks that it runs for things like stats
    // updating and RTCP processing. This function must be called to start those
    // going, and it must be called after Initialize                
    void StartTasks();

	//
	// Run every * seconds to check all status.
	bool ProcessMsg();
protected:
	//
    // CreateListeners
    //
    // This function may be called multiple times & at any time.
    // It updates the server's listeners to reflect what the preferences say.
    // Returns false if server couldn't listen on one or more of the ports, true otherwise
    bool CreateListeners(Bool16 startListeningNow);

private:
	AvconPushServer(void);

	static AvconPushServer* m_pushServer;
	static OSMutex        m_Lock;   //lock   
};
#endif

