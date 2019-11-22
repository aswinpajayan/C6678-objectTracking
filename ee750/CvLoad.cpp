#include<stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <iostream>
#include <pthread.h>
#include <thread>
#include <opencv2/imgproc/imgproc.hpp>

#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions   */
#include <errno.h>
#include <string.h>
#include<math.h>

#define HEIGHT 240
#define WIDTH 320
extern "C" {
#include "UDP_client.c"
}

using namespace cv;
using namespace std;

int fd;
void serial_comm(int x,int y);
void serial_comm_init(void);

Mat frame,gray,framein;
void CallBackFunc(int event,int x,int y,int flags,void* userdata){
   if(event==EVENT_MOUSEMOVE){
      // Test if greyscale or color
      if(gray.channels()==1){
         cout << "Grey Pixel (" << x << ", " << y << "): " << (int)gray.at<uchar>(y,x) << endl;
      } else {
         cout << "RGB Pixel (" << x << ", " << y << "): " << (int)gray.at<Vec3b>(y,x)[2] << "/" << (int)gray.at<Vec3b>(y,x)[1] << "/" << (int)gray.at<Vec3b>(y,x)[0] << endl;
      }
   }
}

int main()
{
	serial_comm_init();
	unsigned int i = 0,x1,y1;
	unsigned char array[320*240];
	//VideoCapture vidIn("vid_out.mp4");
	VideoCapture vidIn(0);
	if(!vidIn.isOpened()){
		cout << "Error in opening the video" << endl;
		return -1;
	}
    	vidIn.set(cv::CAP_PROP_FRAME_WIDTH,640);
    	vidIn.set(cv::CAP_PROP_FRAME_HEIGHT,480);
	namedWindow("Frame",1);
	//std::vector<uchar> array(320*240);
//	setMouseCallback("Frame",CallBackFunc,nullptr);
	while(1){
		for(int fnum=2;fnum>0;fnum--){
		vidIn >> framein;
		resize(framein,frame,cv::Size(),0.5,0.5);
		//cout << "looping" << endl;
		//imshow("Frame",frame);
		}
		if(frame.empty())
			break;
		cvtColor(frame,gray,COLOR_BGR2GRAY);
		//imshow("Frame",gray);
		cv::Mat flat = gray.reshape(1, 320*240);
		if(!gray.isContinuous()) {
        		flat = flat.clone();}
		uchar * ptr = flat.data;
		
		//if (gray.isContinuous())
	    		//array = gray.data;
		for(i=0;i<NUM_OF_ROWS;i++){
			for(int j = 0; j < BUFSIZE; j++){
				//outBuf[j] = (uchar)gray.at<uchar>(i,j);
				//cout<<*(ptr+(i*320 + j))<<endl;
				outBuf[j] = *(ptr + (i* 1024 ) + j);
			}
			outBuf[1022] = (unsigned char)i&0xFF;
			outBuf[1023] = (unsigned char)((i>>8)&0xFF);
			startProcessing(i,"Testing Phase");
		}
		x1 = (inBuf[1] << 8) | inBuf[0];
		y1 = (inBuf[3] << 8) | inBuf[2];
		circle(frame, Point(x1,y1), 25, Scalar(0,0,255), 3, 8); 
		serial_comm(x1,y1);
		imshow("Frame",frame);
		//cout << "Waiting for Keypress" << endl;	
		waitKey(1);
	}
    return 0;
}

void serial_comm_init(void)
{
//int fd;/*File Descriptor*/

printf("\n +----------------------------------+");
printf("\n |        Serial Port Write         |");
printf("\n +----------------------------------+");

/*------------------------------- Opening the Serial Port -------------------------------*/

/* Change /dev/ttyUSB0 to the one corresponding to your system */

        fd = open("/dev/ttyACM1",O_RDWR | O_NOCTTY | O_NDELAY); /* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
  /* O_RDWR Read/Write access to serial port           */
/* O_NOCTTY - No terminal will control the process   */
/* O_NDELAY -Non Blocking Mode,Does not care about-  */
/* -the status of DCD line,Open() returns immediatly */                                        

        if(fd == -1) /* Error Checking */
              printf("\n  Error! in Opening ttyUSB0  ");
        else
              printf("\n  ttyUSB0 Opened Successfully ");


/*---------- Setting the Attributes of the serial port using termios structure --------- */

struct termios SerialPortSettings; /* Create the structure                          */

tcgetattr(fd, &SerialPortSettings); /* Get the current attributes of the Serial port */

cfsetispeed(&SerialPortSettings,B115200); /* Set Read  Speed as 9600                       */
cfsetospeed(&SerialPortSettings,B115200); /* Set Write Speed as 9600                       */

SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
SerialPortSettings.c_cflag &= ~CSIZE; /* Clears the mask for setting the data size             */
SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */

SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */


SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */

SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/

if((tcsetattr(fd,TCSANOW,&SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
   printf("\n  ERROR ! in Setting attributes");
else
                    printf("\n  BaudRate =115200 \n  StopBits = 1 \n  Parity   = none");
}

void serial_comm(int x,int y)
{


       /*------------------------------- Write data to serial port -----------------------------*/

int8_t ind=0,len=log10(x)+1;
int8_t ind2=0,len2=0;
char write_buffer[15] = {'X',0,0,0,0,0,0,0,0,0,0,0,0,0,0};
for(ind=len;ind>=1;ind--)
{
write_buffer[ind]=x%10+'0';
x=x/10;
}

write_buffer[++len]='Y';
len2=len+log10(y)+1;
ind2=len2;

for(;ind2>=len+1;ind2--)

{
write_buffer[ind2]=y%10 + '0';
y=y/10;
}
write_buffer[++len2]='\r';


/* Buffer containing characters to write into port     */
int  bytes_written  = 0;   /* Value for storing the number of bytes written to the port */

//bytes_written = write(fd,write_buffer,sizeof(write_buffer));/* use write() to send data to port
bytes_written = write(fd,write_buffer,++len2);
    /* "fd"                   - file descriptor pointing to the opened serial port */
    /* "write_buffer"         - address of the buffer containing data            */
    /* "sizeof(write_buffer)" - No of bytes to write                               */
// printf("\n  %s written to ttyUSB0",write_buffer);
// printf("\n  %d Bytes written to ttyUSB0", bytes_written);
// printf("\n +----------------------------------+\n\n");

// close(fd);/* Close the Serial port */




}


