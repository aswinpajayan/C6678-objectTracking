/*
 * helloWorld_bios6.c
 *
 * TCP/IP Stack 'Hello World!' Example ported to use BIOS6 OS.
 *
 * Copyright (C) 2007, 2011 Texas Instruments Incorporated - http://www.ti.com/
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

//--------------------------------------------------------------------------
// IP Stack 'Hello World!' Example
//
// This is a skeleton application, intended to provide application  
// programmers with a basic Stack setup, to which they can start 
// adding their code.
//
// To test it as is, use with helloWorld.exe from \winapps directory
//

#include <stdio.h>
#include <ti/ndk/inc/netmain.h>

/* BIOS6 include */
#include <ti/sysbios/BIOS.h>

/* Platform utilities include */
#include "ti/platform/platform.h"
#include "ti/platform/resource_mgr.h"

#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>



// Libraries for image processing
#include <stdio.h>
#include <ti/imglib/src/common/IMG_profile.h>
#include <ti/imglib/src/common/IMG_memory.h>
#include <ti/imglib/src/IMG_conv_3x3_i8_c8s/c66/IMG_conv_3x3_i8_c8s.h>
#include <ti/imglib/src/IMG_conv_3x3_i8_c8s/c66/IMG_conv_3x3_i8_c8s.c>


#define M_SIZE 3
#define O_N 32
#define N (M_SIZE * O_N)
#define C_N (M_SIZE * M_SIZE)
#define PAD 64
#define SIZE (N + 2*PAD)
#define C_SIZE (C_N + 2*PAD)
#define O_SIZE (O_N + 2*PAD)


typedef unsigned char  I_type;
typedef char  C_type;
typedef unsigned char  O_type;

#define IMG_WIDTH (320)
#define IMG_HEIGHT (240)
#define IMG_SIZE (IMG_WIDTH*IMG_HEIGHT)
I_type input_1[SIZE];
C_type mask[C_SIZE];
O_type output[O_SIZE];
O_type ref_output[O_SIZE];


extern unsigned char img_input[IMG_SIZE];


unsigned char img_output[IMG_SIZE];

unsigned char img_output_ref[IMG_SIZE];




/* Platform Information - we will read it form the Platform Library */
platform_info  gPlatformInfo;


//---------------------------------------------------------------------------
// Title String
//
char *VerStr = "\nTCP/IP Stack 'Hello World!' Application\n\n";

// Our NETCTRL callback functions
static void   NetworkOpen();
static void   NetworkClose();
static void   NetworkIPAddr( IPN IPAddr, uint IfIdx, uint fAdd );

// Fun reporting function
static void   ServiceReport( uint Item, uint Status, uint Report, HANDLE hCfgEntry );

// External references
extern int dtask_udp_hello();
extern Semaphore_Handle sem, semStartProcess;
extern unsigned char *pBuf;
extern unsigned char *pBufOut;
unsigned char BufOut[14];
extern int numRecv;
extern unsigned char FLAG_START_PROCESSING;

//---------------------------------------------------------------------------
//Parallel tasks
int processImage();
int udpStart();
int blinkLED();
int thresholdImage();
//int collectImage();
//Semaphore_Handle semStartProcess;
//---------------------------------------------------------------------------
// Configuration
//
char *HostName    = "tidsp";
char *LocalIPAddr = "10.107.2.126";
char *LocalIPMask = "255.255.240.0";    // Not used when using DHCP
char *GatewayIP   = "10.107.1.250";    // Not used when using DHCP
char *DomainName  = "demo.net";         // Not used when using DHCP
char *DNSServer   = "0.0.0.0";          // Used when set to anything but zero

// Simulator EMAC Switch does not handle ALE_LEARN mode, so please configure the
// MAC address of the PC where you want to launch the webpages and initiate PING to NDK */

Uint8 clientMACAddress [6] = {0xe0, 0xd5, 0x5e, 0x70, 0xef, 0x02}; /* MAC address for my PC */

