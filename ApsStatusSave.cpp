#include "ApsLogger.h"
#include "ApsMongodbLocation.h"
#include "ApsStatusSave.h"
#include "mongo/db/json.h"
#include <time.h>
#include "OS.h"

ApsMessageStatus* ApsMessageStatus::m_messgageStatus = NULL;

ApsMessageStatus* ApsMessageStatus::GetInstance(void)
{
	if (m_messgageStatus == NULL)
	{
		ApsLogger::Info("New ApsMessageStatus!");
		m_messgageStatus = new ApsMessageStatus();
	}
	
	return m_messgageStatus;
}

ApsMessageStatus::ApsMessageStatus(void)
{

}

ApsMessageStatus::~ApsMessageStatus(void)
{

}
//return the time of save
std::string ApsMessageStatus::AddMessage(ApsAppRequestMessage &request,std::string appid,ApsLcsWrapper * wrapper)
{
	time_t timep; 
	time(&timep);
	Json::Writer *write_tmp = new Json::FastWriter();
	Json::Value receive = request.GetMessage();
	receive["receive_time"] = asctime(localtime(&timep));
	receive["flag"] = false;
	receive["receive_milsecond"] = (int)(OS::Milliseconds()%1000);
	const char * cmd = request.GetCommand();
	if(strcmp(cmd,"connect"))
	{	
		receive["appid"] = appid.c_str();
	}
	std::string msg(write_tmp->write(receive).c_str());
	mongo::BSONObj p = mongo::fromjson(msg); 
	ApsLcsQuery* pQuery = new ApsLcsQuery("Add", wrapper);
	pQuery->SetCollection("provider.message");
	pQuery->SetUpdate(p);
	LCS::Instance().RequestQuery(pQuery);
	//ApsMongodbLocation::GetInstance()->Add("provider.message",p);
	return receive["receive_time"].asString();
}

void ApsMessageStatus::UpdateMessage(std::string recTime,std::string appid,const char *status,ApsLcsWrapper * wrapper)
{
	time_t timep; 
	time(&timep);
	
	Json::Writer *write_tmp = new Json::FastWriter();
	Json::Value find;
	find["appid"] = appid.c_str();
	find["receive_time"] = recTime.c_str();
	find["flag"] = false;
	std::string message(write_tmp->write(find).c_str());
	mongo::Query query(message); 
	char cmd[1024];
	Json::Value up;
	up["deal_time"] = asctime(localtime(&timep));
	up["deal_milsecond"] = (int)(OS::Milliseconds()%1000);
	up["status"] = status;
	up["flag"] = true;
	message = write_tmp->write(up).c_str();
	snprintf(cmd,sizeof(cmd),"{$set:%s}",message.c_str());
	mongo::BSONObj update = mongo::fromjson(cmd);  
	ApsLcsQuery* pQuery = new ApsLcsQuery("Update", wrapper);
	pQuery->SetCollection("provider.message");
	pQuery->SetQuery(query);
	pQuery->SetUpdate(update);
	LCS::Instance().RequestQuery(pQuery);
	//ApsMongodbLocation::GetInstance()->Update("provider.message",query,update); 
	delete write_tmp;
}

ApsErrorMessage* ApsErrorMessage::m_errorMessgage = NULL;

ApsErrorMessage* ApsErrorMessage::GetInstance(void)
{
	if (m_errorMessgage == NULL)
	{
		if (m_errorMessgage == NULL)
		{	
			ApsLogger::Info("New ApsErrorMessage!");
			m_errorMessgage = new ApsErrorMessage();
		}
	}
	
	return m_errorMessgage;
}

ApsErrorMessage::ApsErrorMessage(void)
{

}

ApsErrorMessage::~ApsErrorMessage(void)
{

}

void ApsErrorMessage::AddErrorMessage(ApsAppRequestMessage &request,const char *status,ApsLcsWrapper * wrapper)
{
	Json::Writer *write_tmp = new Json::FastWriter();
	Json::Value receive = request.GetMessage();
	receive["error"] = status;
	std::string message(write_tmp->write(receive).c_str());
	mongo::BSONObj p = mongo::fromjson(message);  
	//ApsMongodbLocation::GetInstance()->Add("provider.error",p);
	
	ApsLcsQuery* pQuery = new ApsLcsQuery("Add", wrapper);
	pQuery->SetCollection("provider.error");
	pQuery->SetUpdate(p);
	LCS::Instance().RequestQuery(pQuery);

	delete write_tmp;
}

ApsConnectStatus* ApsConnectStatus::m_connectStatus = NULL;

ApsConnectStatus* ApsConnectStatus::GetInstance(void)
{
	if (m_connectStatus == NULL)
	{
		if (m_connectStatus == NULL)
		{	
			ApsLogger::Info("New ApsConnectStatus!");
			m_connectStatus = new ApsConnectStatus();
		}
	}
	
	return m_connectStatus;
}

