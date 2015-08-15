#ifndef _APS_APNS_ADVANCE_CLENT_H_
#define _APS_APNS_ADVANCE_CLENT_H_
#include <arpa/inet.h>
#include "SslTcpClient.h"
#include "ApsApnsPrimeryRequestMessage.h"
#include "ApsApnsAdvanceResponseMessage.h"
class ApsApnsClientPool;
class ApsApnsAdvanceClient : public SslTcpClient
{
public:
	ApsApnsAdvanceClient(ApsApnsClientPool* pool);
	virtual ~ApsApnsAdvanceClient(void);

	virtual void		OnResponse(ApsApnsAdvanceResponseMessage response);
	virtual void		OnPublish(ApsApnsPrimeryRequestMessage &publish);
	ZQ_Error 			OnProcessRecvMsg(char* pBuffer, UInt32& nLen);
	void 				OnTimeout();
};

#endif

