#include "MqttServer.h"
#include "ApsLogger.h"
#include "MqttSessionHandler.h"
MqttServer::MqttServer()
{
	ApsLogger::Debug("MqttServer");
	connectStatus = MQTT_INIT_NEW;
	
}

MqttServer::~MqttServer()
{
	ApsLogger::Debug("~MqttServer");  
}

SInt64 MqttServer::Run()
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
			if ((err = fInputStream.ReadRequest()) == ZQ_NoErr)
			{
				//+rt use the socket that reads the data, may be different now.
				fInputSocketP->RequestEvent(EV_RE);
				events -= Task::kReadEvent;
				continue; 
			}
				
			if ((err != ZQ_RequestArrived) && (err != E2BIG))
			{
				// Any other error implies that the client has gone away. At this point,
				// we can't have 2 sockets, so we don't need to do the "half closed" check
				// we do below
				Assert(err > 0); 
				Assert(!this->IsLiveSession());
				events -= Task::kReadEvent; 
				break; 
			}
				
			if (err == ZQ_RequestArrived)
			{
				char* pBuffer = fInputStream.GetRequestBuffer();
				int nLen = fInputStream.GetRequsetLength();
				ZQ_Error err = ZQ_NoErr;
				while(err == ZQ_NoErr && nLen > 0)
				{
					err = ProcessRecvMsg(pBuffer, nLen);
				}
				
				if(ZQ_NoErr == err)
				{
					err = fOutputStream.Flush();
					if (err == EAGAIN)
					{
						UInt32 datalen = 0;
						char* pData = fOutputStream.GetBufferCopy(&datalen);
						if(pData)
						{
							OSMutexLocker locker(&fMutexList); 
							ListAppend(&fMsgList, pData, datalen);
						}
						// If we get this error, we are currently flow-controlled and should
						// wait for the socket to become writeable again
						fSocket.RequestEvent(EV_RE | EV_WR);
					}
				}
				else
				{
					CleanupRequest();
				}
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

ZQ_Error MqttServer::ProcessRecvMsg(char* pBuffer, int& nLen)
{
	int rc = 0;
	MqttPacket packet;
	ApsLogger::Debug("ProcessRecvMsg:nLen = %d!",nLen);  
	if(pBuffer == NULL || nLen == 0)
	{
		ApsLogger::Debug("ProcessRecvMsg:param error!");	
		return MQTT_ERR_PROTOCOL;
	}
	
	char * buf = pBuffer;
	int buf_len = nLen;
	/* Clients must send CONNECT as their first command. */
	if(connectStatus == MQTT_INIT_NEW&& (*buf&0xF0) != CONNECT)
	{	
		ApsLogger::Debug("ProcessRecvMsg:Please connect first!");	
		return MQTT_ERR_PROTOCOL;
	}
	
	packet.SetCommand(*buf++);
	unsigned char remaining_count = 0;
	unsigned int remaining_mult = 1;
	unsigned int  remaining_length = 0;
	do
	{
		remaining_count++;
		/* Max 4 bytes length for remaining length as defined by protocol.
		* Anything more likely means a broken/malicious client.
		*/
		if(remaining_count > 4)
		{	
			ApsLogger::Debug("ProcessRecvMsg:message to long!");	
			return MQTT_ERR_PROTOCOL;
		}
	
		remaining_length += (*buf & 127) * remaining_mult;
		remaining_mult *= 128;
	}while(((*buf++ & 128) != 0)&&(buf-pBuffer<=nLen));

	if((remaining_length > 0)&&(remaining_length+remaining_count+1<=nLen))
	{
		packet.SetReceivePayload(buf,remaining_length);
		buf += remaining_length;
	}
	
	rc = mqtt_packet_handle(packet);
	if(rc == MQTT_ERR_SUCCESS)
	{
		nLen -= remaining_length+remaining_count+1;
		if(nLen > 0)
        {
			memmove(pBuffer, buf, nLen);
            memset(pBuffer + nLen, 0, buf_len - nLen);
        }
	}
	
	return rc;
}

int MqttServer::mqtt_packet_handle(class MqttPacket &packet)
{
	ApsLogger::Debug("mqtt_packet_handle");  
	switch((packet.GetCommand())&0xF0){
		case PINGREQ:
			return mqtt_handle_pingreq(packet);
		case CONNECT:
			return mqtt_handle_connect(packet);
		case DISCONNECT:
			return mqtt_handle_disconnect(packet);
		default:
			/* If we don't recognise the command, return an error straight away. */
			return MQTT_ERR_PROTOCOL;
	}
}

int MqttServer::mqtt_handle_connect(class MqttPacket &packet)
{
	ApsLogger::Debug("mqtt_handle_connect");  
	fTimeoutTask.RefreshTimeout();

	/* Don't accept multiple CONNECT commands. */
	if(connectStatus != MQTT_INIT_NEW)
	{
		ApsLogger::Debug("mqtt_handle_connect not init status %d",connectStatus);  
		mqtt_server_disconnect();
		return MQTT_ERR_PROTOCOL;
	}

	if(packet.ReadString(protocolName))
	{	
		ApsLogger::Debug("mqtt_handle_connect connot read protocol name");  
		mqtt_send_connack(MQTT_ERR_PROTOCOL);
		mqtt_server_disconnect();
		return 1;
	}
	if(protocolName.empty())
	{
		ApsLogger::Debug("mqtt_handle_connect protocol name is empty");	
		mqtt_send_connack(MQTT_ERR_PROTOCOL);
		mqtt_server_disconnect();
		return 3;
	}

	if(strcmp(protocolName.c_str(), "MQIsdp"))
	{
		ApsLogger::Debug("mqtt_handle_connect protocol name is not value");	
		mqtt_send_connack(MQTT_ERR_PROTOCOL);
		mqtt_server_disconnect();
		return MQTT_ERR_PROTOCOL;
	}

	if(packet.ReadUint8(protocolVersion))
	{
		ApsLogger::Debug("mqtt_handle_connect connot read protocolVersion");	
		mqtt_send_connack(MQTT_ERR_INVAL);		
		mqtt_server_disconnect();
		return 1;
	}

	if(packet.ReadUint8(flag))
	{
		ApsLogger::Debug("mqtt_handle_connect connot read flag");	
		mqtt_send_connack(MQTT_ERR_INVAL);		
		mqtt_server_disconnect();
		return 1;
	}
		
	if(packet.ReadUint16(keepAlive))
	{
		ApsLogger::Debug("mqtt_handle_connect connot read keepalive");	
		mqtt_send_connack(MQTT_ERR_INVAL);		
		mqtt_server_disconnect();
		return 1;
	}
	
	fTimeoutTask.SetTimeout(keepAlive*2000);
/*	
	if(packet.ReadString(clientId))
	{
		ApsLogger::Debug("mqtt_handle_connect connot read clientid\n");		
		mqtt_server_disconnect();
		return 1;
	}
*/
	std::string input = packet.GetJsonMsg();
	 //3: Parse the request string message
	bool parsingSuccessful = reader.parse( input, root );
	if ( !parsingSuccessful )//The message is not json
	{
		  ApsLogger::Debug("Failed to parse request message!"); 
		  mqtt_send_connack(MQTT_ERR_INVAL);	  
		  mqtt_server_disconnect();
		  return MQTT_ERR_PROTOCOL;
	}

	clientId = root["username"].asString();
	if(clientId.length() != 16)
	{
		ApsLogger::Debug("mqtt_handle_connect client_id is not value!");	
		mqtt_send_connack(MQTT_ERR_CONN_REFUSED);
		mqtt_server_disconnect();
		return MQTT_ERR_PROTOCOL;
	}
	//because the devicetoken is 32bytes
	clientId += root["username"].asString();
	ApsLogger::Debug("uuid is %s!",clientId.c_str());	
    //add store to redis message
	ApsLogger::Debug("mqtt_handle_connect sucessful!");	    
	connectStatus = MQTT_CONNECTED;
	MqttSessionHandler::GetInstance()->AddMqttSession(clientId,this);
	return mqtt_send_connack(MQTT_ERR_SUCCESS);
}

int MqttServer::mqtt_handle_disconnect(class MqttPacket &packet)
{
/*	if(packet.GetRemainingLength() != 0)
	{	
		ApsLogger::Debug("mqtt_handle_disconnect protocol error!");
		return MQTT_ERR_PROTOCOL;
	}
*/
	ApsLogger::Debug("mqtt_handle_disconnect!");
	connectStatus = MQTT_DISCONNECTING;	
	mqtt_server_disconnect();
	return MQTT_ERR_SUCCESS;
}

int MqttServer::mqtt_handle_pingreq(class MqttPacket &packet)
{
/*	if(packet.GetRemainingLength() != 0)
	{	
		ApsLogger::Debug("mqtt_handle_pingreq protocol error!");
		return MQTT_ERR_PROTOCOL;
	}
*/
	fTimeoutTask.RefreshTimeout();

	int rc;

	char buf[2];
	buf[0] = PINGRESP;
	buf[1] = 0;

	UInt32 theLengthSent = 0;
	ZQ_Error err = fSocket.Send(buf, 2, &theLengthSent);
	if(err != ZQ_NoErr)
	{
		char* ptr = new char[2];
		memcpy(ptr, buf, 2);
		OSMutexLocker locker(&fMutexList); 
		ListAppend(&fMsgList, ptr, 2);

		fSocket.RequestEvent(EV_RE | EV_WR);
	}
	ApsLogger::Debug("mqtt_handle_pingreq %d\n",theLengthSent);
	sleep(1);
	//for test
//	mqtt_send_publish(1,sizeof("{\"appid\": \"XXXXXX\"}"),"{\"appid\": \"XXXXXX\"}",1,false,false);

	return err;

}

int MqttServer::mqtt_handle_puback(class MqttPacket &packet)
{
	ApsLogger::Debug("mqtt_handle_puback");
	fTimeoutTask.RefreshTimeout();

	return MQTT_ERR_SUCCESS;
}

int MqttServer::mqtt_send_connack(int result)
{
	char buf[4];
	buf[0] = CONNACK;
	buf[1] = 2;
	buf[2] = 0;
	buf[3] = result;

	UInt32 theLengthSent = 0;
	ZQ_Error err = fSocket.Send(buf, 4, &theLengthSent);
	if(err != ZQ_NoErr)
	{
		char* ptr = new char[4];
		memcpy(ptr, buf, 4);
		OSMutexLocker locker(&fMutexList); 
		ListAppend(&fMsgList, ptr, 4);

		fSocket.RequestEvent(EV_RE | EV_WR);
	}
	ApsLogger::Debug("mqtt_send_connack");

	return err;
}

int MqttServer::mqtt_send_pingresp()
{
	ApsLogger::Debug("mqtt_send_pingresp");
	char buf[2];
	buf[0] = PINGRESP;
	buf[1] = 0;

	UInt32 theLengthSent = 0;
	ZQ_Error err = fSocket.Send(buf, 2, &theLengthSent);
	if(err != ZQ_NoErr)
	{
		char* ptr = new char[2];
		memcpy(ptr, buf, 2);
		OSMutexLocker locker(&fMutexList); 
		ListAppend(&fMsgList, ptr, 2);

		fSocket.RequestEvent(EV_RE | EV_WR);
	}

	return err;
	//return mqtt_send_command(PINGRESP);
}

int MqttServer::mqtt_send_publish(uint16_t mid,uint32_t payloadlen, const void *payload, int qos, bool retain, bool dup)
{
	class MqttPacket packet;
	int rc;
	ApsLogger::Debug("mqtt_send_publish");
	int pos = 0;
	char buf[2048];
	buf[pos++] = PUBLISH | ((dup&0x1)<<3) | (qos<<1);
	buf[pos++] = payloadlen+2;
	buf[pos++] = mid>>8;
	buf[pos++] = mid&0xff;

	memcpy((void *)&buf[pos],(const void *)payload,payloadlen);
	pos += payloadlen;

	UInt32 theLengthSent = 0;
	ZQ_Error err = fSocket.Send(buf, pos, &theLengthSent);
	if(err != ZQ_NoErr)
	{
		char* ptr = new char[pos];
		memcpy(ptr, buf, pos);
		OSMutexLocker locker(&fMutexList); 
		ListAppend(&fMsgList, ptr, pos);

		fSocket.RequestEvent(EV_RE | EV_WR);
	}
	return err;
}
/* For DISCONNECT, PINGREQ and PINGRESP */
/*
int MqttServer::mqtt_send_command(uint8_t command)
{
	class MqttPacket packet;
	ApsLogger::Debug("mqtt_send_command\n");

	packet.SetCommand(command);
	packet.SetRemainingLength(0);

	char data[1024];
	int length = packet.GetTelegram(data,sizeof(data));

	UInt32 theLengthSent = 0;
	ZQ_Error err = fSocket.Send(data, length, &theLengthSent);
	if(err != ZQ_NoErr)
	{
		char* ptr = new char[theLengthSent];
		memcpy(ptr, data, theLengthSent);
		OSMutexLocker locker(&fMutexList); 
		ListAppend(&fMsgList, ptr, theLengthSent);

		fSocket.RequestEvent(EV_RE | EV_WR);
	}

	return err;
}
*/
void MqttServer::mqtt_server_disconnect()
{
	ApsLogger::Debug("Disconn session!"); 	
	MqttSessionHandler::GetInstance()->DeleteMqttSession(clientId);
	fTimeoutTask.SetTimeout(1);
}

void MqttServer::CleanupRequest()
{
	char theDumpBuffer[2000];
    
    ZQ_Error theErr = ZQ_NoErr;
    while (theErr == ZQ_NoErr)
        theErr = this->Read(theDumpBuffer, 2000, NULL);
}

