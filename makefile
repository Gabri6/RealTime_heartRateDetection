all: PPG
	g++ PPG.o -o PPG.exe
PPG: PPG.cpp
	g++ -c PPG.cpp
clean:
	rm *.o
	rm *.exe