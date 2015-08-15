#include "ApsLcsUtil.h"
#include "ApsLcs.h"
#include "ApsLogger.h"
OSRefTable	ApsLcsWrapper::sLcsRefTable(1024*128-1);

ApsLcsWrapper::ApsLcsWrapper(Task* pTask)
: m_pTask(pTask)
{
	Assert(NULL != m_pTask);
	Register();
}

ApsLcsWrapper::~ApsLcsWrapper(void)
{
	//must add free memory code
	//m_QResultPool.Clear();
	Unregister();
}

void ApsLcsWrapper::Register()
{
	char lcsName[32];
	int nn = sprintf(lcsName, "%p", m_pTask);
	m_splLcsName.Set(lcsName, nn);

	m_LcsRef.Set(m_splLcsName, this);
	m_splLcsName.Ptr = NULL;
	m_splLcsName.Len = 0;

	ApsLcsWrapper::sLcsRefTable.Register(&m_LcsRef);
}

void ApsLcsWrapper::Unregister()
{
	ApsLcsWrapper::sLcsRefTable.UnRegister(&m_LcsRef);
}

void ApsLcsWrapper::NotifyQueryResult(ApsLcsResult*pKVResult)
{
	m_vectorResult.push_back(pKVResult);

	m_pTask->Signal(Task::kLcsEvent);
}

void ApsLcsWrapper::ProcessQueryResult()
{
	ApsLcsResult* pQResult = NULL;
	std::list<ApsLcsResult *>::iterator itr;
	while(!m_vectorResult.empty())
	{
		itr = m_vectorResult.begin();
		pQResult = *itr;

		if(pQResult)
		{
			const char* pQType = pQResult->QType();
			if(strcmp(pQType, "Add") == 0)
			{
			}
			else if(strcmp(pQType, "Find") == 0)
			{
				ApsLogger::Info("ApsLcsWrapper::ProcessQueryResult");
			}
			else if(strcmp(pQType, "Remove") == 0)
			{
			}
			else if(strcmp(pQType, "Update") == 0)
			{
			}

			//* Important
			{// Release the ref.
				m_vectorResult.erase(itr);
				delete pQResult;
				pQResult = NULL;
			}
		}
		else
			break;
	}
}

//===========================================================
ApsLcsQuery::ApsLcsQuery(const char*pQType, ApsLcsWrapper*pWrapper)
: m_pWrapper(pWrapper)
{
	Assert(NULL != pQType);
	Assert(NULL != pWrapper);
	m_strQType=pQType;
};
ApsLcsQuery::~ApsLcsQuery()
{
}

void ApsLcsQuery::SetCollection(char * collection)
{
	m_strQCollection = collection;
}
void ApsLcsQuery::SetQuery(mongo::Query &query)
{
	m_query = query;
}
void ApsLcsQuery::SetUpdate(mongo::BSONObj &update)
{
	m_obj = update;
}

const char * ApsLcsQuery::GetCollection()
{
	return m_strQCollection.c_str();
}
mongo::Query& ApsLcsQuery::GetQuery()
{
	return m_query;
}
mongo::BSONObj& ApsLcsQuery::GetUpdate()
{
	return m_obj;
}

//===========================================================
ApsLcsResult::ApsLcsResult(const char*pQType)
{
	Assert(NULL != pQType);
	m_strQType=pQType;
	m_nLen = 0;
};
ApsLcsResult::~ApsLcsResult()
{

}

void ApsLcsResult::AddResult(mongo::BSONObj data)
{
	m_pResult.push_back(data);
	m_nLen++;
}

std::vector<mongo::BSONObj>& ApsLcsResult::GetResult(mongo::BSONObj data)
{
	return m_pResult;
}

