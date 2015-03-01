#include <cstring>
#include <vector>
#include <windows.h>

#include "VoicebankModule.hpp"




class VoicebankModule::HandleHolder {
public:
  HMODULE handle;
};




VoicebankModule::VoicebankModule(
  std::string const & absPath)
  :
  m_modulePath(absPath),
  m_load(NULL),
  m_unload(NULL),
  m_setup(NULL),
  m_exists(NULL),
  m_getpcmdata(NULL),
  m_freepcm(NULL),
  m_getfrqdata(NULL),
  m_freefrq(NULL),
  m_hModule(NULL)
{
 std::string::size_type p = m_modulePath.find_last_of("\\//");
 m_dirPath.append(m_modulePath, 0, p + 1);
 m_fileName.append(m_modulePath, p + 1, m_modulePath.size() - (p + 1));

 HMODULE hMod = ::LoadLibrary(m_modulePath.c_str());
 if (hMod) {
  m_load = (LoadFuncType)::GetProcAddress(hMod, "load");
  m_unload = (UnloadFuncType)::GetProcAddress(hMod, "unload");
  m_setup = (SetupFuncType)::GetProcAddress(hMod, "setup");
  m_exists = (ExistsFuncType)::GetProcAddress(hMod, "exists");
  m_getpcmdata = (GetPcmDataFuncType)::GetProcAddress(hMod, "getpcmdata");
  m_freepcm = (FreePcmFuncType)::GetProcAddress(hMod, "freepcm");
  m_getfrqdata = (GetFrqDataFuncType)::GetProcAddress(hMod, "getfrqdata");
  m_freefrq = (FreeFrqFuncType)::GetProcAddress(hMod, "freefrq");
  m_hModule = new HandleHolder();
  m_hModule->handle = hMod;
 }
}


VoicebankModule::~VoicebankModule()
{
 if (m_hModule) {
  ::FreeLibrary(m_hModule->handle);
  delete m_hModule;
  m_hModule = NULL;
 }
}


bool
VoicebankModule::good() const
{
 if (! m_hModule) {
  return true;
 }

 return
   (m_load != NULL) ||
   (m_unload != NULL) ||
   (m_setup != NULL) ||
   (m_exists != NULL) ||
   (m_getpcmdata != NULL) ||
   (m_freepcm != NULL) ||
   (m_getfrqdata != NULL) ||
   (m_freefrq != NULL);
}


void
VoicebankModule::load(
  char const * baseDir) const
{
 if (! hasLoadFunc()) {
  return;
 }

 std::vector< char > baseDirBuf;
 if (baseDir) {
  baseDirBuf.resize(std::strlen(baseDir) + 1);
  std::strcpy(&baseDirBuf[0], baseDir);
 } else {
  baseDirBuf.resize(m_dirPath.size() + 1);
  std::strncpy(&baseDirBuf[0], m_dirPath.c_str(), m_dirPath.size());
  baseDirBuf[m_dirPath.size()] = '\0';
 }

 m_load(&baseDirBuf[0]);
}


void
VoicebankModule::unload() const
{
 if (! hasUnloadFunc()) {
  return;
 }

 m_unload();
}


void
VoicebankModule::setup(
  int hwndParent) const
{
 if (! hasSetupFunc()) {
  return;
 }

 m_setup(hwndParent);
}


int
VoicebankModule::exists(
  char const * noteName,
  int noteTone) const
{
 if (! hasExistsFunc()) {
  return 0;
 }

 char * bufp = NULL;
 std::vector< char > buf;
 if (noteName) {
  buf.resize(std::strlen(noteName) + 1);
  std::strcpy(&buf[0], noteName);
  bufp = &buf[0];
 }

 return m_exists(bufp, noteTone);
}


void *
VoicebankModule::getPcmData(
  char const * noteName,
  int noteTone) const
{
 if (! hasGetPcmDataFunc()) {
  return NULL;
 }

 char * bufp = NULL;
 std::vector< char > buf;
 if (noteName) {
  buf.resize(std::strlen(noteName) + 1);
  std::strcpy(&buf[0], noteName);
  bufp = &buf[0];
 }

 return m_getpcmdata(bufp, noteTone);
}


void
VoicebankModule::freePcm(
  void * pPcm) const
{
 if (! hasFreePcmFunc()) {
  return;
 }

 if (pPcm) {
  m_freepcm(pPcm);
 }
}


void *
VoicebankModule::getFrqData(
  char const * noteName,
  int noteTone) const
{
 if (! hasGetFrqDataFunc()) {
  return NULL;
 }

 char * bufp = NULL;
 std::vector< char > buf;
 if (noteName) {
  buf.resize(std::strlen(noteName) + 1);
  std::strcpy(&buf[0], noteName);
  bufp = &buf[0];
 }

 return m_getfrqdata(bufp, noteTone);
}


void
VoicebankModule::freeFrq(
  void *pFrq) const
{
 if (! hasFreePcmFunc()) {
  return;
 }

 if (pFrq) {
  m_freefrq(pFrq);
 }
}




