#ifndef __APS_LOCATION_H__
#define __APS_LOCATION_H__
#include <map>
#include <string>

typedef struct tagDID
{
	std::string		strDType;
}DID;	// �豸��Ϣ
typedef std::map<std::string,DID>	MapDID;
typedef std::map<std::string,DID>::iterator	MapDIDIter;
// Location����
class ApsLocation
{
public:
	ApsLocation(void){};
	virtual ~ApsLocation(void){};

	// �������ݿ⣬���ڷ����ݿ����Ͳ���Ҫʵ�ִ˷���
	virtual bool	ConnectSql(void)=0;
	
	// 
	virtual bool	CheckConnection()=0;
};

#endif