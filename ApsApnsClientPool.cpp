#include "ApsApnsClientPool.h"

ApsApnsClientPool* ApsApnsClientPool::m_APNsSocket = NULL;
OSMutex        ApsApnsClientPool::m_Lock;   //lock
XMLPrefsParser* ApsApnsClientPool::sPrefsSource = NULL;
uint32_t ApsApnsClientPool::m_messageID = 0;

ApsApnsClientPool::ApsApnsClientPool(): Task(),m_advanceClient(this),m_primeryClient(this),m_feedbackClient(this),ApsLcsWrapper(this),fTimeoutTask(this, 5*60*1000)
{
	this->SetThreadPicker(Task::GetBlockingTaskThreadPicker());
	//fTimeoutTask.SetTask(this);
	//fTimeoutTask.SetTimeout(5*60*1000);//300s read feedback message
}

ApsApnsClientPool::~ApsApnsClientPool()
{

}
void ApsApnsClientPool::ReInitializeParam()
{
	m_feedbackClient.InitializeParam(m_strFeedbackAddr,m_nFeedbackPort,m_strPassword,m_strPath,m_strCert,m_strKey);
	m_advanceClient.InitializeParam(m_strAddr,m_nPort,m_strPassword,m_strPath,m_strCert,m_strKey);
}

void ApsApnsClientPool::InitializeParam()
{
	sPrefsSource = ApsConfigParser::GetInstance();

	ContainerRef svrPref = sPrefsSource->GetRefForAPNsType();
	ContainerRef pref = NULL;
    char* thePrefValue = NULL;
	char* thePrefTypeStr = NULL;
    char* thePrefName = NULL;
	ZQ_AttrDataType theType;
	UInt32 convertedBufSize = 0;
	
	pref = sPrefsSource->GetPrefRefByName( svrPref, "apns_server_type" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strtype.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}

	if(strncmp(m_strtype.GetAsCString(),"production",sizeof("production")))
	{
		svrPref = sPrefsSource->GetRefForAPNsDevelopment();
	}
	else
	{
		svrPref = sPrefsSource->GetRefForAPNsProduction();
	}
	//Get the APNs server ip
	pref = sPrefsSource->GetPrefRefByName( svrPref, "apns_server_ip" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strAddr.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}

	//Get the APNs server port
    pref = sPrefsSource->GetPrefRefByName( svrPref, "apns_server_port");
	if (pref != NULL)
	{
		if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName, (char**)&thePrefTypeStr))
		{
			theType = ZQDataConverter::TypeStringToType(thePrefTypeStr);
			convertedBufSize = sizeof(UInt16);
			ZQDataConverter::StringToValue(thePrefValue, theType, &m_nPort, &convertedBufSize);
		}
	}

	pref = sPrefsSource->GetPrefRefByName( svrPref, "apns_key_password" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strPassword.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}

	//Get the APNs feedback server ip
	pref = sPrefsSource->GetPrefRefByName( svrPref, "apns_feedback_server_ip" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strFeedbackAddr.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}

	//Get the APNs feedback server port
    pref = sPrefsSource->GetPrefRefByName( svrPref, "apns_feedback_server_port");
	if (pref != NULL)
	{
		if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName, (char**)&thePrefTypeStr))
		{
			theType = ZQDataConverter::TypeStringToType(thePrefTypeStr);
			convertedBufSize = sizeof(UInt16);
			ZQDataConverter::StringToValue(thePrefValue, theType, &m_nFeedbackPort, &convertedBufSize);
		}
	}
	//Get the certification dir
	pref = sPrefsSource->GetPrefRefByName( svrPref, "ca_cert_path" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strPath.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the certification file path
	pref = sPrefsSource->GetPrefRefByName( svrPref, "rsa_client_cert" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strCert.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}
	//Get the certification key path
	pref = sPrefsSource->GetPrefRefByName( svrPref, "rsa_client_key" );
    if (pref != NULL)
	{
        if(thePrefValue = sPrefsSource->GetPrefValueByRef( pref, 0, &thePrefName,(char**)&thePrefTypeStr))
		{
			m_strKey.Set(thePrefValue, ::strlen(thePrefValue));		
		}
	}

	m_advanceClient.InitializeParam(m_strAddr,m_nPort,m_strPassword,m_strPath,m_strCert,m_strKey);
	m_primeryClient.InitializeParam(m_strAddr,m_nPort,m_strPassword,m_strPath,m_strCert,m_strKey);
	m_feedbackClient.InitializeParam(m_strFeedbackAddr,m_nFeedbackPort,m_strPassword,m_strPath,m_strCert,m_strKey);
	fQueue.EnQueue(&(m_advanceClient.fTaskQueueElem));
	fQueue.EnQueue(&(m_primeryClient.fTaskQueueElem));
//	fQueue.EnQueue(&(m_feedbackClient.fTaskQueueElem));
}

