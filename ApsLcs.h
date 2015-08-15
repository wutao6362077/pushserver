#ifndef __LOCATON_SERVER_H__
#define __LOCATON_SERVER_H__

#include "ApsLcsUtil.h"
#include "ApsMongodbLocation.h"

class LCS : public Task
{
public:
	static LCS&Instance(void);

	void StartLcs();
	void StopLcs();

	void RequestExcute(ApsLcsQuery* pExcute);
	void RequestQuery(ApsLcsQuery* pQuery);
protected:
	LCS();
	virtual ~LCS(void);
	virtual SInt64          Run();
	void ProcessExcute(ApsLcsQuery* pQuery);
	void ProcessQuery(ApsLcsQuery* pQuery);
private:
	Bool16			m_bStarted;
	std::list<ApsLcsQuery *>	m_QueryPool;
	ApsMongodbLocation *m_Location;
};

#endif //__LOCATON_H__