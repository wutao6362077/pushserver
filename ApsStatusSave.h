#ifndef __APS_MESSAGE_SAVE_H__
#define __APS_MESSAGE_SAVE_H__
#include "ApsAppRequestMessage.h"
#include <string>
#include <stdint.h>
#include "ApsLcs.h"

//Table Date Gateway Model to operation provider.message mongodb collection 
class ApsMessageStatus
{
public:
	//call this before calling anything else
	static ApsMessageStatus* GetInstance();
	
	virtual ~ApsMessageStatus(void);

	std::string AddMessage(ApsAppRequestMessage &request,std::string appid,ApsLcsWrapper * wrapper);

	void UpdateMessage(std::string recTime,std::string appid,const char *status,ApsLcsWrapper * wrapper);
	
private:
	ApsMessageStatus(void);
	static ApsMessageStatus* m_messgageStatus;
};

//Table Date Gateway Model to operation provider.error mongodb collection 
class ApsErrorMessage
{
public:
	//call this before calling anything else
	static ApsErrorMessage* GetInstance();
	
	virtual ~ApsErrorMessage(void);

	void AddErrorMessage(ApsAppRequestMessage &request,const char *status,ApsLcsWrapper * wrapper);
	
private:
	ApsErrorMessage(void);
	
	static ApsErrorMessage* m_errorMessgage;
};

//Table Date Gateway Model to operation provider.connect mongodb collection 
class ApsConnectStatus
{
public:
	//call this before calling anything else
	static ApsConnectStatus* GetInstance();
	
	virtual ~ApsConnectStatus(void);

	std::string AddConnectMessage(ApsAppRequestMessage &request,ApsLcsWrapper * wrapper);

	void UpdateConnectStatus(std::string conTime,std::string appid,const char *status,ApsLcsWrapper * wrapper);
	
private:
	ApsConnectStatus(void);
	
	static ApsConnectStatus* m_connectStatus;
};

//Table Date Gateway Model to operation provider.ios mongodb collection 
class ApsIosMessage
{
public:
	//call this before calling anything else
	static ApsIosMessage* GetInstance();
	
	virtual ~ApsIosMessage(void);

	std::string AddPublishMessage(ApsAppRequestMessage &request,std::string token,uint16_t mid,ApsLcsWrapper * wrapper);

	void FindPublishMessage(std::string startTime,uint16_t mid,ApsLcsWrapper * wrapper);
	
private:
	ApsIosMessage(void);
	
	static ApsIosMessage* m_iosMessage;
};

#endif
