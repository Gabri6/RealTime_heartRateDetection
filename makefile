all: PPG
	g++ PPG.o -o PPG.exe
ppg: PPG.cpp
	g++ -c PPG.cpp
clean:
	rm *.o
	rm *.exe