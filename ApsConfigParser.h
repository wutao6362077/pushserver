#ifndef __APS_CONFIG_PARSER_H__
#define __APS_CONFIG_PARSER_H__

#include "XMLPrefsParser.h"

static char* theXMLFilePath = "/etc/push-svr.conf";

class ApsConfigParser : public XMLPrefsParser
{
public:
    //call this before calling anything else
	static ApsConfigParser* GetInstance(char* inPath = theXMLFilePath);
	void CheckFileCorrect(void);

	virtual ~ApsConfigParser(void);

private:
	ApsConfigParser(char* inPath);
	static ApsConfigParser* m_configParser;
	static OSMutex        m_lock;   //lock   
	char*           m_configPath;
};

#endif //__PUSHCONFIGPARSER_H__
