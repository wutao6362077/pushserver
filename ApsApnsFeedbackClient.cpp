#include "ApsApnsFeedbackClient.h"

ApsApnsFeedbackClient::ApsApnsFeedbackClient(ApsApnsClientPool* pool):SslTcpClient(pool)
{
}

ApsApnsFeedbackClient::~ApsApnsFeedbackClient()
{

}

void ApsApnsFeedbackClient::OnResponse(ApsApnsFeedbackMessage &response)
{
	ApsLogger::Debug("Get feedback = %d.",response.GetDeviceToken());
}

ZQ_Error	ApsApnsFeedbackClient::OnProcessRecvMsg(char* pBuffer, UInt32& nLen)
{
	if(pBuffer[4]!=0||pBuffer[5]!=32)
	{
		return ZQ_BadIndex;
	}
	ApsApnsFeedbackMessage feedbackMessage;
	feedbackMessage.SetTime(ntohl((uint32_t)pBuffer[0]));
	feedbackMessage.SetDeviceToken(&pBuffer[6]);
	OnResponse(feedbackMessage);
	return OS_NoErr;
}

