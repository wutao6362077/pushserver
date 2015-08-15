#include <stdio.h>
#include <stdlib.h>

#include<sys/types.h>
#include<sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h> 

#include "RunServer.h"
#include "ApsLogger.h"

static pid_t sChildPID = 0;

Bool16 sendtochild(int sig, pid_t myPID)
{
    if (sChildPID != 0 && sChildPID != myPID) // this is the parent
    {   // Send signal to child
    	ApsLogger::Debug("Send signal %d to child\n", sig);    
        ::kill(sChildPID, sig);
        return true;
    }

    return false;
}

void sigcatcher(int sig)
{
//#if DEBUG
    ApsLogger::Debug("Signal %d caught\n", sig);
//#endif
    pid_t myPID = getpid();
    //
    // SIGHUP means we should reread our preferences
    if (sig == SIGHUP)
    {
        if (sendtochild(sig,myPID))
        {
            return;
        }
    }
        
    //Try to shut down gracefully the first time, shutdown forcefully the next time
    if (sig == SIGINT) // kill the child only
    {
        if (sendtochild(sig,myPID))
        {
            return;// ok we're done 
        }
    }
	
	if (sig == SIGTERM || sig == SIGQUIT) // kill child then quit
    {
        if (sendtochild(sig,myPID))
        {
             return;// ok we're done 
        }
	}
}

int daemon(int nochdir, int noclose)
{
    int fd;

    switch (fork()) {
    case -1:
        return (-1);
    case 0:
        break;
    default:
        _exit(0);
    }

    if (setsid() == -1)
        return (-1);

    if (!nochdir)
        (void)chdir("/");

    if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
        (void)dup2(fd, STDIN_FILENO);
        (void)dup2(fd, STDOUT_FILENO);
        (void)dup2(fd, STDERR_FILENO);
        if (fd > 2)
            (void)close (fd);
    }
    return (0);
}

Bool16 RestartServer(char* theXMLFilePath)
{
	Bool16 autoRestart = true;
	XMLPrefsParser theXMLParser(theXMLFilePath);
	theXMLParser.Parse();
	
	ContainerRef server = theXMLParser.GetRefForServer();
	ContainerRef pref = theXMLParser.GetPrefRefByName( server, "auto_restart" );
	char* autoStartSetting = NULL;
	
	if (pref != NULL)
		autoStartSetting = theXMLParser.GetPrefValueByRef( pref, 0, NULL, NULL );
		
	if ( (autoStartSetting != NULL) && (::strcmp(autoStartSetting, "false") == 0) )
		autoRestart = false;

	ApsLogger::Debug("Restart flag is %d.",autoRestart);
	
	return autoRestart;
}

