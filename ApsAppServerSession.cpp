#include "ApsAppServerSession.h"
#include "ApsLogger.h"
#include "mongo/db/json.h"
#include <time.h>
#include "ApsStatusSave.h"
//RTK连接会话类
ApsAppSrvSession::ApsAppSrvSession(ApsApnsClientPool * socket,PushC2DMClientSocket *C2DMsocket):ApsLcsWrapper(this)
{
	fTimeoutTask.SetTask(this);
	fTimeoutTask.SetTimeout(300*1000);
    clientPool = socket;
	PushC2DMSocket = C2DMsocket;
	client_state = mosq_cs_new;
	ApsLogger::Debug("ApsAppSrvSession!"); 
}

ApsAppSrvSession::~ApsAppSrvSession()
{
	ApsLogger::Debug("~ApsAppSrvSession!"); 
}

//RTK连接会话类的处理函数，解析来自客户端的数据，更新状态，并处理订阅推送等信息。
void ApsAppSrvSession::OnHttpRequest(HTTPRequest* pRequest)
{
	//1: Refresh receive message time or will timeout and close socket 
   fTimeoutTask.RefreshTimeout();

	//2: Parse the request content message to string
   StrPtrLen *content = fHttpReq.GetContentBody();
   std::string input = content->GetAsCString();
   ApsAppRequestMessage request;
   ApsAppResponseMessage response;
   if(request.ParseRequestMessage(input))//parse request message right
   {
   		receiveTime = ApsMessageStatus::GetInstance()->AddMessage(request, appid,this);
   		time_t timep; 
		time(&timep);
   		Json::Writer *write_tmp = new Json::FastWriter();
		Json::Value receive = request.GetMessage();
		receive["receive_time"] = asctime(localtime(&timep));

	   const char * cmd = request.GetCommand();
	   if(!strcmp(cmd,"connect"))
	   {
		   OnConnect(request);
	   }
	   else if(!strcmp(cmd,"pingreq"))
	   {
	   	   receive["appid"] = appid.c_str();
		   OnPingReq(request);
	   }
	   else if(!strcmp(cmd,"publish"))
	   {
	       receive["appid"] = appid.c_str();
		   OnPublish(request);
	   }
	   else if(!strcmp(cmd,"disconn"))
	   {receive["appid"] = appid.c_str();
		   OnDisconn(request);
	   }
	   else
	   {
	   		receive["appid"] = appid.c_str();   
		   ApsLogger::Debug("Receive undefined command is %s!",cmd); 
		   //printf("Undefined command is %s!\n",cmd);
		   OnUndefinedCmd(request);
	   }
	   	//std::string msg(write_tmp->write(receive).c_str());
		//mongo::BSONObj p = mongo::fromjson(msg);  
		//ApsMongodbLocation::GetInstance()->Add("provider.message",p);
		//delete write_tmp;
		
   }
   else//parse request message error
   {
   		std::string cmd(request.GetCommand());
		response.SetCommand(cmd);
   		response.SetErrorCode(7);
		StrPtrLen type(HTTPBase::sContentTypeJson.Ptr, HTTPBase::sContentTypeJson.Len);
		GenHttpRespMsg(NULL,response.GetResponseMessage(),&type,httpBadRequest);
//		fTimeoutTask.SetTimeout(1);
   }
}
//发布消息
void ApsAppSrvSession::OnPublish(ApsAppRequestMessage &request)
{
	bool response_flag = false;
	ApsAppResponseMessage response;
	
	if(client_state == mosq_cs_connected)
	{
		ApsLogger::Debug("Receive Publish message successful!"); 
		if(request.GetDeviceID().empty())
		{
			MapDID  id;
			MapDIDIter iter;
			ApsRedisLocation::GetInstance()->GetUserToken(request.GetUserID().c_str(),id);
			for(iter= id.begin();iter!=id.end();iter++)
			{	
				if(iter->first.empty())
				{
					continue;
				}

				response_flag = true;
				
				ApsLogger::Debug("Publish to usrid = %s, deviceToken is %s!",request.GetUserID().c_str(),iter->first.c_str()); 
				Json::Writer *write_tmp = new Json::FastWriter();
				//测试发送阻塞
				//for(int i=0;i<500;i++)
				//{
				//	PushSocket->PushNotification(devid.c_str(),write_tmp->write(root["notification"]).c_str());			
				//}
				
				if(!ApsTokenRedisLocation::GetInstance()->IsValidDeviceToken(iter->first.c_str(),iter->second.strDType.c_str()))
				{
					ApsLogger::Debug("Invalid DeviceToken %s!",iter->first.c_str()); 
					continue;
				}
				if(!strcmp(iter->second.strDType.c_str(),"ios"))
				{
					ApsLogger::Debug("Publish to usrid = %s, ios!",request.GetUserID().c_str()); 		
					clientPool->PushNotification(iter->first.c_str(),write_tmp->write(request.GetIOSMessage()).c_str());
					ApsIosMessage::GetInstance()->AddPublishMessage(request,iter->first.c_str(),clientPool->GetMessageID(),this);

				}
				else
				{
					ApsLogger::Debug("Publish to usrid = %s, android!",request.GetUserID().c_str()); 	
					PushC2DMSocket->PushNotification(iter->first.c_str(),write_tmp->write(request.GetAndroidMessage()).c_str());
					//Json::Value android = request.GetAndroidMessage();
					//android["deviceToken"] = iter->first.c_str();
					///android["status"] = "publish";
					//std::string message(write_tmp->write(android).c_str());
					//mongo::BSONObj p = mongo::fromjson(message);  
					//ApsMongodbLocation::GetInstance()->Add("provider.android",p);
				}
				delete write_tmp;
			}
		}
		else
		{
			MapDID  id;
			ApsRedisLocation::GetInstance()->GetDeviceToken(request.GetUserID().c_str(),request.GetDeviceID().c_str(),id);
			ApsLogger::Debug("Publish to usrid = %s, deviceToken is %s!",request.GetUserID().c_str(),id.begin()->first.c_str()); 
			if(!id.begin()->first.empty())
			{
				response_flag = true;
				Json::Writer *write_tmp = new Json::FastWriter();
				//测试发送阻塞
				//for(int i=0;i<500;i++)
				//{
				//	PushSocket->PushNotification(devid.c_str(),write_tmp->write(root["notification"]).c_str());			
				//}
				if(!ApsTokenRedisLocation::GetInstance()->IsValidDeviceToken(id.begin()->first.c_str(),id.begin()->second.strDType.c_str()))
				{
					ApsLogger::Debug("Invalid DeviceToken %s!",id.begin()->first.c_str()); 
				}
				else if(!strcmp(id.begin()->second.strDType.c_str(),"ios"))
				{
					ApsLogger::Debug("Publish to usrid = %s, ios!",request.GetUserID().c_str()); 		
					clientPool->PushNotification(id.begin()->first.c_str(),write_tmp->write(request.GetIOSMessage()).c_str());
					ApsIosMessage::GetInstance()->AddPublishMessage(request,id.begin()->first.c_str(),clientPool->GetMessageID(),this);
				}
				else
				{
					ApsLogger::Debug("Publish to usrid = %s, android!",request.GetUserID().c_str()); 	
					PushC2DMSocket->PushNotification(id.begin()->first.c_str(),write_tmp->write(request.GetAndroidMessage()).c_str());
					//Json::Value android = request.GetAndroidMessage();
					//android["deviceToken"] = id.begin()->first.c_str();
					//android["msgid"] = clientPool->GetMessageID();
					//std::string message(write_tmp->write(android).c_str());
					//mongo::BSONObj p = mongo::fromjson(message);  
					//ApsMongodbLocation::GetInstance()->Add("provider.android",p);
				}
				delete write_tmp;
			}
		}

		if(response_flag)
		{
			std::string cmd("puback");
			response.SetCommand(cmd);
			response.SetSuccessFlag(true);
			response.SetMessageID(request.GetMessageID());

			StrPtrLen type(HTTPBase::sContentTypeJson.Ptr, HTTPBase::sContentTypeJson.Len);
			GenHttpRespMsg(NULL,response.GetResponseMessage(),&type,httpOK);
			
			ApsMessageStatus::GetInstance()->UpdateMessage(receiveTime,appid,"ok",this);
		}
		else//devid is null
		{
		   	std::string cmd(request.GetCommand());
			response.SetCommand(cmd);
   			response.SetErrorCode(12);
			StrPtrLen type(HTTPBase::sContentTypeJson.Ptr, HTTPBase::sContentTypeJson.Len);
			GenHttpRespMsg(NULL,response.GetResponseMessage(),&type,httpForbidden);
			//Json::Writer *write_tmp = new Json::FastWriter();
			//Json::Value receive = request.GetMessage();
			//receive["error"] = "devid is null!";
			//std::string message(write_tmp->write(receive).c_str());
			//mongo::BSONObj p = mongo::fromjson(message);  
			//ApsMongodbLocation::GetInstance()->Add("provider.error",p);
			//delete write_tmp;
			//AddErrorMessage(request,"devid is null!");
			
			ApsMessageStatus::GetInstance()->UpdateMessage(receiveTime,appid,"devid is null",this);
		}
	}
	else
	{
		ApsLogger::Debug("Receive publish while state is not mosq_cs_connected!"); 
		//printf("receive publish while state is not mosq_cs_connected!\n");
		std::string cmd(request.GetCommand());
		response.SetCommand(cmd);
		response.SetErrorCode(8);
		StrPtrLen type(HTTPBase::sContentTypeJson.Ptr, HTTPBase::sContentTypeJson.Len);
		GenHttpRespMsg(NULL,response.GetResponseMessage(),&type,httpForbidden);
//		fTimeoutTask.SetTimeout(1);
		//AddErrorMessage(request,"Receive publish in wrong state!");
		ApsMessageStatus::GetInstance()->UpdateMessage(receiveTime,appid,"Receive publish in wrong state",this);

	}
}
//处理保活消息
void ApsAppSrvSession::OnPingReq(ApsAppRequestMessage &request)
{
	ApsAppResponseMessage response;
	if(client_state == mosq_cs_connected)
	{
		std::string cmd("pingresp");
		response.SetCommand(cmd);
		StrPtrLen type(HTTPBase::sContentTypeJson.Ptr, HTTPBase::sContentTypeJson.Len);
		GenHttpRespMsg(NULL,response.GetResponseMessage(),&type);	
		ApsLogger::Debug("Receive PingReq message successful!"); 
		ApsMessageStatus::GetInstance()->UpdateMessage(receiveTime,appid,"OK",this);

	}
	else
	{
		
		ApsLogger::Debug("Receive pingreq while state is not mosq_cs_connected!"); 
		std::string cmd(request.GetCommand());
		response.SetCommand(cmd);
		response.SetErrorCode(8);
		StrPtrLen type(HTTPBase::sContentTypeJson.Ptr, HTTPBase::sContentTypeJson.Len);
		GenHttpRespMsg(NULL,response.GetResponseMessage(),&type,httpForbidden);
		
		//AddErrorMessage(request,"Receive pingreq while state is not connected!");
		
		ApsMessageStatus::GetInstance()->UpdateMessage(receiveTime,appid,"Receive pingreq while state is not connected",this);
//		fTimeoutTask.SetTimeout(1);
	}
}
//处理连接消息
void ApsAppSrvSession::OnConnect(ApsAppRequestMessage &request)
{
	ApsAppResponseMessage response;

	if(client_state == mosq_cs_new)
	{
		client_state = mosq_cs_rec_connect;
		appid = request.GetAppID();
		cleansession = request.GetCleanFlag();
		keepalive = request.GetKeepalive();
		fTimeoutTask.SetTimeout(keepalive*1000*2);//keepalive time * 2, avoid receive pingreq when disconnect

		std::string cmd("connack");
		response.SetCommand(cmd);
		response.SetSuccessFlag(true);
		StrPtrLen type(HTTPBase::sContentTypeJson.Ptr, HTTPBase::sContentTypeJson.Len);
		GenHttpRespMsg(NULL,response.GetResponseMessage(),&type);
		ApsLogger::Debug("Receive Connect message successful!"); 
		client_state = mosq_cs_connected;
		
   		//time_t timep; 
		//time(&timep);
		//Json::Writer *write_tmp = new Json::FastWriter();
		//Json::Value receive = request.GetMessage();	
		//receive["start_time"] = asctime(localtime(&timep));
		//receive["status"] = true;
		//connectTime = asctime(localtime(&timep));
		//std::string message(write_tmp->write(receive).c_str());
		//mongo::BSONObj p = mongo::fromjson(message);  
		//ApsMongodbLocation::GetInstance()->Add("provider.connect",p);
		//delete write_tmp;
		ApsMessageStatus::GetInstance()->UpdateMessage(receiveTime,appid,"ok",this);
		connectTime = ApsConnectStatus::GetInstance()->AddConnectMessage(request,this);
	}
	else
	{	
		ApsLogger::Debug("Receive connect while state is not mosq_cs_new!"); 
		std::string cmd(request.GetCommand());
		response.SetCommand(cmd);
		response.SetErrorCode(10);

		StrPtrLen type(HTTPBase::sContentTypeJson.Ptr, HTTPBase::sContentTypeJson.Len);
		GenHttpRespMsg(NULL,response.GetResponseMessage(),&type,httpForbidden);

		//AddErrorMessage(request,"Receive connect while the state is connected!");
		
		ApsMessageStatus::GetInstance()->UpdateMessage(receiveTime,appid,"error",this);
//		fTimeoutTask.SetTimeout(1);
	}
}
//未定义消息处理
void ApsAppSrvSession::OnUndefinedCmd(ApsAppRequestMessage &request)
{
	ApsAppResponseMessage response;

	std::string cmd(request.GetCommand());
	response.SetCommand(cmd);
	response.SetErrorCode(6);

	StrPtrLen type(HTTPBase::sContentTypeJson.Ptr, HTTPBase::sContentTypeJson.Len);
	GenHttpRespMsg(NULL,response.GetResponseMessage(),&type,httpForbidden);

	//AddErrorMessage(request,"Receive undefined command!");
	ApsMessageStatus::GetInstance()->UpdateMessage(receiveTime,appid,"error",this);
//	fTimeoutTask.SetTimeout(1);
}
//断开消息处理
void ApsAppSrvSession::OnDisconn(ApsAppRequestMessage &request)
{
	if(client_state == mosq_cs_connected)
	{
		ApsLogger::Debug("Receive Disconn message successful!"); 	
		fTimeoutTask.SetTimeout(1);
		client_state = mosq_cs_disconnecting;
		//UpdateDisconnMessge("receive client disconnect messgage");
		ApsConnectStatus::GetInstance()->UpdateConnectStatus( receiveTime,appid,"Receive Disconn message",this);
		ApsMessageStatus::GetInstance()->UpdateMessage(receiveTime,appid,"ok",this);
	}
	else
	{	
		ApsLogger::Debug("Receive disconnect while the state is not mosq_cs_connected!");
		//AddErrorMessage(request,"Receive disconnect while the state is not mosq_cs_connected!");
		//printf("receive disconnect while the state is not mosq_cs_connected!\n");
		
		ApsMessageStatus::GetInstance()->UpdateMessage(receiveTime,appid,"error",this);
	}
}
//server disconnet
void ApsAppSrvSession::OnDisconnect()
{
	//UpdateDisconnMessge("server disconnect");
	ApsConnectStatus::GetInstance()->UpdateConnectStatus( receiveTime,appid,"server disconnect",this);
	ApsLogger::Debug("server disconnect connection!");
}

