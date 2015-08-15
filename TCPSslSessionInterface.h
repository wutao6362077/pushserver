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
    File:       TCPSessionInterface.h

    Contains:   Presents an API for session-wide resources for modules to use.
                Implements the TCP Session dictionary for ZQ API.
    
    
*/

#ifndef __TCPSSLSESSIONINTERFACE_H__
#define __TCPSSLSESSIONINTERFACE_H__
#include "TCPSessionInterface.h"
#include "TCPSslRequestStream.h"
#include "Task.h"
#include "ZQTypes.h"
#include "atomic.h"
#include "IdleTask.h"
#include "TimeoutTask.h"


class TCPSslSessionInterface : public Task
{
public:

    //Initialize must be called right off the bat to initialize dictionary resources
    static void     Initialize();
    
    TCPSslSessionInterface(int fd,struct sockaddr_in *addr);
    virtual ~TCPSslSessionInterface();
    
    // Allows clients to refresh the timeout
    void RefreshTimeout()       { fTimeoutTask.RefreshTimeout(); }

    // In order to facilitate sending out of band data on the TCP connection,
    // other objects need to have direct pointer access to this object. But,
    // because this object is a task object it can go away at any time. If # of
    // object holders is > 0, the TCPSession will NEVER go away. However,
    // the object managing the session should be aware that if IsLiveSession returns
    // false it may be wise to relinquish control of the session
    void IncrementObjectHolderCount() { (void)atomic_add(&fObjectHolders, 1); }
    void DecrementObjectHolderCount();
    
    
    //Two main things are persistent through the course of a session, not
    //associated with any one request. The RequestStream (which can be used for
    //getting data from the client), and the socket. OOps, and the ResponseStream
    TCPSslRequestStream*	GetInputStream()    { return &fInputStream; }
    SslTCPClientSocket*          GetSocket()         { return &fSocket; }
    OSMutex*            GetSessionMutex()   { return &fSessionMutex; }
    
    UInt32              GetSessionID()      { return fSessionID; }
    
    // ZQ STREAM FUNCTIONS
    
    // Allows non-buffered writes to the client. These will flow control.
    
    // THE FIRST ENTRY OF THE IOVEC MUST BE BLANK!!!
    virtual ZQ_Error Read(SSL *m_pssl,void* ioBuffer, UInt32 inLength, UInt32* outLenRead);   

protected:
	enum
	{
		kMaxUserNameLen         = 32,
		kMaxUserPasswordLen     = 32,
		kMaxUserRealmLen        = 64
	};
    char                fUserNameBuf[kMaxUserNameLen];
    char                fUserPasswordBuf[kMaxUserPasswordLen];
    char                fUserRealmBuf[kMaxUserRealmLen];

    TimeoutTask         fTimeoutTask;//allows the session to be timed out
    
    TCPSslRequestStream    fInputStream;
    
    // Any RTP session sending interleaved data on this TCP session must
    // be prevented from writing while an TCP request is in progress
    OSMutex             fSessionMutex;

    //+rt  socket we get from "accept()"
    SslTCPClientSocket           fSocket;
    SslTCPClientSocket*          fInputSocketP;  // <-- usually same as fSocketP, unless we're Proxying
    
    // What session type are we?
    unsigned int        fObjectHolders;

	//Each TCP session has a unique number that identifies it.
    UInt32              fSessionID;
    UInt32              fLocalAddr;
    UInt32              fRemoteAddr;
    
    UInt16              fLocalPort;
    UInt16              fRemotePort;
		
    static unsigned int		sSessionIDCounter;
};
#endif // __TCPSSLSESSIONINTERFACE_H__

