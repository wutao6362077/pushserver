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
    File:       TCPRequestStream.cpp

    Contains:   Implementation of TCPRequestStream class. 
                    
    

*/


#include "TCPSslRequestStream.h"
#include "StringParser.h"
#include "OSMemory.h"
#include "base64.h"
#include "OSArrayObjectDeleter.h"
#include "OS.h"

#include <errno.h>

#define READ_DEBUGGING 0

TCPSslRequestStream::TCPSslRequestStream(SslTCPClientSocket* sock)
:   fSslSocket(sock),
    fCurOffset(0),
    fPrintTCP(false)
{	
}

ZQ_Error TCPSslRequestStream::ReadRequest(SSL	*m_pssl)
{
	fCurOffset = 0;
    while (true)
    {
        int newOffset = 0;
        // We don't have any new data, get some from the socket...
        ZQ_Error sockErr = fSslSocket->Read(m_pssl,&fRequestBuffer[fCurOffset], 
                                            (kRequestBufferSizeInBytes - fCurOffset) - 1, &newOffset);
        //assume the client is dead if we get an error back
        if (sockErr == EAGAIN)
            return ZQ_NoErr;
        if (sockErr != ZQ_NoErr)
        {
  //          Assert(!fSslSocket->IsConnected());
            return sockErr;
        }  

		Assert(newOffset > 0);
		fCurOffset += newOffset; 
        
        //check for a full buffer
        if (fCurOffset == kRequestBufferSizeInBytes - 1)
        {//fRequestPtr = &fRequest;
            return E2BIG;
        }

		if(fPrintTCP)
		{
			DateBuffer theDate;
			DateTranslator::UpdateDateBuffer(&theDate, 0); // get the current GMT date and time
			qtss_printf("\n\n#C->S:\n#time: ms=%"_U32BITARG_" date=%s\n", (UInt32) OS::StartTimeMilli_Int(), theDate.GetDateBuffer());
			fRequestBuffer[fCurOffset] = '\0';
			qtss_printf("%s\n", fRequestBuffer);
			if (fSslSocket != NULL && false)    
			{
				UInt16 serverPort = fSslSocket->GetLocalPort();
				UInt16 clientPort = fSslSocket->GetRemotePort();    
				StrPtrLen* theLocalAddrStr = fSslSocket->GetLocalAddrStr();
				StrPtrLen* theRemoteAddrStr = fSslSocket->GetRemoteAddrStr();
				if (theLocalAddrStr != NULL)
				{	qtss_printf("#server: ip="); theLocalAddrStr->PrintStr(); qtss_printf(" port=%u\n" , serverPort );
				}
				else
          		{	qtss_printf("#server: ip=NULL port=%u\n" , serverPort );
          		}
	           	
				if (theRemoteAddrStr != NULL)
				{	qtss_printf("#client: ip="); theRemoteAddrStr->PrintStr(); qtss_printf(" port=%u\n" , clientPort );
				}
        		else
        		{	qtss_printf("#client: ip=NULL port=%u\n" , clientPort );
        		}

				qtss_printf("%s\n", fRequestBuffer);
				//StrPtrLen str(fRequestBuffer, fCurOffset);
				//str.PrintStrEOL("\n\r\n", "\n");// print the request but stop on \n\r\n and add a \n afterwards.
			}
		}
		return ZQ_RequestArrived;
    }
}

ZQ_Error TCPSslRequestStream::Read(SSL *m_pssl,void* ioBuffer, UInt32 inBufLen, UInt32* outLengthRead)
{
    int theLengthRead = 0;
    UInt8* theIoBuffer = (UInt8*)ioBuffer;
    
    //
    // Read data directly from the socket and place it in our buffer
    int theNewOffset = 0;
    ZQ_Error theErr = fSslSocket->Read(m_pssl,&theIoBuffer[theLengthRead], inBufLen - theLengthRead, &theNewOffset);
#if READ_DEBUGGING
    qtss_printf("In HTTPRequestStream::Read: Got %d bytes off Socket\n",theNewOffset);
#endif  
    if (outLengthRead != NULL)
        *outLengthRead = theNewOffset + theLengthRead;
        
    return theErr;
}
