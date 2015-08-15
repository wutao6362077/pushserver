#ifndef __LOCATON_UTIL_H__
#define __LOCATON_UTIL_H__
#include "Task.h"
#include "OSRef.h"
#include <list>
#include <vector>
#include "mongo/client/dbclient.h"
class ApsLcsQuery;
class ApsLcsResult;
class ApsLcsWrapper
{
public:
	ApsLcsWrapper(Task* pTask);
	virtual ~ApsLcsWrapper(void);

	static OSRefTable		sLcsRefTable;
	void Register();
	void Unregister();
public:
	void NotifyQueryResult(ApsLcsResult*pKVResult);
	void ProcessQueryResult();
public:
//	virtual void OnAdd(Bool16 result, const char* pToken, const char* msgSeqn)=0;
//	virtual void OnFind(const char*pRQType, const char*pTUid, const char*pTDid, const char*pTSid, const char*pPackBuf, int nLen)=0;
//	virtual void OnRemove(const char*pRQType, const char*pTUid, const char*pTDid, const char*pPackBuf, int nLen)=0;
//	virtual void OnUpdate(const char*pCID, const char*pTUid, const char*pTDid, const char*pTSid, const char*pRlyAddrJson)=0;

private:
	Task*		m_pTask;
	OSRef		m_LcsRef;

	StrPtrLenDel m_splLcsName;
	std::list<ApsLcsResult *> m_vectorResult;
};

class ApsLcsQuery
{
public:
	ApsLcsQuery(const char*pQType, ApsLcsWrapper*pWrapper);
	virtual ~ApsLcsQuery();

	const char* QType(){return m_strQType.c_str();};
	ApsLcsWrapper* Wrapper(){return m_pWrapper;};
	void SetCollection(char * collection);
	void SetQuery(mongo::Query &query);
	void SetUpdate(mongo::BSONObj &update);
	const char * GetCollection();
	
	mongo::Query& GetQuery();
	
	mongo::BSONObj& GetUpdate();
private:
	std::string		m_strQType;
	std::string		m_strQCollection;
	mongo::Query	m_query;
	mongo::BSONObj	m_obj;
	
	ApsLcsWrapper*	m_pWrapper;
};

class ApsLcsResult
{
public:
	ApsLcsResult(const char*pQType);
	virtual ~ApsLcsResult();

	const char* QType(){return m_strQType.c_str();};

	bool 	GetResultFlag(){return flag;};

	void	AddResult(mongo::BSONObj data);

	std::vector<mongo::BSONObj>& GetResult(mongo::BSONObj data);
	
private:
	std::string		m_strQType;
	bool			flag;
	UInt32			m_nLen;	
	std::vector<mongo::BSONObj> m_pResult;
};

#endif	// __LOCATON_UTIL_H__
