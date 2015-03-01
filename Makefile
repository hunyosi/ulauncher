PROJNAME=ulauncher
VERSION=v0.0.2
RELEASEDIR=../$(PROJNAME)-$(VERSION)

#DEBUGFLAG=-DDEBUG
DEBUGFLAG=


CXX=g++
CXXFLAGS=-std=c++11 $(DEBUGFLAG)


TLOBJS=\
 tlauncher.o \
 chrcnv.o \
 strutil.o \
 randutil.o \
 fsutil.o \
 cmdlutil.o \
 envutil.o \
 inifile.o \
 miscutil.o \
 VoicebankModule.o \
 BatExecInfo.o \
 BatToolSet.o \
 BatExecList.o \
 BatEnv.o


all: ulauncher.exe tlauncher.exe

ulauncher.exe: ulauncher.cpp
	g++ -std=c++11 -o ulauncher.exe ulauncher.cpp -mwindows

tlauncher.exe: $(TLOBJS)
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) -o $@ $(TLOBJS) $(LDLIBS)


clean:
	rm -f *.exe
	rm -f *.o


release: all
	mkdir $(RELEASEDIR)
	cp -f LICENSE $(RELEASEDIR)
	cp -f README.md $(RELEASEDIR)
	cp -f *.exe $(RELEASEDIR)
	cp -f *.ini $(RELEASEDIR)

