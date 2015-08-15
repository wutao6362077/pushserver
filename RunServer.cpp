/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 */
/*
    File:       main.cpp

    Contains:   main function to drive streaming server.

    

*/

#include <errno.h>

#include "RunServer.h"
#include "SafeStdLib.h"
#include "OS.h"
//#include "OSMemory.h"
#include "OSThread.h"
#include "Socket.h"
#include "SocketUtils.h"
#include "ev.h"
#include "OSArrayObjectDeleter.h"
#include "Task.h"
#include "IdleTask.h"
#include "TimeoutTask.h"
#include "DateTranslator.h"


#ifndef __Win32__
    #include <sys/types.h>
    #include <unistd.h>
#endif

#include <stdlib.h>
#include <sys/stat.h>
#include "ApsLogger.h"
#include "ApsLcs.h"
AvconPushServer *theSver = NULL;
int sStatusUpdateInterval = 0;
Bool16 sHasPID = false;
UInt64 sLastStatusPackets = 0;
UInt64 sLastDebugPackets = 0;
SInt64 sLastDebugTotalQuality = 0;
#ifdef __sgi__ 
#include <sched.h>
#endif

ZQUtils * util;

QTSS_ServerState StartServer()
{
	QTSS_ServerState theServerState = qtssStartingUpState;
//	std::string msg = "/home/running/log4cxx.properties";  
//	g_logger = log4cxx::Logger::getRootLogger();  
//	log4cxx::PropertyConfigurator::configure(msg);	
	ApsLogger::Info("3: Push server init now!");  
//	printf("Push server starting now!\n");
	
	//Get Config Parser Instance and Check XML config file correct
	ApsConfigParser::GetInstance()->CheckFileCorrect();
	
	//Get Redis Client Instance and Connect to server
	ApsRedisLocation::GetInstance()->Initialize();
	ApsTokenRedisLocation::GetInstance()->Initialize();
//	ApsMongodbLocation::GetInstance()->Initialize();
	//Init task thread
	double pa = 2, pb = 17; // 2^5=131072
	util = new ZQUtils(false, pow(pa, pb) - 1);
	
	//Create the APNs client socket
//	PushClientSocket::GetInstance()->Initialize();

	//创建server类，并初始化
	theSver = AvconPushServer::GetInstance();
	if(!theSver->Initialize())
	{
		ApsLogger::Fatal("AvconPushServer init error!");  
		//qtss_printf("ZQMqttSvr init error!");
		::exit(0);
	} 
	ApsLogger::Info("AvconPushServer init successful!");
	//开启server，注册listener的socket句柄到eventthread中去
	theSver->StartTasks();
	LCS::Instance().StartLcs();
	return theServerState;
}

void WritePid(Bool16 forked)
{
#ifndef __Win32__
    // WRITE PID TO FILE
/*    OSCharArrayDeleter thePidFileName(theSver->GetPrefs()->GetPidFilePath());
    FILE *thePidFile = fopen(thePidFileName, "w");
    if(thePidFile)
    {
        if (!forked)
            fprintf(thePidFile,"%d\n",getpid());    // write own pid
        else
        {
            fprintf(thePidFile,"%d\n",getppid());    // write parent pid
            fprintf(thePidFile,"%d\n",getpid());    // and our own pid in the next line
        }                
        fclose(thePidFile);
        sHasPID = true;
    }
    */
#endif
}

void CleanPid(Bool16 force)
{
#ifndef __Win32__
/*    if (sHasPID || force)
    {
        OSCharArrayDeleter thePidFileName(theSver->GetPrefs()->GetPidFilePath());
        unlink(thePidFileName);
    }
    */
#endif
}

void RunServer()
{   
    Bool16 restartServer = false;
    UInt32 loopCount = 0;
    UInt32 debugLevel = 0;
    Bool16 printHeader = false;
    Bool16 printStatus = false;


    //just wait until someone stops the server or a fatal error occurs.
    QTSS_ServerState theServerState = theSver->GetServerState();
    while ((theServerState != qtssShuttingDownState) &&
            (theServerState != qtssFatalErrorState))
    {
#ifdef __sgi__
        OSThread::Sleep(999);
#else
        OSThread::Sleep(1000);
#endif

        if ((theSver->SigIntSet()) || (theSver->SigTermSet()))
        {
            //
            // start the shutdown process
            theServerState = qtssShuttingDownState;
   //         (void)QTSS_SetValue(QTSServerInterface::GetServer(), qtssSvrState, 0, &theServerState, sizeof(theServerState));

            if (theSver->SigIntSet())
                restartServer = true;
        }
    }
        
    //Now, make sure that the server can't do any work
    TaskThreadPool::RemoveThreads();
    
    //now that the server is definitely stopped, it is safe to initate
    //the shutdown process
    delete theSver;
    
    CleanPid(false);
    //ok, we're ready to exit. If we're quitting because of some fatal error
    //while running the server, make sure to let the parent process know by
    //exiting with a nonzero status. Otherwise, exit with a 0 status
    if (theServerState == qtssFatalErrorState || restartServer)
        ::exit (-2);//-2 signals parent process to restart server
}