void ApsApnsClientPool::PushNotification(const char *pToken,const char *pMsg)
{
	ApsApnsPrimeryRequestMessage request;
	if(0)
	{
		request.SetCommand(0);
		request.SetDeviceToken(pToken);
		request.SetPayload(pMsg);
		m_primeryClient.OnPublish(request);
	}
	else
	{
		request.SetCommand(1);
		request.SetId(++m_messageID);
		request.SetExpriy(100);
		request.SetDeviceToken(pToken);
		request.SetPayload(pMsg);
		m_advanceClient.OnPublish(request);
		//m_feedbackClient.Signal(Task::kReadEvent);
	}
}

//具体的业务处理类，需要根据接收的话题等信息，查找redis中的deviceToken，然后发送给SslTcpSocket
ApsApnsClientPool* ApsApnsClientPool::GetInstance(void)
{
	if (m_APNsSocket == NULL)
    {
		OSMutexLocker locker(&m_Lock);
    	if (m_APNsSocket == NULL)
    	{ 	
			ApsLogger::Debug("New ApsApnsPrimeryClient!"); 
       		m_APNsSocket = new ApsApnsClientPool();
			m_APNsSocket->InitializeParam();
    	}
    }

    return m_APNsSocket;
}

SInt64 ApsApnsClientPool::Run()
{
	ApsLogger::Debug("ApsApnsClientPool::Run!"); 

	EventFlags events = this->GetEvents();//we must clear the event mask!
	if(events&Task::kLcsEvent)
	{
		ApsLogger::Debug("Task::kLcsEvent!"); 
		ProcessQueryResult();
	}
	else if(events&Task::kConnectEvent)
	{
    	//ok, check for connect request now. Go through the whole queue
    	OSMutexLocker locker(&fMutex);

		//send connection
    	for (OSQueueIter iter(&fQueue); !iter.IsDone(); iter.Next())
    	{
        	SslTcpClient* theClient = (SslTcpClient*)iter.GetCurrent()->GetEnclosingObject();
        
        	//if it's time to time this task out, signal it
        	if(theClient==NULL)
        	{
        		if(fQueue.GetLength() == 0)
					break;
				else
					continue;
			}
        	if (!theClient->GetConnectStatus())
        	{
				//theTimeoutTask->fTask->Signal(Task::kTimeoutEvent);
				OS_Error theErr = theClient->GetClientSocket().Connect((TCPSocket *)&(theClient->GetClientSocket().fSocket));
    			if (theErr != OS_NoErr)
    			{	
					ApsLogger::Debug("ssl connect error!"); 
        			continue;
    			}
				else
				{
					ApsLogger::Debug("ssl connect sucessful!"); 
					fQueue.Remove(&(theClient->fTaskQueueElem));
					theClient->Online();
					//设置Task，响应系统的读写事件
					theClient->GetClientSocket().SetTask(theClient);
					theClient->GetClientSocket().SetListener(EV_RE);
					//尝试发送缓冲中的数据
					theClient->NotifySendMsg();
				}
			}
			else
			{
				fQueue.Remove(&(theClient->fTaskQueueElem));
				//theClient->Signal(Task::kConnectEvent);
			}
		}
	}
	else if(events&Task::kTimeoutEvent)
	{
		fTimeoutTask.RefreshTimeout();
		//feedback connection
		m_feedbackClient.OnTimeout();
		ReInitializeParam();
		OS_Error theErr = m_feedbackClient.GetClientSocket().Connect((TCPSocket *)&(m_feedbackClient.GetClientSocket().fSocket));
    	if (theErr != OS_NoErr)
    	{
			ApsLogger::Debug("feedback connect error!"); 
    	}
		else
		{
			ApsLogger::Debug("feedback connect successful!"); 

			m_feedbackClient.SetConnectStatus(true);
			//设置Task，响应系统的读写事件
			m_feedbackClient.GetClientSocket().SetTask(&m_feedbackClient);
			m_feedbackClient.GetClientSocket().SetListener(EV_RE);
		}
	}
	OSThread::ThreadYield();
    
    return 0;//don't delete me!
}

uint32_t ApsApnsClientPool::GetMessageID()
{
	return m_messageID;
}

