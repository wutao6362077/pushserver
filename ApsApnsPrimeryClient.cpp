#include "ApsApnsPrimeryClient.h"


ApsApnsPrimeryClient::ApsApnsPrimeryClient(ApsApnsClientPool* pool):SslTcpClient(pool)
{

}

ApsApnsPrimeryClient::~ApsApnsPrimeryClient()
{

}

void ApsApnsPrimeryClient::OnPublish(ApsApnsPrimeryRequestMessage &publish)
{
	char buf[2048]={0};
	uint32_t dataLen = sizeof(buf);
	bool ret = publish.GetRequestMessage(buf,dataLen);
	this->OnSendMessage(buf,dataLen);
}

void ApsApnsPrimeryClient::OnResponse(StringParser *reqLine, StrPtrLen *content)
{

}

