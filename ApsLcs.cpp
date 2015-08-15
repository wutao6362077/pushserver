#include "ApsLcs.h"
#include "OS.h"
#include "ApsLogger.h"
LCS&LCS::Instance(void)
{
	static LCS gLcs;
	return gLcs;
}

LCS::LCS()
: Task()
, m_bStarted(false)
{
	Task::SetThreadPicker(Task::GetBlockingTaskThreadPicker());
	m_Location = ApsMongodbLocation::GetInstance();
	//m_QueryPool.reserve(4*1024*1024);
}

LCS::~LCS(void)
{
	m_QueryPool.clear();
}

void LCS::StartLcs()
{	
	m_Location->ConnectSql();
	ApsLogger::Debug("LCS::StartLcs.");

	m_bStarted = true;
	this->Signal(Task::kStartEvent);
}

void LCS::StopLcs()
{
	m_bStarted = false;

	this->Signal(Task::kKillEvent);
}


void LCS::RequestExcute(ApsLcsQuery* pExcute)
{
	RequestQuery(pExcute);
}

void LCS::RequestQuery(ApsLcsQuery* pQuery)
{
	m_QueryPool.push_back(pQuery);

	ApsLogger::Debug("LCS::RequestQuery Query = %p.",pQuery);

	this->Signal(Task::kUpdateEvent);
}

SInt64 LCS::Run()
{
	EventFlags events = this->GetEvents();
	ApsLcsQuery *last = NULL;

	if(events & Task::kKillEvent)
	{
		ApsLogger::Debug("LCS::Run Task::kKillEvent.");
		return -1;
	}

	if(!m_bStarted)
		return 0;
	ApsLogger::Debug("LCS::Run.");
	if(events & Task::kUpdateEvent)
	{
		ApsLcsQuery* pQuery = NULL;
		std::list<ApsLcsQuery *>::iterator itr;
		while(!m_QueryPool.empty())
		{
			{// Get Query cmd.
				//pQuery = m_QueryPool.front();
				itr = m_QueryPool.begin();
				pQuery = *itr;
			}
			//有时会删除不掉，所以重新删除一次，对于vector存在于重新分配缓冲区时
	/*		if(last == pQuery)
			{
				m_QueryPool.erase(itr);
				ApsLogger::Debug("LCS::Run Query twice = %p.",pQuery);
				continue;
			}
			last = pQuery;
	*/
			ApsLogger::Debug("LCS::Run Query = %p.",pQuery);

			if(pQuery)
			{// Process and delete it.
				const char* pQType = pQuery->QType();
				switch(pQType[0])
				{
				case 'A':
					{
						ProcessExcute(pQuery);
						break;
					}
				case 'F':
					{
						ProcessQuery(pQuery);
						break;
					}
				case 'R':
					{
						ProcessExcute(pQuery);
						break;
					}
				case 'U':
					{
						ProcessExcute(pQuery);
						break;
					}
				}
				m_QueryPool.remove(pQuery);
				delete pQuery;
				pQuery = NULL;
			}
			else
				break;
		}
	}
	
	return 0;
}

void LCS::ProcessExcute(ApsLcsQuery* pQuery)
{
	ApsLogger::Debug("ProcessExcute Query = %p.",pQuery);

	const char* pQType = pQuery->QType();
	if(strcmp(pQType, "Add") == 0)
	{// Login
		m_Location->Add(pQuery->GetCollection(), pQuery->GetUpdate());
	}
	else if(strcmp(pQType, "Remove") == 0)
	{// Logout
		m_Location->Remove(pQuery->GetCollection(), pQuery->GetUpdate());
	}
	else if(strcmp(pQType, "Update") == 0)
	{// Logout
		m_Location->Update(pQuery->GetCollection(), pQuery->GetQuery(), pQuery->GetUpdate());
	}
	else
	{
		ApsLogger::Debug("ProcessExcute error.");
	}
}

void LCS::ProcessQuery(ApsLcsQuery* pQuery)
{
	ApsLogger::Debug("ProcessQuery Query = %p.",pQuery);

	const char* pQType = pQuery->QType();
	if(strcmp(pQType, "Find") == 0)
	{
		ApsLcsResult *result = new ApsLcsResult("Find");
		std::auto_ptr<mongo::DBClientCursor> message = m_Location->Find(pQuery->GetCollection(), &(pQuery->GetQuery()));	
		mongo::BSONObj doc = message->next();
		//while(message->more())
		//{
			std::string id = mongo::tojson(doc);
			ApsLogger::Debug("ProcessQuery Get mongodb = %s.",id.c_str());
			result->AddResult(doc);
			doc = message->next();
		//}
		pQuery->Wrapper()->NotifyQueryResult(result);
	}
	else
	{
		ApsLogger::Debug("ProcessQuery error.");
	}
}

