/*
 * udpHello.c
 *
 * This program implements a UDP echo server, which echos back any
 * input it receives.
 *
 * Copyright (C) 2007 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include <ti/ndk/inc/netmain.h>
#include <stdio.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>

#define IMG_WIDTH (320)
#define IMG_HEIGHT (240)
#define IMG_SIZE (IMG_WIDTH*IMG_HEIGHT)
unsigned char *pBuf; //buffer to recieve data
unsigned char *pBufOut; //buffer to recieve data
Semaphore_Handle sem,semStartProcess;
unsigned char img_input[IMG_SIZE];
int X0,Y0,num;
unsigned char FLAG_START_PROCESSING;
extern unsigned char BufOut[14];
int numRecv;
int collectImage(){
	unsigned long int i =0,offset,recvRow=0,index ;
	static int rowNum = 0;
	static unsigned char startFlag= 0;
		if(startFlag == 0){
			startFlag = 0xFF;
			X0 = 0;
			Y0 = 0;
			num = 0;
			Semaphore_pend(semStartProcess,BIOS_WAIT_FOREVER);
		}
		recvRow = (pBuf[1023] << 8) | (pBuf[1022]);
		offset = recvRow * 1024;
		//printf("rowNum : %lu recvd : %lu recvCount : %d\n",rowNum,recvRow,numRecv);	
		for(i=0;i<1024;i++){
			img_input[offset + i] = pBuf[i];
			if(pBuf[i] < 50){
				index = offset+i;
				X0 += index%320;
				Y0 += (int)index/320;
				//printf("%ld : %lld : %lld\n",index,Y0,X0);
				num++;
			}
		}
		rowNum++;
		if(rowNum >= IMG_HEIGHT || (recvRow >IMG_HEIGHT -2)){
			rowNum = 0;
			startFlag =0;
			FLAG_START_PROCESSING = 0xFF;
			X0 = X0/num;
			Y0 = Y0/num;
			BufOut[0] = (unsigned char)(X0&0xFF);
			BufOut[1] = (unsigned char)((X0>>8)&0xFF);
			BufOut[2] = (unsigned char)(Y0&0xFF);
			BufOut[3] = (unsigned char)((Y0>>8)&0xFF);
			
			X0 = 0;
			Y0 = 0;
			num = 1;
			
			Semaphore_post(semStartProcess);
		}
}
//
// dtask_udp_hello() - UDP Echo Server Daemon Function
// (SOCK_DGRAM, port 7)
//
// Returns "1" if socket 's' is still open, and "0" if its been closed
//
int dtask_udp_hello( SOCKET s, UINT32 unused )
{
    struct sockaddr_in sin1;
    struct timeval     to;
    int                tmp;
    HANDLE             hBuffer;



    (void)unused;

    // Configure our socket timeout to be 3 seconds
    to.tv_sec  = 3;
    to.tv_usec = 0;
    setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof( to ) );
    setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof( to ) );

    for(;;){ 
    	/* Get access to resource */
        tmp = sizeof( sin1 );
	numRecv = (int)recvncfrom( s, (void **)&pBuf, 0, (PSA)&sin1, &tmp, &hBuffer );

        // Spit any data back out
        if( numRecv >= 0 ){ 
		//printf("coordinate of mid point send(x,y)=(%d, %d)\n",(BufOut[1] << 8) | BufOut[0], BufOut[3] << 8 | BufOut[2]);
            sendto( s, BufOut, 14, 0, (PSA)&sin1, sizeof(sin1) );
	    collectImage();
            recvncfree( hBuffer);
            //puts(pBuf);
        }
        else
            break;
    }

    // Since the socket is still open, return "1"
    // (we need to leave UDP sockets open)
    return(1);    
}


