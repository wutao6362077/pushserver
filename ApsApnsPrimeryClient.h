#ifndef _APS_APNS_PRIMERY_CLENT_H_
#define _APS_APNS_PRIMERY_CLENT_H_

#include "SslTcpClient.h"
#include "ApsApnsPrimeryRequestMessage.h"
class ApsApnsClientPool;

class ApsApnsPrimeryClient : public SslTcpClient
{
public:
	ApsApnsPrimeryClient(ApsApnsClientPool* pool);
	virtual ~ApsApnsPrimeryClient(void);

	virtual void		OnResponse(StringParser *reqLine, StrPtrLen *content);
	virtual void		OnPublish(ApsApnsPrimeryRequestMessage &publish);
};

#endif

