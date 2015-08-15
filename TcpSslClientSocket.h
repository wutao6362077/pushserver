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
    File:       ClientSocket.h

    
    
*/


#ifndef __TCP_SSL_CLIENT_SOCKET__
#define __TCP_SSL_CLIENT_SOCKET__

#include "SslTcpSocket.h"
#include "ClientSocket.h"

class SslTcpClientSocket : public ClientSocket
{
    public:
        
		SslTcpClientSocket(UInt32 inSocketType=Socket::kNonBlockingSocketType);
        virtual ~SslTcpClientSocket() {}
      	void 				InitializeParam(StrPtrLen	addr,short port,StrPtrLen password,StrPtrLen	path,StrPtrLen	cert,StrPtrLen	key);
		virtual void		SetTask(Task *task){fSocket.SetTask(task);};
		virtual void		SetListener(int theMask){fSocket.RequestEvent(theMask);};
        //
        // Implements the ClientSocket Send and Receive interface for a TCP connection
        virtual OS_Error    SendV(iovec* inVec, UInt32 inNumVecs);
        virtual OS_Error    Read(void* inBuffer, const UInt32 inLength, UInt32* outRcvLen);

        virtual UInt32  GetLocalAddr() { return fSocket.GetLocalAddr(); }
        virtual void    SetRcvSockBufSize(UInt32 inSize) { fSocket.SetSocketRcvBufSize(inSize); }
        virtual void    SetOptions(int sndBufSize = 8192,int rcvBufSize=1024);
        
        virtual UInt16  GetLocalPort() { return fSocket.GetLocalPort(); }
        
 //   private:
    
        SslTcpSocket   fSocket;
};

#endif //__CLIENT_SOCKET__
