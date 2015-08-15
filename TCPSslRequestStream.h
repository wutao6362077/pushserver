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
    File:       TCPRequestStream.h

    Contains:   Provides a stream abstraction for TCP. Given a socket, this object
                can read in data until an entire TCP request header is available.
                (do this by calling ReadRequest). It handles TCP pipelining (request
                headers are produced serially even if multiple headers arrive simultaneously),
                & TCP request data.
                    
*/

#ifndef __TCPSSLREQUESTSTREAM_H__
#define __TCPSSLREQUESTSTREAM_H__


//INCLUDES
#include "StrPtrLen.h"
#include "SslTcpClientSocket.h"
#include "ZQTypes.h"

class TCPSslRequestStream
{
public:

    //CONSTRUCTOR / DESTRUCTOR
    TCPSslRequestStream(SslTCPClientSocket* sock);
    
    // We may have to delete this memory if it was allocated due to base64 decoding
    ~TCPSslRequestStream() {}

    //ReadRequest
    //This function will not block.
    //Attempts to read data into the stream, stopping when we hit the EOL - EOL that
    //ends an TCP header.
    //
    //Returns:          ZQ_NoErr:     Out of data, haven't hit EOL - EOL yet
    //                  ZQ_RequestArrived: full request has arrived
    //                  E2BIG: ran out of buffer space
    //                  ZQ_RequestFailed: if the client has disconnected
    //                  EINVAL: if we are base64 decoding and the stream is corrupt
    //                  ZQ_OutOfState: 
    ZQ_Error    ReadRequest(SSL 			*m_pssl);
    

	ZQ_Error	Read(SSL *m_pssl,void* ioBuffer, UInt32 inBufLen, UInt32* outLengthRead);
    
    // Use a different TCPSocket to read request data 
    // this will be used by TCPSessionInterface::SnarfInputSocket
    void        AttachToSocket(SslTCPClientSocket* sock) { fSslSocket = sock; }
    
    void        ShowTCP(Bool16 enable) {fPrintTCP = enable; }     

	//GetRequestBuffer
    //This returns a buffer containing the full client request. The length is set to
    //the exact length of the request headers. This will return NULL UNLESS this object
    //is in the proper state (has been initialized, ReadRequest has been called until it returns
    //RequestArrived).
    char*	GetRequestBuffer()  { return fRequestBuffer; }
	UInt32	GetRequsetLength()  { return fCurOffset;};		
protected:
	//CONSTANTS:
    enum
    {
        kRequestBufferSizeInBytes = 2048        //UInt32
    };

protected:
    SslTCPClientSocket*              fSslSocket;
    
    char                    fRequestBuffer[kRequestBufferSizeInBytes];
    UInt32                  fCurOffset; // tracks how much valid data is in the above buffer
   
    Bool16                  fPrintTCP;     // debugging printfs
    
};

#endif
