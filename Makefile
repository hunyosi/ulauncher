PROJNAME=ulauncher
VERSION=v0.0.2
RELEASEDIR=../$(PROJNAME)-$(VERSION)

#DEBUGFLAG=-DDEBUG
DEBUGFLAG=


CXX=g++
CXXFLAGS=-std=c++11 $(DEBUGFLAG)


ULOBJS=\
 ulauncher.o \
 chrcnv.o \
 strutil.o \
 randutil.o \
 fsutil.o \
 cmdlutil.o \
 envutil.o \
 inifile.o


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

ulauncher.exe: $(ULOBJS)
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) -o $@ $(ULOBJS) $(LDLIBS) -mwindows

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

