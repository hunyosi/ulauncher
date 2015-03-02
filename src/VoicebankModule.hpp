#ifndef VOICEBANKMODULE_HPP__HUNYOSI
#define VOICEBANKMODULE_HPP__HUNYOSI


#include <string>


class VoicebankModule
{
public:
 typedef void _stdcall (*LoadFuncType)(char * baseDir);
 typedef void _stdcall (*UnloadFuncType)();
 typedef void _stdcall (*SetupFuncType)(int hwndParent);
 typedef int _stdcall (*ExistsFuncType)(char * noteName, int noteTone);
 typedef void * _stdcall (*GetPcmDataFuncType)(char * noteName, int noteTone);
 typedef void _stdcall (*FreePcmFuncType)(void * pPcmData);
 typedef void * _stdcall (*GetFrqDataFuncType)(char * noteName, int noteTone);
 typedef void _stdcall (*FreeFrqFuncType)(void *pFrqData);

private:
 class HandleHolder;

private:
 std::string m_modulePath;
 std::string m_dirPath;
 std::string m_fileName;
 HandleHolder *m_hModule;

 LoadFuncType m_load;
 UnloadFuncType m_unload;
 SetupFuncType m_setup;
 ExistsFuncType m_exists;
 GetPcmDataFuncType m_getpcmdata;
 FreePcmFuncType m_freepcm;
 GetFrqDataFuncType m_getfrqdata;
 FreeFrqFuncType m_freefrq;

public:
 VoicebankModule(std::string const & absPath);
 ~VoicebankModule();

 bool good() const;
 bool bad() const { return ! good(); }
 std::string const & path() const { return m_modulePath; }
 std::string const & dirPath() const { return m_dirPath; }
 std::string const & fileName() const { return m_fileName; }

 bool hasLoadFunc() const { return (m_load != NULL); }
 bool hasUnloadFunc() const {return (m_unload != NULL); }
 bool hasSetupFunc() const {return (m_setup != NULL); }
 bool hasExistsFunc() const {return (m_exists != NULL); }
 bool hasGetPcmDataFunc() const {return (m_getpcmdata != NULL); }
 bool hasFreePcmFunc() const {return (m_freepcm != NULL); }
 bool hasGetFrqDataFunc() const {return (m_getfrqdata != NULL); }
 bool hasFreeFrqFunc() const {return (m_freefrq != NULL); }

 void load(char const * baseDir = NULL) const;
 void unload() const;
 void setup(int hwndParent) const;
 int exists(char const * noteName, int noteTone) const;
 void * getPcmData(char const * noteName, int noteTone) const;
 void freePcm(void * pPcm) const;
 void * getFrqData(char const * noteName, int noteTone) const;
 void freeFrq(void *pFrq) const;

private:
 VoicebankModule(VoicebankModule const &);
 VoicebankModule & operator=(VoicebankModule const &);
};


#endif /* VOICEBANKMODULE_HPP__HUNYOSI */
