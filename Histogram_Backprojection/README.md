# Histogram back Projection
This Python scripts track the ball , using laptops inbuilt web cam .


![ball5.png](Histogram_Backprojection/ball5.png)


The original version of this code can be found at [opencv website](https://docs.opencv.org/3.4/dc/df6/tutorial_py_histogram_backprojection.html). 
Our version written by [Shubham Singhania](https://github.com/shub19). Tries to covert a 2D histogram into a 1D histogram without much loss
of color informtion. This was motivated by the fact that 1D histogram can be computed in real time by the dsp core.

Image was first converted to HSV color space and 5 Msb bits of H and 3MSbits of V grouped to form a new 8 bit pixel
