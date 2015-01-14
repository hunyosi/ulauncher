RELEASEDIR=../ulauncher-`date +'%Y%m%d'`000

all: ulauncher.exe tlauncher.exe

ulauncher.exe: ulauncher.cpp
	g++ -std=c++11 -o ulauncher.exe ulauncher.cpp -mwindows

tlauncher.exe: tlauncher.cpp
	g++ -std=c++11 -o tlauncher.exe tlauncher.cpp


debug: ulauncher_d.exe tlauncher_d.exe
	cp -f ulauncher_d.exe ulauncher.exe
	cp -f tlauncher_d.exe tlauncher.exe

ulauncher_d.exe: ulauncher.cpp
	g++ -std=c++11 -DDEBUG -o ulauncher_d.exe ulauncher.cpp -mwindows

tlauncher_d.exe: tlauncher.cpp
	g++ -std=c++11 -DDEBUG -o tlauncher_d.exe tlauncher.cpp


clean:
	rm -f *.exe


release: all
	mkdir $(RELEASEDIR)
	cp -f LICENSE $(RELEASEDIR)
	cp -f README.md $(RELEASEDIR)
	cp -f *.exe $(RELEASEDIR)
	cp -f *.ini $(RELEASEDIR)

