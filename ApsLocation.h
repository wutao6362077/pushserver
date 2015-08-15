#ifndef __APS_LOCATION_H__
#define __APS_LOCATION_H__
#include <map>
#include <string>

typedef struct tagDID
{
	std::string		strDType;
}DID;	// 设备信息
typedef std::map<std::string,DID>	MapDID;
typedef std::map<std::string,DID>::iterator	MapDIDIter;
// Location基类
class ApsLocation
{
public:
	ApsLocation(void){};
	virtual ~ApsLocation(void){};

	// 连接数据库，对于非数据库类型不需要实现此方法
	virtual bool	ConnectSql(void)=0;
	
	// 
	virtual bool	CheckConnection()=0;
};

#endif