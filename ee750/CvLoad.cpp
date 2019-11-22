#include<stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <iostream>
#include <pthread.h>
#include <thread>
#include <opencv2/imgproc/imgproc.hpp>

#define HEIGHT 240
#define WIDTH 320
extern "C" {
#include "UDP_client.c"
}

using namespace cv;
using namespace std;

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
		imshow("Frame",frame);
		//cout << "Waiting for Keypress" << endl;	
		waitKey(1);
	}
    return 0;
}

