# C6678-objectTracking
This document provides a step by step procedure to run this project. 

This project contains 3 portions.
* Code for C6678 EVM 
* Host Side Code
* Code for Tiva C to control servo motor.(you can add this directly in C6678 as well.)

# Host Side Code 
This is a simple C++,C program that 
	Reads video from a connected USB web camera ,splits the video frame in to chunks of 1024 bytes. Then adds header information(packet number) encoded in last two bytes of the packet. And sends the data over to C6678 by opening a UDP connection. 
## __Requirements__
* Any linux based operating system (Tested on Ubuntu 18.04)
* OpenCV Cpp libaries (prefereably builded from source). 
This [webpage from codebind](http://www.codebind.com/cpp-tutorial/install-opencv-ubuntu-cpp/) provides a comprehensive yet working intstructions to build and install opencv from source. We had to add one extra flag when running CMAKE `-D OPENCV_GENERATE_PKGCONFIG=ON` This ensures that you can use `pkg-config --cflags --libs opencv4` in the Makefile.
### __Building and Running the code__
* Extract the code to any folder anywhere in your machine, please note that all the files need to be in the same folder 
* Edit the line  `#define SERVER_IP "10.107.2.126` to give any ip address which you can acess, or add the same address in C6678 and connect C6678 directly to PC
* By default C6678 uses port 7 `#define PORT 7 ` . You might have to have sudo permission to use this port. Change the port to some number greater than 1024 to avoid using sudo
* To build the project run the command `make` , `make clean` to clean all the build files . 
* Run the executable by entering `sudo ui` in the terminal

### __Debugging Information__
* In the file CvLoad.cpp you might have to change the camera device number `VideoCapture vidIn(1);` you can find out the connected camera number by the stub file for video `/dev/video1` or `/dev/video2` created once the camera is plugged in 
* If you are using uart to communicate to TIVA board check wether you are using the correct tty in CvLoad.cpp `fd = open("/dev/ttyACM0",O_RDWR | O_NOCTTY | O_NDELAY);` 
Code for serial communication was taken from github repository [xanthium-eneterprises](https://github.com/xanthium-enterprises/Serial-Port-Programming-on-Linux) 
# Code for C6678
* Install CCS and MCSDK from the CD which came along with evm 6678
* If you are installing CCS and MCSDK downloaded from internet you might have to edit some xml files, so that eclipse can detect the installation of MCSDK components
* Inside ccs, goto `Projects->Import CCS Projects`, select the project in `ti/mcsdk_2_01_02_05/examples/ndk/helloWorld/`
* Replace the files udpHello.c,helloWorld.c and hellowWorld.cfg files with the files given in our project
* Edit the server address and port in helloWorld.c if you want to.
* Create a new targetconfiguration. In advanced tab, add  `ti/ccsv5/ccs_base/emulation/boards/evmc6678l/gel/evmc6678l.gel` to core 0
* connect the board. Press the hard reset button on the board. Run the code(choose only single core (core 0)). Each time before running the code, you will have to hard reset the board.
### __Debugging__
* inserting printf statements are the easiest way to debug the code. there are multiple files functions provided in the helloWorld.c code for debugging. In particular, you can switch on the LED blink task by uncommeting the corresponding tasks in the `main()` function in helloWorld.c file. You will have to add $IMG_LIB_ROOT to your build path if you are using the  
* We used wireshark to debug incoming and outgoing UDP packets. `tcpdump` is a cooler option
* processImage() function does a sobel operation on a image stored in computer
* octave/Matlab code for opening the .y images is given with the project
### __Short Aside__
	* Unless you dont know exactly what you are doing, dont use semaphores. Semaphores are used for mutual exclusion of two tasks. But in most of the cases, we want the threads to run in parallel. 
	* Dont use blocking calls inside tasks
	* .y files given by TI are simple binary files, you dont need a special software for that you can write a simple c code for binary file manipulation, or use matlab/octave for the same. Octave file is given in this repository. 
	* check and make sure that you have matching image dimensions everywhere.To avoid redefinition, in most places we use the `extern` keyword in c file.

# Contributors
* [Shubham Singhania](https://github.com/shub19)
* [Jinit Patil](https://github.com/jinitpatil)
* [Krishna Khairnar](https://github.com/krishnajk333)
* [Aswin P Ajayan](https://github.com/aswinpajayan)
