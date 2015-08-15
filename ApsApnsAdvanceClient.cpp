#include "ApsApnsAdvanceClient.h"
#include "ApsApnsClientPool.h"
#include "ApsMongodbLocation.h"
#include "ApsStatusSave.h"
ApsApnsAdvanceClient::ApsApnsAdvanceClient(ApsApnsClientPool* pool):SslTcpClient(pool)
{

}

ApsApnsAdvanceClient::~ApsApnsAdvanceClient()
{

}

void ApsApnsAdvanceClient::OnPublish(ApsApnsPrimeryRequestMessage &publish)
{
	char buf[2048]={0};
	uint32_t dataLen = sizeof(buf);
	bool ret = publish.GetRequestMessage(buf,dataLen);
	this->OnSendMessage(buf,dataLen);
}

void ApsApnsAdvanceClient::OnResponse(ApsApnsAdvanceResponseMessage response)
{
	ApsLogger::Debug("Get response = %d.",response.GetId());
/*	char buf[2048]={0};
	snprintf(buf,sizeof(buf),"{\"msgid\":{\"$gte\":%d}}",response.GetId());
	mongo::Query q(buf);  
	std::auto_ptr<mongo::DBClientCursor> message = ApsMongodbLocation::GetInstance()->Find("provider.ios",&q);	
	mongo::BSONObj doc = message->next();
	while(message->more())
	{
		std::string id = mongo::tojson(doc);
		ApsLogger::Debug("Get mongodb = %s.",id.c_str());
		doc = message->next();
	}*/
	std::string start_time;
	ApsIosMessage::GetInstance()->FindPublishMessage(start_time,response.GetId(),m_clientPool);
}

ZQ_Error	ApsApnsAdvanceClient::OnProcessRecvMsg(char* pBuffer, UInt32& nLen)
{
	ApsApnsAdvanceResponseMessage advanceResponseMessage;
	if(pBuffer[0] != 8)
	{
		return ZQ_BadIndex;
	}
	advanceResponseMessage.SetCommand(pBuffer[0]);
	advanceResponseMessage.SetStatus(pBuffer[1]);
	advanceResponseMessage.SetId(ntohl(*((uint32_t *)&pBuffer[2])));
	nLen -= 6;
	OnResponse(advanceResponseMessage);
	return OS_NoErr;
}

void ApsApnsAdvanceClient::OnTimeout()
{
	SslTcpClient::OnTimeout();
	m_clientPool->ReInitializeParam();
	m_clientPool->fQueue.EnQueue(&fTaskQueueElem);
	m_clientPool->Signal(Task::kConnectEvent);
	
	ApsLogger::Debug("ApsApnsAdvanceClient::OnTimeout.");		
}