/*************************************************************************
 *  @b EVM_init()
 * 
 *  @n
 *     
 *  Initializes the platform hardware. This routine is configured to start in 
 *   the evm.cfg configuration file. It is the first routine that BIOS 
 *   calls and is executed before Main is called. If you are debugging within
 *  CCS the default option in your target configuration file may be to execute 
 *  all code up until Main as the image loads. To debug this you should disable
 *  that option. 
 *
 *  @param[in]  None
 * 
 *  @retval
 *      None
 ************************************************************************/
void EVM_init()
{
 platform_init_flags   sFlags;
 platform_init_config  sConfig;
 /* Status of the call to initialize the platform */
 int32_t pform_status;

 /* 
  * You can choose what to initialize on the platform by setting the following 
  * flags. Things like the DDR, PLL, etc should have been set by the boot loader.
 */
 memset( (void *) &sFlags,  0, sizeof(platform_init_flags));
 memset( (void *) &sConfig, 0, sizeof(platform_init_config));

    sFlags.pll  = 0; /* PLLs for clocking    */
    sFlags.ddr  = 0;     /* External memory    */
    sFlags.tcsl = 1; /* Time stamp counter   */
#ifdef _SCBP6618X_
    sFlags.phy  = 0; /* Ethernet       */
#else
    sFlags.phy  = 1; /* Ethernet       */
#endif
    sFlags.ecc  = 0; /* Memory ECC       */

    sConfig.pllm = 0;  /* Use libraries default clock divisor */

 pform_status = platform_init(&sFlags, &sConfig);

 /* If we initialized the platform okay */
 if (pform_status != Platform_EOK) {
   /* Initialization of the platform failed... die */
   while (1) {
     (void) platform_led(1, PLATFORM_LED_ON, PLATFORM_USER_LED_CLASS);
     (void) platform_delay(50000);
     (void) platform_led(1, PLATFORM_LED_OFF, PLATFORM_USER_LED_CLASS);
     (void) platform_delay(50000);
   }
 }

}

