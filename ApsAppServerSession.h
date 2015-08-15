#ifndef __APS_APP_SERVER_SESSION__H_
#define __APS_APP_SERVER_SESSION__H_

#include "HttpSvrSession.h"
#include "TimeoutTask.h"
#include "ApsApnsClientPool.h"
#include "PushC2DMClientSocket.h"
#include "json/json.h"
#include "ApsAppRequestMessage.h"
#include "ApsAppResponseMessage.h"
#include "ApsLcsUtil.h"
enum push_msg_state {
	mosq_ms_invalid = 0,
	mosq_ms_publish_qos0 = 1,
	mosq_ms_publish_qos1 = 2,
	mosq_ms_wait_for_puback = 3,
	mosq_ms_publish_qos2 = 4,
	mosq_ms_wait_for_pubrec = 5,
	mosq_ms_resend_pubrel = 6,
	mosq_ms_wait_for_pubrel = 7,
	mosq_ms_resend_pubcomp = 8,
	mosq_ms_wait_for_pubcomp = 9,
	mosq_ms_send_pubrec = 10,
	mosq_ms_queued = 11
};

enum push_client_session_state {
	mosq_cs_new = 0,
	mosq_cs_rec_connect = 1,
	mosq_cs_send_connect_res = 2,
	mosq_cs_connected = 3,
	mosq_cs_disconnecting = 4
};

class ApsAppSrvSession : public HttpSvrSession, public ApsLcsWrapper
{
public:
		ApsAppSrvSession(ApsApnsClientPool * socket = ApsApnsClientPool::GetInstance(),PushC2DMClientSocket *C2DMsocket = PushC2DMClientSocket::GetInstance());
        virtual ~ApsAppSrvSession();
		//virtual SInt64 Run();
		virtual void OnHttpRequest(HTTPRequest* pRequest);
		void		OnPublish(ApsAppRequestMessage &request);
		void		OnPingReq(ApsAppRequestMessage &request);
		void		OnConnect(ApsAppRequestMessage &request);
		void		OnUndefinedCmd(ApsAppRequestMessage &request);
		void		OnDisconn(ApsAppRequestMessage &request);
		void 		OnDisconnect();
		
		void 		UpdateDisconnMessge(const char *reason);
	
		void 		AddErrorMessage(ApsAppRequestMessage &request,const char *reason);
private:
		ApsApnsClientPool * clientPool;
		PushC2DMClientSocket * PushC2DMSocket;
		int			 keepalive;
		std::string          appid;
		std::string          receiveTime;
		bool   cleansession;
		push_msg_state msg_state;
		push_client_session_state client_state;
		std::string          connectTime;
};
#endif
