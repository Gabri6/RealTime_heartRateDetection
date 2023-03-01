#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

float FPS = 15.0;
bool isDiscardData = true; 
int countDiscard = 0;
float DISCARD_DURATION = 2.0;
float BUFFER_DURATION = 20.0;

int main()
{
	cv::VideoCapture cap; 
	cap.open(0); 
	if (!cap.isOpened())  
	{ 
		 std::cerr << "[ERROR] Unable to open camera!" << std::endl; 
		 return -2; 
	}
	
	cv::CascadeClassifier faceDetector; 
	if( !faceDetector.load("./haarcascade_frontalface_alt.xml")) 
	{ 
		std::cerr  << "[ERROR] Unable to load face cascade" << std::endl; 
		return -1; 
	};
	
	while (true) 
	{ 
		if (isDiscardData) 
		{ 
		countDiscard++; 
		if (countDiscard == DISCARD_DURATION*FPS) 
			isDiscardData = false; 
		} 
		else 
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
			
			cv::Mat frame_gray;
			
			cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
			
			std::vector<cv::Rect> faceRectangles; 
			faceDetector.detectMultiScale(frame_gray, faceRectangles, 1.1, 3, 0,  cv::Size(20, 20));
			
			cv::rectangle(frame, faceRectangles[0], cv::Scalar(0, 0, 255), 1, 1, 0); 
			
			cv::Rect foreheadROI;  
			foreheadROI = faceRectangles[0]; 
			foreheadROI.height *= 0.3; 
			cv::rectangle(frame, foreheadROI, cv::Scalar(0, 255, 0), 1, 1, 0);
			
			cv::Mat frame_forehead = frame(foreheadROI); 
			cv::Scalar avg_forehead = mean(frame_forehead); 
			
			bool isBufferFull = false; 
			int sampleIdBuffer = 0; 
			cv::Mat greenSignal(1, FPS*BUFFER_DURATION, CV_64F); 
			 
			if (!isBufferFull) 
			{ 
				greenSignal.at<double>(0, sampleIdBuffer) = avg_forehead[1] ; 
				sampleIdBuffer++; 
			 
				if (sampleIdBuffer == FPS*BUFFER_DURATION) 
				{ 
					isBufferFull = true; 
				} 
			}
 
			cv::imshow("Color", frame); 
			
			if (cv::waitKey(1000.0/FPS) >= 0) 
				break; 
		}
	}
}