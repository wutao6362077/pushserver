#include "SslSvrSession.h"
#include "MqttSessionHandler.h"

#include "ApsLogger.h"
SslSvrSession::SslSvrSession(int fd,struct sockaddr_in *addr,SSL_CTX *m_pctx)
:	TCPSslSessionInterface(fd,addr),
	fState(kReadingRequest)
{
	fInputStream.ShowTCP(true);

	ListZero(&fMsgList);

	// 创建当前连接的SSL结构体
	m_pssl = SSL_new(m_pctx);
	
	// 将SSL绑定到套接字上
	SSL_set_fd(m_pssl, fd);
	
//    SSL_set_verify(m_pssl, SSL_VERIFY_PEER,NULL);	
	// 接受SSL连接
	SSL_accept(m_pssl);
	
	// 获取和释放客户端证书
	// 这一步是可选的
	m_pserver_cert = SSL_get_peer_certificate(m_pssl);
	appId = "1234567890";
	X509_free(m_pserver_cert);
}

SslSvrSession::~SslSvrSession(void)
{
	//this->CleanupRequest();// Make sure that all our objects are deleted

	OSMutexLocker locker(&fMutexList);
	ListEmpty(&fMsgList);
	if(m_pssl != NULL)
	{
		SSL_free(m_pssl);
	}
}
/*
OS_Error SslSvrSession::GenHttpRespMsg(StrPtrLen* cmdParams, const char* content, StrPtrLen *contentType, HTTPStatusCode status)
{
	HTTPResponse	httpResponse;
	UInt32 contentLen = content==NULL?0:strlen(content);
	httpResponse.CreateResponseHeader(http11Version, status);
	httpResponse.AppendDateField();
	if(cmdParams && cmdParams->Len > 0)
	{
		httpResponse.AppendResponseHeader(httpPragmaHeader, cmdParams);
	}
	if(contentLen > 0)
	{
		httpResponse.AppendResponseHeader(httpContentTypeHeader, contentType==NULL?&HTTPBase::sContentTypeText:contentType);
		httpResponse.AppendContentLengthHeader(contentLen);
	}

	StrPtrLen* responseHeader = httpResponse.GetCompleteResponseHeader();

	PutResponse(responseHeader->Ptr, responseHeader->Len);
	if(contentLen > 0)
	{
		PutResponse(content, contentLen);
	}

	return OS_NoErr;
}
*/
SInt64 SslSvrSession::Run()
{
    EventFlags events = this->GetEvents();
	this->ForceSameThread();

	// Http session is short connection, need to kill session when occur TimeoutEvent.
	// So return -1.
	if(events&Task::kTimeoutEvent || events&Task::kKillEvent)
		return -1;

    ZQ_Error err = ZQ_NoErr;
    
    while (this->IsLiveSession())
    {
        // HTTP Session state machine. There are several well defined points in an HTTP request
        // where this session may have to return from its run function and wait for a new event.
        // Because of this, we need to track our current state and return to it.
		if(events&Task::kReadEvent)
		{
			switch(fState)
			{
			case kReadingRequest:
				{
					if ((err = fInputStream.ReadRequest(m_pssl)) == ZQ_NoErr)
					{
						// If the RequestStream returns QTSS_NoErr, it means
						// that we've read all outstanding data off the socket,
						// and still don't have a full request. Wait for more data.
	                    
						//+rt use the socket that reads the data, may be different now.
						fInputSocketP->RequestEvent(EV_RE);
						events -= Task::kReadEvent;
						break; 
					}

					if ((err != ZQ_RequestArrived) && (err != E2BIG))
					{
						// Any other error implies that the client has gone away. At this point,
						// we can't have 2 sockets, so we don't need to do the "half closed" check
						// we do below
						Assert(err > 0); 
	//					Assert(!this->IsLiveSession());
						events -= Task::kReadEvent; 
						break; 
					}

					if (err == ZQ_RequestArrived)
						fState = kProcessingRequest;
					// If we get an E2BIG, it means our buffer was overfilled.
					// In that case, we can just jump into the following state, and
					// the code their does a check for this error and returns an error.
					if (err == E2BIG)
						fState = kCleaningUp;
				}
			case kProcessingRequest:
				{
					char* pBuffer = fInputStream.GetRequestBuffer();
					UInt32 nLen = fInputStream.GetRequsetLength();
					ZQ_Error err = ZQ_NoErr;
					while(err == ZQ_NoErr && nLen > 0)
					{
						err = ProcessRecvMsg(pBuffer, nLen);
					}

					if(ZQ_NoErr == err)
					{
						fState = kReadingRequest;
						return 0;
						// Update timer.
						//fTimeoutTask.RefreshTimeout();
					} 
					else
					{
						fState = kCleaningUp;
					}
				}
				break;
			case kCleaningUp:
				{  
					// If we've gotten here, we've flushed all the data. Cleanup,
					// and wait for our next request!
					this->CleanupRequest();
					fState = kReadingRequest;
				}
				break;
			};
		}
		else if(events&Task::kWriteEvent)
		{
			OSMutexLocker locker(&fMutexList); 
			ListElement *elem = NULL; 
			int theLengthSent = 0;
			if((elem = fMsgList.first) != NULL) 
			{ 
//				err = fSocket.Send((const void*)elem->content, elem->size, &theLengthSent);
				if (err == EAGAIN)
				{
					fSocket.RequestEvent(EV_RE | EV_WR);
				}
				else
				{
					ListRemoveHead(&fMsgList); 
					if(NULL != fMsgList.first)
						this->Signal(kWriteEvent);
				}
				
			}
			events -= Task::kWriteEvent;
		} 
		else if(events&Task::kZmqEvent)
		{	
			events -= Task::kZmqEvent;
		}
		else
		{
			return 0;
		}
	}
    
    // Make absolutely sure there are no resources being occupied by the session
    // at this point.
    this->CleanupRequest();

    // Only delete if it is ok to delete!
    if (fObjectHolders == 0)
        return -1;

    // If we are here because of a timeout, but we can't delete because someone
    // is holding onto a reference to this session, just reschedule the timeout.
    //
    // At this point, however, the session is DEAD.
    return 0;
}