int main(int argc, char* argv[])
{
#ifdef __Win32__
    // Start Win32 DLLs
    WORD wsVersion = MAKEWORD(1, 1);
    WSADATA wsData;
    (void)::WSAStartup(wsVersion, &wsData);
#else

	ApsLogger::Info("%s[%d]: ""Push Server Version:1.1.1 Date:2015-2-6 Start Now ...",__FILE__, __LINE__);

	//Daemon Server Application
	if (daemon(0,0) != 0)
	{
		ApsLogger::Fatal("1: Daenon process false.");
		exit(-1);
	}

	
	/* Load encryption & hashing algorithms for the SSL program */
    SSL_library_init();

    /* Load the error strings for SSL & CRYPTO APIs */
    SSL_load_error_strings();
	
	ApsLogger::Info("1: Daenon process: Successful.");
	
    // Set Signal Function
    struct sigaction act;   
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = (void(*)(int))&sigcatcher;
    (void)::sigaction(SIGPIPE, &act, NULL);
    (void)::sigaction(SIGHUP, &act, NULL);
    (void)::sigaction(SIGINT, &act, NULL);
    (void)::sigaction(SIGTERM, &act, NULL);
    (void)::sigaction(SIGQUIT, &act, NULL);
    (void)::sigaction(SIGALRM, &act, NULL);
#endif

#if defined(AUTO_TEST)
	ZQTestPushMsg testPushMsg;
#endif

#ifndef __Win32__
	int status = 0;
	int pid = 0;
	pid_t processID = 0;

    // loop until the server exits normally. If the child server doesn't exit
    // normally, then restart it.
    // normal exit means the following
    // the child quit 
    do // fork at least once but stop on the status conditions returned by wait or if autoStart pref is false
    {
    	ApsLogger::Info("2: Creat child process.");
        processID = fork();
        Assert(processID >= 0);
        if (processID > 0) // this is the parent and we have a child
        {
        	ApsLogger::Debug("parent process running now, child proccess id = %d.",processID);
            sChildPID = processID;
            status = 0;
            while (status == 0) //loop on wait until status is != 0;
            {	
            	// Wait for child status
                pid =::wait(&status);
                SInt8 exitStatus = (SInt8) WEXITSTATUS(status);
                //qtss_printf("Child Process %d wait exited with pid=%d status=%d exit status=%d\n", processID, pid, status, exitStatus);
				ApsLogger::Debug("Child Process %d wait exited with pid=%d status=%d exit status=%d\n", processID, pid, status, exitStatus);                	
				if (WIFEXITED(status) && pid > 0 && status != 0) // child exited with status -2 restart or -1 don't restart 
				{
					//qtss_printf("child exited with status=%d\n", exitStatus);
					ApsLogger::Debug("child exited with status=%d\n", exitStatus);											
					if ( WEXITSTATUS(status) == -1) // child couldn't run don't try again
					{
						ApsLogger::Debug("child exited with -1 fatal error so parent is exiting too.\n"); 															
						qtss_printf("child exited with -1 fatal error so parent is exiting too.\n");
						exit (EXIT_FAILURE); 
					}
					break; // restart the child			
				}
					
				if (WIFSIGNALED(status)) // child exited on an unhandled signal (maybe a bus error or seg fault)
				{	
					ApsLogger::Debug("child was signalled\n");															
					//qtss_printf("child was signalled\n");
					break; // restart the child
				}
  		
                if (pid == -1 && status == 0) // parent woken up by a handled signal
                {
					ApsLogger::Debug("handled signal continue waiting\n");
                   	continue;
                }
                   	
                if (pid > 0 && status == 0)
                {
                 	ApsLogger::Info("child exited cleanly so parent is exiting\n");
                 	exit(EXIT_SUCCESS);                		
                }
                	
                ApsLogger::Fatal("child died for unknown reasons parent is exiting\n");
                exit (EXIT_FAILURE);
            }
        }
        else if (processID == 0) // must be the child
        {	
			ApsLogger::Debug("Child process running now.");
			break;
        }
        else
        {	
			ApsLogger::Fatal("Create child process failed, ret = %d.",processID);
            exit(EXIT_FAILURE);
        } 	
            	
        //eek. If you auto-restart too fast, you might start the new one before the OS has
        //cleaned up from the old one, resulting in startup errors when you create the new
        //one. Waiting for a second seems to work
        sleep(1);
    } while (RestartServer(theXMLFilePath)); // fork again based on pref if server dies

	if (processID != 0) //the parent is quitting
	{	
		ApsLogger::Fatal("Parent breakout without restart\n");
        exit(EXIT_SUCCESS);   
	}
	
    sChildPID = 0;
    //we have to do this again for the child process, because sigaction states
    //do not span multiple processes.
    (void)::sigaction(SIGPIPE, &act, NULL);
    (void)::sigaction(SIGHUP, &act, NULL);
    (void)::sigaction(SIGINT, &act, NULL);
    (void)::sigaction(SIGTERM, &act, NULL);
    (void)::sigaction(SIGQUIT, &act, NULL);

	//This function starts, runs, and shuts down the server
    if (::StartServer() != qtssFatalErrorState)
    {    ::RunServer();
         CleanPid(false); 
		 ApsLogger::Info("Child breakout successful\n");
         exit (EXIT_SUCCESS);
    }
    else
    {
		ApsLogger::Info("Child breakout error,donot restart again\n");  
    	exit(-1); //Cant start server don't try again
    }
#else
	//just wait until someone stops the server or a fatal error occurs.
	Bool16		restartServer = false;
    ZQ_ServerState theServerState = theSver->GetServerState();
    while ((theServerState != zqShuttingDownState) &&
            (theServerState != zqFatalErrorState))
    {
		if(!theSver->ProcessMsg())
		{
			theSver->SetSigTerm();
		}

        if ((theSver->SigIntSet()) || (theSver->SigTermSet()))
        {
            // start the shutdown process
            theServerState = zqShuttingDownState;

            if (theSver->SigIntSet())
                restartServer = true;
        }
		else
		{
			theServerState = theSver->GetServerState();
		}
    }
	//system("PAUSE");
	return 0;
#endif
}