//---------------------------------------------------------------------
// Main Entry Point
//---------------------------------------------------------------------
int main()
{
	bzero(img_output,IMG_SIZE);
	bzero(img_input,IMG_SIZE);
	FLAG_START_PROCESSING = 0;
	sem = Semaphore_create(1, NULL, NULL);
	semStartProcess = Semaphore_create(1, NULL, NULL);
	Task_Handle tskUDP,tskProcess,tskStatus,tskCollect;
    Task_Params taskParams;

    /* Create UDP task to recieve packets*/
	Task_Params_init(&taskParams);
	taskParams.priority = 1;
	//tskStatus = Task_create (blinkLED, &taskParams, NULL);
    Task_Params_init(&taskParams);
    taskParams.priority = 5;
    tskUDP = Task_create (udpStart, &taskParams, NULL);
	Task_Params_init(&taskParams);
	taskParams.priority = 4;
	//tskCollect = Task_create (collectImage, &taskParams, NULL);
    Task_Params_init(&taskParams);
    taskParams.priority = 4;
    //tskProcess = Task_create (thresholdImage, &taskParams, NULL);
    
    /* Start the BIOS 6 Scheduler */

    BIOS_start ();
}
int thresholdImage(){
	 FILE* fp;
    const char mask_ptr[9] = {  -1, 0, 1, -1,  0,  1, -1,  0,  1};
    long long t0,t1;
	while(1){
		if(FLAG_START_PROCESSING == 0){
			Task_sleep(5);
			continue;
		}
		Semaphore_pend(semStartProcess,BIOS_WAIT_FOREVER);

    	t0 = _itoll(TSCH,TSCL);
		printf("started Processing\n");
		int i1,i2,x,y,k;
    	x=0,y=0,k=0;
    	for (i1=0;i1<IMG_HEIGHT;i1++)
    	{
    	    for(i2=0;i2<IMG_WIDTH;i2++)
    	    {
    	        if (img_input[i1* IMG_WIDTH + i2]<100 || img_input[i1* IMG_WIDTH + i2]> 255)
    	            {
    	                img_output[i1* IMG_WIDTH + i2]=0;
    	               // printf("%d,%d\n",i1,i2);
    	            }
    	            else
    	            {
    	                img_output[i1* IMG_WIDTH + i2]=255;
    	                x=x+i1;
    	                y=y+i2;
    	                k=k+1;
    	            }

    	    }
    	}
    	x=(x/k);
    	y=(y/k);
    	//printf("coordinate of mid point (x,y)=(%d, %d)",x,y);
		BufOut[0] = (unsigned char)(x&0xFF);
		BufOut[1] = (unsigned char)((x>>8)&0xFF);
		BufOut[2] = (unsigned char)(y&0xFF);
		BufOut[3] = (unsigned char)((y>>8)&0xFF);
		//printf("coordinate of mid point send(x,y)=(%d, %d)",((BufOut[1] << 8)) | BufOut[0], BufOut[3] << 8 | BufOut[2]);


		t1 = _itoll(TSCH,TSCL);

    	printf("cycle consumed per pixel in optimized c code is %lld \n",(t1-t0));
    	


    	fp =  fopen("/home/aswin/Downloads/TI_workshop_content/outBall.y","wb");

    	if(fp == NULL)
    	  printf("output file could not be opened");
    	
    	//fwrite(img_output,1,IMG_SIZE,fp);
    	
    	fclose(fp);
		bzero(img_output,IMG_SIZE);
    	printf("processing completed");
		FLAG_START_PROCESSING = 0;
		Semaphore_post(semStartProcess);
	}


}
int processImage(){
    FILE* fp;
    const char mask_ptr[9] = {  -1, 0, 1, -1,  0,  1, -1,  0,  1};
    long long t0,t1;
	while(1){
		printf("From Processing\n");
		if(FLAG_START_PROCESSING == 0){
			Task_sleep(5);
			continue;
		}
		Semaphore_pend(semStartProcess,BIOS_WAIT_FOREVER);

    	t0 = _itoll(TSCH,TSCL);
    	IMG_conv_3x3_i8_c8s (img_input, img_output, IMG_WIDTH*(IMG_HEIGHT-1) , IMG_WIDTH, mask_ptr, 3);
    	t1 = _itoll(TSCH,TSCL);
		printf("started Processing\n");

    	printf("cycle consumed per pixel in optimized c code is %lld \n",(t1-t0));
    	


    	fp =  fopen("/home/aswin/Downloads/TI_workshop_content/outBall.y","wb");

    	if(fp == NULL)
    	  printf("output file could not be opened");
    	
    	fwrite(img_output,1,IMG_SIZE,fp);
    	
    	fclose(fp);
    	printf("processing completed");
		FLAG_START_PROCESSING = 0;
		Semaphore_post(semStartProcess);
	}

}
int collectImage2(){
	unsigned long int i =0,offset,recvRow=0 ;
	static int rowNum = IMG_HEIGHT + 4;
	static unsigned char startFlag= 0;
	while(1){
		if(numRecv>0 && startFlag == 0){
			startFlag = 0xFF;
			Semaphore_pend(semStartProcess,BIOS_WAIT_FOREVER);
		}else if(numRecv<1 && startFlag == 0){
			continue;
		}
		Semaphore_pend(sem,BIOS_WAIT_FOREVER);
		recvRow = (pBuf[IMG_WIDTH + 1] << 8) | (pBuf[IMG_WIDTH]);
		offset = recvRow * IMG_WIDTH;
 		printf("rowNum : %lu recvd : %lu\n",rowNum,recvRow);	
		for(i=0;i<IMG_WIDTH;i++){
			img_input[offset + i] = pBuf[i];
		}
		rowNum++;
		if(rowNum >= IMG_HEIGHT || (recvRow >IMG_HEIGHT -2)){
			rowNum = 0;
			startFlag =0;
			numRecv = 0;
			Semaphore_post(semStartProcess);
			recvncfree(pBuf);
		}
		Semaphore_post(sem);
		Task_sleep(10);
	}
}

