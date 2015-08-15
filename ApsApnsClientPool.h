#ifndef _APS_APNS_CLENT_POOL_H_
#define _APS_APNS_CLENT_POOL_H_
#include "StrPtrLen.h"
#include "OSMutex.h"
#include "MyAssert.h"
#include "SocketUtils.h"
#include "XMLPrefsParser.h"
#include "StringParser.h"
#include "TimeoutTask.h"
#include "LinkedList.h"
#include "ZQDataconverter.h"
#include "ApsConfigParser.h"
#include "ApsLogger.h"

#include "ApsApnsPrimeryClient.h"
#include "ApsApnsAdvanceClient.h"
#include "ApsApnsFeedbackClient.h"
#include "ApsLcsUtil.h"
#include "ApsLcs.h"

class ApsApnsClientPool : public Task, public ApsLcsWrapper
{
public:
		//call this before calling anything else
	static ApsApnsClientPool* GetInstance(void);
	ApsApnsClientPool(void);
	virtual ~ApsApnsClientPool(void);
	void InitializeParam();
	void ReInitializeParam();
	virtual SInt64			Run();
	void		PushNotification(const char *pToken,const char *pMsg);
    OSQueue                 fQueue;

	uint32_t    GetMessageID();
private:
	static ApsApnsClientPool* m_APNsSocket;
	TimeoutTask         fTimeoutTask;//allows the session to be timed out
	static uint32_t m_messageID;
	ApsApnsAdvanceClient m_advanceClient;
	ApsApnsPrimeryClient m_primeryClient;
	ApsApnsFeedbackClient m_feedbackClient;
	static OSMutex        m_Lock;   //lock  
	static XMLPrefsParser*	sPrefsSource;
	StrPtrLen		m_strtype;
	StrPtrLen		m_strAddr;
	short			m_nPort;
	StrPtrLen		m_strPassword;
	StrPtrLen		m_strPath;
	StrPtrLen		m_strCert;
	StrPtrLen       m_strKey;
	StrPtrLen       m_strFeedbackAddr;
	short			m_nFeedbackPort;

    OSMutex                 fMutex;

};

#endif