void ApsAppSrvSession::UpdateDisconnMessge(const char *reason)
{
	time_t timep; 
	time(&timep);
	
	Json::Writer *write_tmp = new Json::FastWriter();
	Json::Value find;
	find["appid"] = appid.c_str();
	find["start_time"] = connectTime.c_str();
	find["status"] = true;
	std::string message(write_tmp->write(find).c_str());
	mongo::Query query(message); 
	char cmd[1024];
	Json::Value up;
	up["disconnect_time"] = asctime(localtime(&timep));
	up["disconnet_message"] = reason;
	up["status"] = false;
	message = write_tmp->write(up).c_str();
	snprintf(cmd,sizeof(cmd),"{$set:%s}",message.c_str());
	mongo::BSONObj update = mongo::fromjson(cmd);  
	
	//ApsMongodbLocation::GetInstance()->Update("provider.connect",query,update); 
	delete write_tmp;

}

void ApsAppSrvSession::AddErrorMessage(ApsAppRequestMessage &request,const char *reason)
{
	Json::Writer *write_tmp = new Json::FastWriter();
	Json::Value receive = request.GetMessage();
	receive["error"] = reason;
	std::string message(write_tmp->write(receive).c_str());
	mongo::BSONObj p = mongo::fromjson(message);  
	//ApsMongodbLocation::GetInstance()->Add("provider.error",p);
}

