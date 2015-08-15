#include "ApsConfigParser.h"
#include "ApsLogger.h"

ApsConfigParser* ApsConfigParser::m_configParser = NULL;
OSMutex        ApsConfigParser::m_lock;   //lock

ApsConfigParser* ApsConfigParser::GetInstance(char* inPath)
{
	if (m_configParser == NULL)
    {
		OSMutexLocker locker(&m_lock);
    	if (m_configParser == NULL)
    	{		
			ApsLogger::Debug("New ApsConfigParser!"); 
       		m_configParser = new ApsConfigParser(inPath);
    	}
    }

    return m_configParser;
}

ApsConfigParser::ApsConfigParser(char* inPath)
:   XMLPrefsParser(inPath)
{
	ApsLogger::Debug("Config file path is %s!",inPath); 
	m_configPath = inPath;
}

ApsConfigParser::~ApsConfigParser(void)
{
}

void ApsConfigParser::CheckFileCorrect(void)
{
    // Check to see if the XML file exists as a directory. If it does,
    // just bail because we do not want to overwrite a directory
    if (DoesFileExistAsDirectory())
    {
    	
		ApsLogger::Debug("Directory located at location where streaming server prefs file should be.");  
//        qtss_printf("Directory located at location where streaming server prefs file should be.\n");
        ::exit(0);
    }
    
    if (!CanWriteFile())
    {
    	
		ApsLogger::Debug("Cannot write to the streaming server prefs file.");  
//        qtss_printf("Cannot write to the streaming server prefs file.\n");
        ::exit(0);
    }

    // If we aren't forced to create a new XML prefs file, whether
    // we do or not depends solely on whether the XML prefs file exists currently.
	if(!DoesFileExist())
	{
		
		ApsLogger::Debug("Could not find prefs file.");  
//		 qtss_printf("Fatal Error: Could not find prefs file at: %s.\n", m_configPath);
         ::exit(-1);
	}

    //
    // Parse the configs from the XML file
    int xmlParseErr = Parse();
    if (xmlParseErr)
    {
    	
		ApsLogger::Debug("Could not load configuration file at.");  
//        qtss_printf("Fatal Error: Could not load configuration file at %s. (%d)\n", m_configPath, OSThread::GetErrno());
        ::exit(-1);
    }
}
