#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

float FPS = 15.0;
bool isDiscardData = true; 
int countDiscard = 0;
float DISCARD_DURATION = 2.0;
float BUFFER_DURATION = 20.0;

template <typename T> 
cv::Mat plotGraph(std::vector<T>& vals, int YRange[2]) 
{ 
	auto it = minmax_element(vals.begin(), vals.end()); 
	float scale = 1./ceil(*it.second - *it.first);  
	float bias = *it.first; 
	int rows = YRange[1] - YRange[0] + 1; 
	cv::Mat image = 255*cv::Mat::ones( rows, vals.size(), CV_8UC3 ); 
	image.setTo(255); 
	for (int i = 0; i < (int)vals.size()-1; i++) 
	{ 
		cv::line(image, cv::Point(i, rows - 1 - (vals[i] - bias)*scale*YRange[1]), cv::Point(i+1, rows - 1 - (vals[i+1] - bias)*scale*YRange[1]), cv::Scalar(255, 0, 0), 1);
	}

	return image; 
}

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
			
			std::vector<double> greenSignalNormalized; 
			cv::Scalar mean, stddev; 
			cv::meanStdDev(greenSignal, mean, stddev); 
			for (int l_sample=0; l_sample < FPS*BUFFER_DURATION; l_sample++) 
			{ 
				greenSignalNormalized.push_back((greenSignal.at<double>(0, l_sample) - mean[0])/stddev[0]); 
			}
			
			int range[2] = {0, (int)(FPS*BUFFER_DURATION)}; 
			cv::imshow("green", plotGraph(greenSignalNormalized, range));
			
			//Compute discrete fourier transform
			cv::Mat greenFFT; 
			std::vector<double> greenFFTModule; 
			 
			cv::dft(greenSignalNormalized,greenFFT,cv::DFT_ROWS|cv::DFT_COMPLEX_OUTPUT); 
			cv::Mat planes[] = {cv::Mat::zeros(greenSignalNormalized.size(),1, CV_64F), cv::Mat::zeros(greenSignalNormalized.size(),1, CV_64F)}; 
			cv::split(greenFFT, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
			greenFFTModule.clear(); 
			for (int l=0; l < planes[1].cols; l++) 
			{ 
				double moduleFFT = pow(planes[1].at<double>(0,l),2) + pow(planes[0].at<double>(0,l),2); 
				greenFFTModule.push_back(sqrt(moduleFFT)); 
			} 
			 
			// display green FFT 
			cv::imshow("FFT module green", plotGraph(greenFFTModule, range));

			cv::imshow("Color", frame); 
			
			double maxVal = 0.0;
			int maxIdx = -1;

			float freqStep = FPS / static_cast<double>(greenFFTModule.size());//to have the frequency at the end, it is the interval (Hz) between each sample of the FFT 
			float freqStart = 0.5;
			float freqEnd = 4.0; 

			// loop for the values in the interval
			for (int i = static_cast<int>(freqStart / freqStep); i < static_cast<int>(freqEnd / freqStep); i++) {
			    if (greenFFTModule[i] > maxVal) {
				maxVal = greenFFTModule[i];
				maxIdx = i;
			    }
			}

			// max frequency peak
			float maxFreq = freqStart + maxIdx * freqStep;
			std::cout << "Max frequency: " << maxFreq << " Hz" << std::endl;
			std::cout << "Max value: " << maxVal << std::endl;
			std::cout << "BPM: " << maxFreq*60 << std::endl;
			
			if (cv::waitKey(1000.0/FPS) >= 0) 
				break; 
		}
	}
}