int blinkLED(){
    while(1){
     /* Get access to resource */
    //Semaphore_pend(sem, BIOS_WAIT_FOREVER);
    (void) platform_led(1, PLATFORM_LED_ON, PLATFORM_USER_LED_CLASS);
    (void) platform_delay(50000);
    (void) platform_led(1, PLATFORM_LED_OFF, PLATFORM_USER_LED_CLASS);
    (void) platform_delay(50000);
    //Semaphore_post(sem);
    }
}

//
// Main Thread
//
int udpStart()
{
    int             rc;
    int        i;
    HANDLE          hCfg;
    QMSS_CFG_T      qmss_cfg;
    CPPI_CFG_T      cppi_cfg;

 /* Get information about the platform so we can use it in various places */
 memset( (void *) &gPlatformInfo, 0, sizeof(platform_info));
 (void) platform_get_info(&gPlatformInfo);

 (void) platform_uart_init();
 (void) platform_uart_set_baudrate(115200);
 (void) platform_write_configure(PLATFORM_WRITE_ALL);

 /* Clear the state of the User LEDs to OFF */
 for (i=0; i < gPlatformInfo.led[PLATFORM_USER_LED_CLASS].count; i++) {
   (void) platform_led(i, PLATFORM_LED_OFF, PLATFORM_USER_LED_CLASS);
 }

    /* Initialize the components required to run this application:
     *  (1) QMSS
     *  (2) CPPI
     *  (3) Packet Accelerator
     */
    /* Initialize QMSS */
    if (platform_get_coreid() == 0)
    {
        qmss_cfg.master_core        = 1;
    }
    else
    {
        qmss_cfg.master_core        = 0;
    }
    qmss_cfg.max_num_desc       = MAX_NUM_DESC;
    qmss_cfg.desc_size          = MAX_DESC_SIZE;
    qmss_cfg.mem_region         = Qmss_MemRegion_MEMORY_REGION0;
    if (res_mgr_init_qmss (&qmss_cfg) != 0)
    {
        platform_write ("Failed to initialize the QMSS subsystem \n");
        goto main_exit;
    }
    else
    {
     platform_write ("QMSS successfully initialized \n");
    }

    /* Initialize CPPI */
    if (platform_get_coreid() == 0)
    {
        cppi_cfg.master_core        = 1;
    }
    else
    {
        cppi_cfg.master_core        = 0;
    }
    cppi_cfg.dma_num            = Cppi_CpDma_PASS_CPDMA;
    cppi_cfg.num_tx_queues      = NUM_PA_TX_QUEUES;
    cppi_cfg.num_rx_channels    = NUM_PA_RX_CHANNELS;
    if (res_mgr_init_cppi (&cppi_cfg) != 0)
    {
        platform_write ("Failed to initialize CPPI subsystem \n");
        goto main_exit;
    }
    else
    {
     platform_write ("CPPI successfully initialized \n");
    }


    if (res_mgr_init_pass()!= 0) {
        platform_write ("Failed to initialize the Packet Accelerator \n");
        goto main_exit;
    }
    else
    {
     platform_write ("PA successfully initialized \n");
    }

    //
    // THIS MUST BE THE ABSOLUTE FIRST THING DONE IN AN APPLICATION before
    //  using the stack!!
    //
    rc = NC_SystemOpen( NC_PRIORITY_LOW, NC_OPMODE_INTERRUPT );
    if( rc )
    {
        platform_write("NC_SystemOpen Failed (%d)\n",rc);
        for(;;);
    }

    // Print out our banner
    platform_write(VerStr);

    //
    // Create and build the system configuration from scratch.
    //

    // Create a new configuration
    hCfg = CfgNew();
    if( !hCfg )
    {
        platform_write("Unable to create configuration\n");
        goto main_exit;
    }

    //
    // THIS MUST BE THE ABSOLUTE FIRST THING DONE IN AN APPLICATION!!
    //
    rc = NC_SystemOpen( NC_PRIORITY_LOW, NC_OPMODE_INTERRUPT );
    if( rc )
    {
        printf("NC_SystemOpen Failed (%d)\n",rc);
        for(;;);
    }

    // Print out our banner
    printf(VerStr);

    //
    // Create and build the system configuration from scratch.
    //

    // Create a new configuration
    hCfg = CfgNew();
    if( !hCfg )
    {
        printf("Unable to create configuration\n");
        goto main_exit;
    }

    // We better validate the length of the supplied names
    if( strlen( DomainName ) >= CFG_DOMAIN_MAX ||
        strlen( HostName ) >= CFG_HOSTNAME_MAX )
    {
        printf("Names too long\n");
        goto main_exit;
    }

    // Add our global hostname to hCfg (to be claimed in all connected domains)
    CfgAddEntry( hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_HOSTNAME, 0,
                 strlen(HostName), (UINT8 *)HostName, 0 );

    // If the IP address is specified, manually configure IP and Gateway
#if defined(_SCBP6618X_) || defined(_EVMTCI6614_) || defined(DEVICE_K2H) || defined(DEVICE_K2K)
    /* SCBP6618x, EVMTCI6614, EVMK2H, EVMK2K always uses DHCP */
    if (0)
#else
    if (!platform_get_switch_state(1))
#endif
    {
        CI_IPNET NA;
        CI_ROUTE RT;
        IPN      IPTmp;

        // Setup manual IP address
        bzero( &NA, sizeof(NA) );
        NA.IPAddr  = inet_addr(LocalIPAddr);
        NA.IPMask  = inet_addr(LocalIPMask);
        strcpy( NA.Domain, DomainName );
        NA.NetType = 0;

        // Add the address to interface 1
        CfgAddEntry( hCfg, CFGTAG_IPNET, 1, 0,
                           sizeof(CI_IPNET), (UINT8 *)&NA, 0 );

        // Add the default gateway. Since it is the default, the
        // destination address and mask are both zero (we go ahead
        // and show the assignment for clarity).
        bzero( &RT, sizeof(RT) );
        RT.IPDestAddr = 0;
        RT.IPDestMask = 0;
        RT.IPGateAddr = inet_addr(GatewayIP);

        // Add the route
        CfgAddEntry( hCfg, CFGTAG_ROUTE, 0, 0,
                           sizeof(CI_ROUTE), (UINT8 *)&RT, 0 );

        // Manually add the DNS server when specified
        IPTmp = inet_addr(DNSServer);
        if( IPTmp )
            CfgAddEntry( hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER,
                         0, sizeof(IPTmp), (UINT8 *)&IPTmp, 0 );
    }
    // Else we specify DHCP
    else
    {
        CI_SERVICE_DHCPC dhcpc;

        // Specify DHCP Service on IF-1
        bzero( &dhcpc, sizeof(dhcpc) );
        dhcpc.cisargs.Mode   = CIS_FLG_IFIDXVALID;
        dhcpc.cisargs.IfIdx  = 1;
        dhcpc.cisargs.pCbSrv = &ServiceReport;
        CfgAddEntry( hCfg, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT, 0,
                     sizeof(dhcpc), (UINT8 *)&dhcpc, 0 );
    }

    //
    // Configure IPStack/OS Options
    //

    // We don't want to see debug messages less than WARNINGS
    rc = DBG_WARN;
    CfgAddEntry( hCfg, CFGTAG_OS, CFGITEM_OS_DBGPRINTLEVEL,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

    //
    // This code sets up the TCP and UDP buffer sizes
    // (Note 8192 is actually the default. This code is here to
    // illustrate how the buffer and limit sizes are configured.)
    //

    // UDP Receive limit
    rc = 8192;
    CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_SOCKUDPRXLIMIT,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

    //
    // Boot the system using this configuration
    //
    // We keep booting until the function returns 0. This allows
    // us to have a "reboot" command.
    //
    do
    {
        rc = NC_NetStart( hCfg, NetworkOpen, NetworkClose, NetworkIPAddr );
    } while( rc > 0 );

    // Delete Configuration
    CfgFree( hCfg );

    // Close the OS
main_exit:
    NC_SystemClose();
    return(0);
}


//
// System Task Code [ Server Daemon Servers ]
//
static HANDLE hHello=0;

//
// NetworkOpen
//
// This function is called after the configuration has booted
//
static void NetworkOpen()
{
    // Create our local server
    hHello = DaemonNew( SOCK_DGRAM, 0, 7, dtask_udp_hello,
                       OS_TASKPRINORM, OS_TASKSTKNORM, 0, 1 );
}

//
// NetworkClose
//
// This function is called when the network is shutting down,
// or when it no longer has any IP addresses assigned to it.
//
static void NetworkClose()
{
    DaemonFree( hHello );
}


//
// NetworkIPAddr
//
// This function is called whenever an IP address binding is
// added or removed from the system.
//
static void NetworkIPAddr( IPN IPAddr, uint IfIdx, uint fAdd )
{
    IPN IPTmp;

    if( fAdd )
        printf("Network Added: ");
    else
        printf("Network Removed: ");

    // Print a message
    IPTmp = ntohl( IPAddr );
    printf("If-%d:%d.%d.%d.%d\n", IfIdx,
            (UINT8)(IPTmp>>24)&0xFF, (UINT8)(IPTmp>>16)&0xFF,
            (UINT8)(IPTmp>>8)&0xFF, (UINT8)IPTmp&0xFF );
}

//
// Service Status Reports
//
// Here's a quick example of using service status updates
//
static char *TaskName[]  = { "Telnet","HTTP","NAT","DHCPS","DHCPC","DNS" };
static char *ReportStr[] = { "","Running","Updated","Complete","Fault" };
static char *StatusStr[] = { "Disabled","Waiting","IPTerm","Failed","Enabled" };
static void ServiceReport( uint Item, uint Status, uint Report, HANDLE h )
{
    printf( "Service Status: %-9s: %-9s: %-9s: %03d\n",
            TaskName[Item-1], StatusStr[Status],
            ReportStr[Report/256], Report&0xFF );

    //
    // Example of adding to the DHCP configuration space
    //
    // When using the DHCP client, the client has full control over access
    // to the first 256 entries in the CFGTAG_SYSINFO space.
    //
    // Note that the DHCP client will erase all CFGTAG_SYSINFO tags except
    // CFGITEM_DHCP_HOSTNAME. If the application needs to keep manual
    // entries in the DHCP tag range, then the code to maintain them should
    // be placed here.
    //
    // Here, we want to manually add a DNS server to the configuration, but
    // we can only do it once DHCP has finished its programming.
    //
    if( Item == CFGITEM_SERVICE_DHCPCLIENT &&
        Status == CIS_SRV_STATUS_ENABLED &&
        (Report == (NETTOOLS_STAT_RUNNING|DHCPCODE_IPADD) ||
         Report == (NETTOOLS_STAT_RUNNING|DHCPCODE_IPRENEW)) )
    {
        IPN IPTmp;

        // Manually add the DNS server when specified
        IPTmp = inet_addr(DNSServer);
        if( IPTmp )
            CfgAddEntry( 0, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER,
                         0, sizeof(IPTmp), (UINT8 *)&IPTmp, 0 );
    }
}


