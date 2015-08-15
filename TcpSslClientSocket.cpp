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
    File:       ClientSocket.cpp

    
    
*/

#include "TcpSslClientSocket.h"

SslTcpClientSocket::SslTcpClientSocket(UInt32 inSocketType)
 : fSocket()
{
    //
    // It is necessary to open the socket right when we construct the
    // object because the QTSSSplitterModule that uses this class uses
    // the socket file descriptor in the QTSS_CreateStreamFromSocket call.
    fSocketP = &fSocket;
    this->Open(&fSocket);
}

void SslTcpClientSocket::SetOptions(int sndBufSize,int rcvBufSize)
{   //set options on the socket

    //qtss_printf("TCPClientSocket::SetOptions sndBufSize=%d,rcvBuf=%d,keepAlive=%d,noDelay=%d\n",sndBufSize,rcvBufSize,(int)keepAlive,(int)noDelay);
    int err = 0;
    err = ::setsockopt(fSocket.GetSocketFD(), SOL_SOCKET, SO_SNDBUF, (char*)&sndBufSize, sizeof(int));
    AssertV(err == 0, OSThread::GetErrno());

    err = ::setsockopt(fSocket.GetSocketFD(), SOL_SOCKET, SO_RCVBUF, (char*)&rcvBufSize, sizeof(int));
    AssertV(err == 0, OSThread::GetErrno());

#if __FreeBSD__ || __MacOSX__
    struct timeval time;
    //int len = sizeof(time);
    time.tv_sec = 0;
    time.tv_usec = 0;

    err = ::setsockopt(fSocket.GetSocketFD(), SOL_SOCKET, SO_RCVTIMEO, (char*)&time, sizeof(time));
    AssertV(err == 0, OSThread::GetErrno());

    err = ::setsockopt(fSocket.GetSocketFD(), SOL_SOCKET, SO_SNDTIMEO, (char*)&time, sizeof(time));
    AssertV(err == 0, OSThread::GetErrno());
#endif

}

void	SslTcpClientSocket::InitializeParam(StrPtrLen	addr,short port,StrPtrLen	password,StrPtrLen	path,StrPtrLen	cert,StrPtrLen	key)
{
	fSocket.SetAddr(addr);
	fSocket.SetPort(port);
	fSocket.SetPassword(password);
	fSocket.SetCert(cert);
	fSocket.SetKey(key);
	fSocket.SetPath(path);
}

OS_Error SslTcpClientSocket::SendV(iovec* inVec, UInt32 inNumVecs)
{
    if (fSendBuffer.Len == 0)
    {
        for (UInt32 count = 0; count < inNumVecs; count++)
        {
            ::memcpy(fSendBuffer.Ptr + fSendBuffer.Len, inVec[count].iov_base, inVec[count].iov_len);
            fSendBuffer.Len += inVec[count].iov_len;
            Assert(fSendBuffer.Len < ClientSocket::kSendBufferLen);
        }
    }
    
    OS_Error theErr; //= this->Connect(&fSocket);
    //if (theErr != OS_NoErr)
    //    return theErr;
        
	theErr = this->SendSendBuffer(&fSocket);
	if(OS_NoErr == theErr)// Send success, so listen it.
		fSocket.RequestEvent(EV_RE);

	if(theErr != OS_NoErr && !fSocket.IsConnected())
		fSocket.Cleanup();
    return theErr;
}
            
OS_Error SslTcpClientSocket::Read(void* inBuffer, const UInt32 inLength, UInt32* outRcvLen)
{
    //this->Connect(&fSocket);
    OS_Error theErr = fSocket.Read(inBuffer, inLength, outRcvLen);
    if (theErr != OS_NoErr)
        fEventMask = EV_RE;

	if(!fSocket.IsConnected())
	{
		fSocket.Cleanup();
	}
    return theErr;
}

