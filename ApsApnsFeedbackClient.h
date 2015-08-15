#ifndef _APS_APNS_FEEDBACK_CLENT_H_
#define _APS_APNS_FEEDBACK_CLENT_H_
#include <arpa/inet.h>
#include "SslTcpClient.h"
#include "ApsApnsFeedbackMessage.h"
class ApsApnsClientPool;


class ApsApnsFeedbackClient : public SslTcpClient
{
public:
	ApsApnsFeedbackClient(ApsApnsClientPool* pool);
	virtual ~ApsApnsFeedbackClient(void);

	virtual void		OnResponse(ApsApnsFeedbackMessage &response);
	ZQ_Error 			OnProcessRecvMsg(char* pBuffer, UInt32& nLen);
};

#endif

