
all: ulauncher.exe tlauncher.exe

ulauncher.exe: ulauncher.cpp
	g++ -std=c++11 -o ulauncher.exe ulauncher.cpp -mwindows

tlauncher.exe: tlauncher.cpp
	g++ -std=c++11 -o tlauncher.exe tlauncher.cpp