ZQ_Error SslSvrSession::ProcessRecvMsg(char* pBuffer, UInt32& nLen)
{
	int rc = 0;
	ProviderPacket packet;
	ApsLogger::Debug("ProcessRecvMsg:nLen = %d!",nLen);  
	if(pBuffer == NULL || nLen <= 35)
	{
		ApsLogger::Debug("ProcessRecvMsg:param error!");	
		return MQTT_ERR_PROTOCOL;
	}
	
	char * buf = pBuffer;
	int buf_len = nLen;
	uint8_t cmd;
	uint8_t msb,lsb;
	uint16_t tokenLen;
	uint16_t payloadLen;
	cmd = *(buf++);
	packet.SetCommand(cmd);
	msb = *(buf++);
	lsb = *(buf++);
	tokenLen = (msb<<8) + lsb;
	packet.SetTokenLength(tokenLen);
	packet.SetToken((const char *)buf,tokenLen);
	buf += tokenLen;
	msb = *(buf++);
	lsb = *(buf++);
	payloadLen = (msb<<8) + lsb;
	if(nLen< payloadLen+35)
	{
		ApsLogger::Debug("ProcessRecvMsg:param error!");	
		return MQTT_ERR_PROTOCOL;
	}
	packet.SetPayloadLength(payloadLen);
	packet.SetPayload((const char *)buf,payloadLen);
	buf += payloadLen;

	rc = PacketHandle(packet);
	if(rc == MQTT_ERR_SUCCESS)
	{
		nLen -= payloadLen+2+tokenLen+2+1;
		if(nLen > 0)
        {
			memmove(pBuffer, buf, nLen);
            memset(pBuffer + nLen, 0, buf_len - nLen);
        }
	}
	
	return rc;
}
int SslSvrSession::PacketHandle(ProviderPacket &packet)
{
	int ret = 0;
	std::string token = packet.GetToken();
	bool parsingSuccessful = reader.parse( packet.GetPayload(), root );
	if ( !parsingSuccessful )//The message is not json
    {
   		ApsLogger::Debug("PacketHandle: Failed to parse request message!"); 
		return ret;
    }
    else//pares json message ok
    {
    	root["appid"] = appId;
    }

	MqttServer * session = MqttSessionHandler::GetInstance()->FindMqttSession(token);
	if(session != NULL)
	{
		Json::Writer *write_tmp = new Json::FastWriter();	
   		ApsLogger::Debug("PacketHandle: send message = %s!",write_tmp->write(root).c_str()); 
		session->mqtt_send_publish(1,write_tmp->write(root).length(),write_tmp->write(root).c_str(),0,false,false);
		delete write_tmp;
	}
	else//save the push message for send when the android device is connect to the push server
	{

	}
	return ret;
}

void SslSvrSession::CleanupRequest()
{
	char theDumpBuffer[kHttpBufferSize];
    
    ZQ_Error theErr = ZQ_NoErr;
    while (theErr == ZQ_NoErr)
        theErr = this->Read(m_pssl,theDumpBuffer, kHttpBufferSize, NULL);
}

//RTK连接会话类的处理函数，解析来自客户端的数据，更新状态，并处理订阅推送等信息。
/*void SslSvrSession::OnHttpRequest(HTTPRequest* pRequest)
{
	//1: Refresh receive message time or will timeout and close socket 
   fTimeoutTask.RefreshTimeout();

	//2: Get the request content message to string
   StrPtrLen *content = fHttpReq.GetContentBody();
   std::string input = content->GetAsCString();
	//3: Parse the request string message
   bool parsingSuccessful = reader.parse( input, root );
   if ( !parsingSuccessful )//The message is not json
   {
   		ApsLogger::Debug("Failed to parse request message!"); 
		//printf( "Failed to parse request message!\n");
		char message_body[256] = {0};
		qtss_snprintf(message_body,sizeof(message_body) -1, "{\"errno\": %d,\"message\": \"%s\"}", PushClientSession::sStatusCodes[6],PushClientSession::sStatusCodeStrings[6].Ptr);
		StrPtrLen type(HTTPBase::sContentTypeJson.Ptr, HTTPBase::sContentTypeJson.Len);
		GenHttpRespMsg(NULL,message_body,&type,httpBadRequest);	
		fTimeoutTask.SetTimeout(1);
   }
   else//pares json message ok
   {
		//4: Get the command string and match 
		std::string command = root["name"].asString();
		const char * cmd = command.c_str();
		if(!strcmp(cmd,"connect"))
		{
			OnConnect(root);
		}
		else if(!strcmp(cmd,"pingreq"))
		{
			OnPingReq(root);
		}
		else if(!strcmp(cmd,"publish"))
		{
			OnPublish(root);
		}
		else if(!strcmp(cmd,"disconn"))
		{
			OnDisconn(root);
		}
		else
		{		
			ApsLogger::Debug("Receive undefined command is %s!",cmd); 
			//printf("Undefined command is %s!\n",cmd);
			OnUndefinedCmd(root);
		}  
   }
  
}
*/

