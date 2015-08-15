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
    File:       TCPSessionInterface.cpp

    Contains:   Implementation of TCPSessionInterface object.
    
    

*/

#include "atomic.h"

#include "TCPSslSessionInterface.h"
#include "OSMemory.h"

#include <errno.h>


#if DEBUG
	#define TCP_SESSION_INTERFACE_DEBUGGING 1
#else
    #define TCP_SESSION_INTERFACE_DEBUGGING 0
#endif



unsigned int            TCPSslSessionInterface::sSessionIDCounter = kFirstTCPSessionID;


void    TCPSslSessionInterface::Initialize()
{
	// DJM PROTOTYPE
	::srand((unsigned int) OS::Microseconds());
}


TCPSslSessionInterface::TCPSslSessionInterface(int fd,struct sockaddr_in *addr) 
:   Task(), 
    fTimeoutTask(NULL, 120 * 1000),
    fInputStream(&fSocket),
    fSessionMutex(),
    fSocket(),
    fInputSocketP(&fSocket),
    fObjectHolders(0)
{
	memset(fUserNameBuf, 0, kMaxUserNameLen);
	memset(fUserPasswordBuf, 0, kMaxUserPasswordLen);
	memset(fUserRealmBuf, 0, kMaxUserRealmLen);
	fSocket.Set(fd, addr);
    fTimeoutTask.SetTask(this);
    fSocket.SetTask(this);

    fSessionID = (UInt32)atomic_add(&sSessionIDCounter, 1);
}


TCPSslSessionInterface::~TCPSslSessionInterface()
{
     //delete fInputSocketP;
}

void TCPSslSessionInterface::DecrementObjectHolderCount()
{

#if __Win32__
//maybe don't need this special case but for now on Win32 we do it the old way since the killEvent code hasn't been verified on Windows.
    this->Signal(Task::kReadEvent);//have the object wakeup in case it can go away.
    atomic_sub(&fObjectHolders, 1);
#else
    if (0 == atomic_sub(&fObjectHolders, 1))
        this->Signal(Task::kKillEvent);
#endif

}

ZQ_Error TCPSslSessionInterface::Read(SSL *m_pssl,void* ioBuffer, UInt32 inLength, UInt32* outLenRead)
{
	UInt32 theLenRead = 0;
	ZQ_Error theErr = fInputStream.Read(m_pssl,ioBuffer, inLength, &theLenRead);

	if (outLenRead != NULL)
		*outLenRead = theLenRead;

	return theErr;
}