ApsConnectStatus::ApsConnectStatus(void)
{

}

ApsConnectStatus::~ApsConnectStatus(void)
{

}
//return the connect time
std::string ApsConnectStatus::AddConnectMessage(ApsAppRequestMessage &request,ApsLcsWrapper * wrapper)
{
	time_t timep; 
	time(&timep);
	Json::Writer *write_tmp = new Json::FastWriter();
	Json::Value receive = request.GetMessage(); 
	receive["start_time"] = asctime(localtime(&timep));
	receive["receive_milsecond"] = (int)(OS::Milliseconds()%1000);
	receive["status"] = true;
	std::string message(write_tmp->write(receive).c_str());
	mongo::BSONObj p = mongo::fromjson(message);  
	//ApsMongodbLocation::GetInstance()->Add("provider.connect",p);
	ApsLcsQuery* pQuery = new ApsLcsQuery("Add", wrapper);
	pQuery->SetCollection("provider.connect");
	pQuery->SetUpdate(p);
	LCS::Instance().RequestQuery(pQuery);
	delete write_tmp;
	return receive["start_time"].asString();
}

void ApsConnectStatus::UpdateConnectStatus(std::string conTime,std::string appid,const char *status,ApsLcsWrapper * wrapper)
{
	time_t timep; 
	time(&timep);
	
	Json::Writer *write_tmp = new Json::FastWriter();
	Json::Value find;
	find["appid"] = appid.c_str();
	find["start_time"] = conTime.c_str();
	find["status"] = true;
	std::string message(write_tmp->write(find).c_str());
	mongo::Query query(message); 
	char cmd[1024];
	Json::Value up;
	up["disconnect_time"] = asctime(localtime(&timep));
	up["deal_milsecond"] = (int)(OS::Milliseconds()%1000);
	up["disconnet_message"] = status;
	up["status"] = false;
	message = write_tmp->write(up).c_str();
	snprintf(cmd,sizeof(cmd),"{$set:%s}",message.c_str());
	mongo::BSONObj update = mongo::fromjson(cmd);  
	ApsLcsQuery* pQuery = new ApsLcsQuery("Update", wrapper);
	pQuery->SetCollection("provider.connect");
	pQuery->SetQuery(query);
	pQuery->SetUpdate(update);
	LCS::Instance().RequestQuery(pQuery);
	//ApsMongodbLocation::GetInstance()->Update("provider.connect",query,update); 
	delete write_tmp;
}

ApsIosMessage* ApsIosMessage::m_iosMessage = NULL;

ApsIosMessage* ApsIosMessage::GetInstance(void)
{
	if (m_iosMessage == NULL)
	{	
		ApsLogger::Info("New ApsIosMessage!");
		m_iosMessage = new ApsIosMessage();
	}
	
	return m_iosMessage;
}

ApsIosMessage::ApsIosMessage(void)
{

}

ApsIosMessage::~ApsIosMessage(void)
{

}
//return the receive time
std::string ApsIosMessage::AddPublishMessage(ApsAppRequestMessage &request,std::string token,uint16_t mid,ApsLcsWrapper * wrapper)
{
	time_t timep; 
	time(&timep);
	Json::Writer *write_tmp = new Json::FastWriter();
	Json::Value ios = request.GetIOSMessage();
	ios["deviceToken"] = token;
	ios["receive_time"] = asctime(localtime(&timep));
	ios["receive_milsecond"] = (int)(OS::Milliseconds()%1000);
	ios["msgid"] = mid;
	std::string message(write_tmp->write(ios).c_str());
	mongo::BSONObj p = mongo::fromjson(message);  
	//ApsMongodbLocation::GetInstance()->Add("provider.ios",p);
	ApsLcsQuery* pQuery = new ApsLcsQuery("Add", wrapper);
	pQuery->SetCollection("provider.ios");
	pQuery->SetUpdate(p);
	LCS::Instance().RequestQuery(pQuery);

	return ios["receive_time"].asString();
}

void ApsIosMessage::FindPublishMessage(std::string startTime,uint16_t mid,ApsLcsWrapper * wrapper)
{
	char buf[2048]={0};
	if(startTime.empty())
		snprintf(buf,sizeof(buf),"{msgid:{$gte:%d}}",mid);
	else
		snprintf(buf,sizeof(buf),"{msgid:{$gte:%d},startTime:%s}",mid,startTime.c_str());
		
	mongo::Query q(buf);  
	ApsLcsQuery* pQuery = new ApsLcsQuery("Find", wrapper);
	pQuery->SetCollection("provider.ios");
	pQuery->SetQuery(q);
	LCS::Instance().RequestQuery(pQuery);
}

