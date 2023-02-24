#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

int main() 
{
	float FPS = 30.0;
	
	bool isDiscardData = true;
	int countDiscard = 0;
	
	std::cout << "PPG algorithm"<<std::endl;
	
	cv::VideoCapture cap;
	cap.open(0);
	if (!cap.isOpened())
	{
		std::cerr << "[ERROR] Unable to open camera!" << std::endl;
		return -2;
	}
	if (isDiscardData)
	{
		countDiscard++;
		if (countDiscard == DISCARD_DURATION*FPS)
		isDiscardData = false;
	}
	else
	{
		
		while (true)
		{
			// create a matrix to store the image from the cam
			cv::Mat frame;
			// wait for a new frame from camera and store it into 'frame'
			cap.read(frame);
			// check if we succeeded
			if (frame.empty())
			{
				std::cerr << "[ERROR] blank frame grabbed" << std::endl;
				break;
			}
			cv::imshow("Color", frame);
			if (cv::waitKey(1000.0/FPS) >= 0)
			break;
		}
	}
	
	return 0;
} 