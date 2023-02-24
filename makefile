all: PPG
	g++ PPG.o -o PPG.exe -L /usr/lib/x86_64-linux-gnu/ -l opencv2/opencv.hpp -l opencv2/videoio.hpp -l opencv2/highgui.hpp
PPG: PPG.cpp
	g++ -c PPG.cpp -I/usr/include/opencv4 
clean:
	rm *.o
	rm *.exe