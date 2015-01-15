#include <windows.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdexcept>
#include <memory>
#include <clocale>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>




typedef std::wstring WStr;
typedef std::shared_ptr< WStr > SpWStr;
typedef std::vector< SpWStr > VecSpWStr;
typedef std::shared_ptr< VecSpWStr > SpVecSpWStr;


typedef std::string Str;
typedef std::shared_ptr< Str > SpStr;
typedef std::vector< SpStr > VecSpStr;
typedef std::shared_ptr< VecSpStr > SpVecSpStr;


class ConvStrToWStrError : public std::runtime_error
{
public:
 explicit ConvStrToWStrError() :
   std::runtime_error("ConvStrToWStrError") {}
};

static SpWStr
convStrToWStr(
  char const * s)
{
 int bufSize = ::MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
 if (bufSize == 0) {
  throw ConvStrToWStrError();
 }

 std::vector< wchar_t > buf(bufSize);
 if (! ::MultiByteToWideChar(CP_ACP, 0, s, -1, &buf[0], bufSize)) {
  throw ConvStrToWStrError();
 }

 SpWStr str(new WStr(&buf[0]));
 return str;
}


class ConvWStrToStrError : public std::runtime_error
{
public:
 explicit ConvWStrToStrError() :
   std::runtime_error("ConvWStrToStrError") {}
};

static SpStr
convWStrToStr(
  wchar_t const * ws)
{
 int bufSize = ::WideCharToMultiByte(CP_ACP, 0, ws, -1, NULL, 0, NULL, NULL);
 if (bufSize == 0) {
  throw ConvWStrToStrError();
 }

 std::vector< char > buf(bufSize);
 if (! ::WideCharToMultiByte(CP_ACP, 0, ws, -1, &buf[0], bufSize, NULL, NULL)) {
  throw ConvWStrToStrError();
 }

 SpStr str(new Str(&buf[0]));
 return str;
}




static void
rtrim(
  std::string& str,
  const std::string& trimChrs = " \t\n\r\v\f")
{
 std::string::size_type p = str.find_last_not_of(trimChrs);
 if (p == std::string::npos) {
  str.erase(0);
  return;
 }

 ++ p;
 str.erase(p);
}


static void
ltrim(
  std::string& str,
  const std::string& trimChrs = " \t\n\r\v\f")
{
 std::string::size_type p = str.find_first_not_of(trimChrs);
 str.erase(0, p);
}


static void
trim(
  std::string& str,
  const std::string& trimChrs = " \t\n\r\v\f")
{
 rtrim(str, trimChrs);
 ltrim(str, trimChrs);
}


static void
chomp(
  std::string& str)
{
 std::string::size_type p = str.find_last_not_of("\r\n");
 if (p == std::string::npos) {
  return;
 }

 ++ p;
 str.erase(p);
}

static void
toUpperCase(
  std::string& str)
{
 for (size_t i = 0, z = str.size(); i < z; ++ i) {
  char c = str[i];
  if ('a' <= c && c <= 'z') {
   str[i] = 'A' + (c - 'a');
  }
 }
}


static std::string
cmdLineEncoding(
 char const *cmdLine)
{
 if (cmdLine[0] == '\0') {
  return "\"\"";
 }

 SpWStr v = convStrToWStr(cmdLine);
 if (v->find_first_of(L" \t\r\n\v\f") != WStr::npos) {
  WStr v2;
  v2.append(1, L'\"');

  wchar_t prevvc = 0;
  for (wchar_t vc : *v) {
   if (vc == L'\"') {
    if (prevvc == L'\\') {
     v2.append(1, L'\\');
    }
    v2.append(1, L'\\');
   }

   v2.append(1, vc);
   prevvc = vc;
  }

  v2.append(1, L'\"');

  SpStr v3 = convWStrToStr(v2.c_str());
  return *v3;
 } else {
  return cmdLine;
 }
}


static std::string
getDirName(
  std::string const & path)
{
 SpWStr wPath = convStrToWStr(path.c_str());
 WStr::size_type p = wPath->find_last_of(L"\\/:");
 if (p == WStr::npos) {
  p = 0;
 } else {
  ++ p;
 }

 WStr wDirName(*wPath, 0, p);
 SpStr dirName = convWStrToStr(wDirName.c_str());
 return *dirName;
}


static std::string
getFileName(
  std::string const & path)
{
 SpWStr wPath = convStrToWStr(path.c_str());
 WStr::size_type p = wPath->find_last_of(L"\\/:");
 if (p == WStr::npos) {
  p = 0;
 } else {
  ++ p;
 }

 WStr wFileName(*wPath, p, wPath->size() - p);
 SpStr fileName = convWStrToStr(wFileName.c_str());
 return *fileName;
}


static std::string
getBaseName(
  std::string const & path)
{
 std::string fileName(getFileName(path));

 std::string::size_type p = fileName.find_last_of(".");
 if (p != std::string::npos) {
  fileName.erase(p);
 }

 return fileName;
}


static std::string
getExtName(
  std::string const & path)
{
 std::string fileName(getFileName(path));

 std::string::size_type p = fileName.find_last_of(".");
 if (p == std::string::npos) {
  return "";
 }

 fileName.erase(0, p);
 return fileName;
}


static std::string
eraseLastPathSep(
  std::string const & path)
{
 SpWStr wPath = convStrToWStr(path.c_str());
 wchar_t c = (*wPath)[wPath->size() - 1];
 if (c == L'\\' || c == L'/') {
  wPath->erase(wPath->size() - 1);
 }

 SpStr erased = convWStrToStr(wPath->c_str());
 return *erased;
}

static std::string
toFullPathName(
  std::string const & path)
{
 TCHAR *p;
 std::vector< TCHAR > buf(MAX_PATH);
 DWORD s;
 s = ::GetFullPathName(path.c_str(), buf.size(), &buf[0], &p);
 if (buf.size() - 1 < s) {
  buf.resize(s, 0);
  s = ::GetFullPathName(path.c_str(), buf.size(), &buf[0], &p);
 }

 if (s < 1) {
  buf[0] = 0;
 }

 std::string str(&buf[0]);
 return str;
}


static std::string
getCurDir()
{
 std::vector< TCHAR > buf(MAX_PATH);
 DWORD s;
 s = ::GetCurrentDirectory(buf.size(), &buf[0]);
 if (buf.size() - 1 < s) {
  buf.resize(s, 0);
  s = ::GetCurrentDirectory(buf.size(), &buf[0]);
 }

 if (s < 1) {
  buf[0] = 0;
 }

 std::string str(&buf[0]);
 return str;
}


static bool
setCurDir(
 std::string const & path)
{
 return ::SetCurrentDirectory(path.c_str()) != 0;
}


static bool
existsFile(char const * path)
{
 DWORD attrs = ::GetFileAttributes(path);
 if (attrs == (DWORD)-1) {
  return false;
 }

 return ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0);
}


static bool
existsDir(char const * path)
{
 DWORD attrs = ::GetFileAttributes(path);
 if (attrs == (DWORD)-1) {
  return false;
 }

 return ((attrs & FILE_ATTRIBUTE_DIRECTORY) != 0);
}


static int
existsFileOrDir(char const * path)
{
 DWORD attrs = ::GetFileAttributes(path);
 if (attrs == (DWORD)-1) {
  return false;
 }

 return ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0 ? 1 : -1);
}


static bool
existsPath(char const * path)
{
 DWORD attrs = ::GetFileAttributes(path);
 if (attrs == (DWORD)-1) {
  return false;
 }

 return true;
}


static std::string
getTempDir()
{
 std::vector< TCHAR > buf(MAX_PATH);
 DWORD s;
 s = ::GetTempPath(buf.size(), &buf[0]);
 if (buf.size() - 1 < s) {
  buf.resize(s, 0);
  s = ::GetTempPath(buf.size(), &buf[0]);
 }

 if (s < 1) {
  buf[0] = 0;
 }

 std::string str(&buf[0]);
 return str;
}


static int
randInt(
  int minVal,
  int maxVal)
{
 int trueMinVal = minVal;
 int trueMaxVal = maxVal;
 if (trueMinVal == trueMaxVal) {
  return trueMinVal;
 } if (trueMaxVal < trueMinVal) {
  trueMinVal = maxVal;
  trueMaxVal = minVal;
 }
 return (int)(((double)std::rand() * (trueMaxVal - trueMinVal)) / RAND_MAX + trueMinVal);
}


static char
randChar(
  std::string const chars)
{
 return chars[randInt(0, (int)chars.size() - 1)];
}


static std::string
randStr(
  int minLen = 4,
  int maxLen = 8,
  std::string const & chars = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
  std::string const & firstChars = "",
  std::string const & lastChars = "")
{
 int len = randInt(minLen, maxLen);
 if (len < 1) {
  return "";
 }

 std::string buf(len, 0);
 if (0 < firstChars.size()) {
  buf[0] = randChar(firstChars);
 } else {
  buf[0] = randChar(chars);
 }
 if (len == 1) {
  return buf;
 }

 int i = 1;
 int len2 = len - 1;
 for (; i < len2; ++ i) {
  buf[i] = randChar(chars);
 }

 if (0 < lastChars.size()) {
  buf[i] = randChar(lastChars);
 } else {
  buf[i] = randChar(chars);
 }

 return buf;
}


static std::string
makeTempDir()
{
 std::string baseDir(getTempDir());
 std::string tempDir;
 std::string randDirName;

 do {
  randDirName = randStr();
  tempDir = baseDir + randDirName;
 } while (existsPath(tempDir.c_str()));

 if (::CreateDirectory(tempDir.c_str(), NULL) == 0) {
  return "";
 }

 return tempDir + "\\";
}




static int
noteNameToNoteNumber(
  char const * noteName)
{
 static int const nameToNum[] = {9, 11, 0, 2, 4, 5, 7};
 int noteNoBase, numPos, octave;

 noteNoBase = nameToNum[noteName[0] - 'A'];
 if (noteName[1] == '#') {
  noteNoBase += 1;
  numPos = 2;
 } else {
  numPos = 1;
 }

 if (noteName[numPos] == '-') {
  octave = - (noteName[numPos + 1] - '0');
 } else {
  octave = noteName[numPos] - '0';
 }

 return (octave + 1) * 12 + noteNoBase;
}




/* ****************************************************
 *  EIN -- Extended INI 
 * ****************************************************/
namespace ein {

 class EinEntry;
 typedef std::shared_ptr< EinEntry > SpEinEntry;
 class EinSection;
 typedef std::shared_ptr< EinSection > SpEinSection;
 class EinNode;
 typedef std::shared_ptr< EinNode > SpEinNode;


 class EinEntry : public std::vector< std::string >
 {
 private:
  SpEinNode m_children;
 public:
  EinEntry();
  SpEinNode children() { return m_children; }
  void children(SpEinNode node) { m_children = node; }
 };


 class EinEntries : public std::vector< SpEinEntry >
 {
 public:
  SpEinEntry
  entry(std::string const & key)
  {
   for (auto entry : *this) {
    for (auto elm : *entry) {
     if (elm == key) {
      return entry;
     }
    }
   }

   SpEinEntry dmy;
   return dmy;
  }
 };


 class EinSection : public EinEntries
 {
 private:
  std::string m_name;

 public:
  std::string const & name() const { return m_name; }
  void name(std::string const & nameVal) { m_name = nameVal; }
 };


 class EinNode : public EinEntries
 {
 private:
  std::vector< SpEinSection > m_sections;

 public:
  std::vector< SpEinSection > & sections() { return m_sections; }

  SpEinSection section(std::string const & key)
  {
   for (auto section : m_sections) {
    if (section->name() == key) {
     return section;
    }
   }

   SpEinSection dmy;
   return dmy;
  }

 };


 inline
 EinEntry::EinEntry()
   :
   m_children(new EinNode())
 {
 }


 class EinParser
 {
 public:
  SpEinNode
  parse(
    std::istream & istrm)
  {
   SpEinNode node(new EinNode());
   return node;
  }
 };

}




typedef std::map<std::string, std::vector< std::string > > MapVec;
typedef std::shared_ptr< MapVec > SpMapVec;

static SpMapVec
readIniFile(
  char const * path)
{
 SpMapVec p;
 SpMapVec mp( new MapVec() );
 std::string prevKey = "";

 std::vector< char > buf(4096);
 std::ifstream ifs(path);
 while (ifs.good() && ! ifs.getline(&buf[0], buf.size()).eof()) {
  std::string line(&buf[0]);
  trim(line);
  if (line[0] == '#') {
   continue;
  }

  std::string::size_type p = line.find('=');
  if (p == std::string::npos) {
   std::vector< std::string > & vec = (*mp)[prevKey];
   vec.push_back(line);
   continue;

  } else {
   std::string key(line, 0, p);
   std::string val(line, p + 1, line.size() - (p + 1));
   trim(key);
   trim(val);
   std::vector< std::string > & vec = (*mp)[key];
   if (0 < val.size()) {
    vec.push_back(val);
   }

   prevKey = key;
  }
 }

 if (ifs.bad()) {
  return p;
 }

 return mp;
}




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
 std::string m_modulePath;
 std::string m_dirPath;
 std::string m_fileName;
 HMODULE m_hModule;

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
  m_freefrq(NULL)
{
 std::string::size_type p = m_modulePath.find_last_of("\\//");
 m_dirPath.append(m_modulePath, 0, p + 1);
 m_fileName.append(m_modulePath, p + 1, m_modulePath.size() - (p + 1));

 m_hModule = ::LoadLibrary(m_modulePath.c_str());
 if (m_hModule) {
  m_load = (LoadFuncType)::GetProcAddress(m_hModule, "load");
  m_unload = (UnloadFuncType)::GetProcAddress(m_hModule, "unload");
  m_setup = (SetupFuncType)::GetProcAddress(m_hModule, "setup");
  m_exists = (ExistsFuncType)::GetProcAddress(m_hModule, "exists");
  m_getpcmdata = (GetPcmDataFuncType)::GetProcAddress(m_hModule, "getpcmdata");
  m_freepcm = (FreePcmFuncType)::GetProcAddress(m_hModule, "freepcm");
  m_getfrqdata = (GetFrqDataFuncType)::GetProcAddress(m_hModule, "getfrqdata");
  m_freefrq = (FreeFrqFuncType)::GetProcAddress(m_hModule, "freefrq");
 }
}


VoicebankModule::~VoicebankModule()
{
 if (m_hModule) {
  ::FreeLibrary(m_hModule);
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




static SpStr
expandEnvVar(
  char const * str)
{
 int bufSize = ::ExpandEnvironmentStrings(str, NULL, 0);
 std::vector< char > buf(bufSize);
 ::ExpandEnvironmentStrings(str, &buf[0], bufSize);
 std::shared_ptr< std::string > cmdPath(new std::string(&buf[0]));
 return cmdPath;
}


static SpStr
expandEnvVar(
  char const * str,
  SpVecSpStr args)
{
 std::string argsExpanded;
 if (str[0] != '\0') {
  char c, nc;
  for (int i = 0; str[i + 1] != '\0'; ++ i) {
   c = str[i];
   nc = str[i + 1];
   if (c == '%' && '0' <= nc && nc <= '9') {
    argsExpanded.append(cmdLineEncoding((*args)[nc - '0']->c_str()));
    ++ i;
   } else {
    argsExpanded.append(1, c);
   }
  }

  if (nc != '\0') {
   argsExpanded.append(1, nc);
  }
 }

 int bufSize = ::ExpandEnvironmentStrings(argsExpanded.c_str(), NULL, 0);
 std::vector< char > buf(bufSize);
 ::ExpandEnvironmentStrings(argsExpanded.c_str(), &buf[0], bufSize);
 std::shared_ptr< std::string > cmdPath(new std::string(&buf[0]));
 return cmdPath;
}




class ParseCommandLineError : public std::runtime_error
{
public:
 explicit ParseCommandLineError() :
   std::runtime_error("ParseCommandLineError") {}
};

static SpVecSpStr
parseCommandLine(
  char const * cmdLine)
{
 SpWStr wCmdLine = convStrToWStr(cmdLine);
 int argc;
 wchar_t **argv = ::CommandLineToArgvW(wCmdLine->c_str(), &argc);
 if (! argv) {
  throw ParseCommandLineError();
 }
 std::shared_ptr< wchar_t * > srcArgV(argv, [](wchar_t **p){
  ::LocalFree(p);
 });

 SpVecSpStr argVec(new VecSpStr());
 argVec->reserve(argc);
 for (int i = 0; i < argc; ++ i) {
  argVec->push_back(convWStrToStr(argv[i]));
 }

 return argVec;
}


class WinHandleHolder
{
private:
 HANDLE m_handle;

public:
 WinHandleHolder(HANDLE handle) : m_handle(handle) {}

 ~WinHandleHolder()
 {
  close();
 }

 operator HANDLE() const { return m_handle; }

 void close()
 {
  if (m_handle) {
   ::CloseHandle(m_handle);
   m_handle = NULL;
  }
 }

private:
 WinHandleHolder(WinHandleHolder const &);
 WinHandleHolder & operator=(WinHandleHolder const &);
};


static int
concatBinFiles(
  SpVecSpStr srcFiles,
  const char *destPath)
{
 HANDLE fileHandle = ::CreateFile(destPath, GENERIC_WRITE,
   0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 if (fileHandle == INVALID_HANDLE_VALUE) {
  return -1;
 }

 WinHandleHolder fh(fileHandle);
 std::vector< unsigned char > buf(4096);
 DWORD readSize;
 DWORD wroteSize;
 size_t pos;

 for (SpStr srcFile : *srcFiles) {
  HANDLE fileHandle2 = ::CreateFile(srcFile->c_str(), GENERIC_READ,
    0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (fileHandle2 == INVALID_HANDLE_VALUE) {
   return -1;
  }

  WinHandleHolder fh2(fileHandle2);

  for (;;) {
   if (! ::ReadFile(fh2, &buf[0], buf.size(), &readSize, NULL)) {
    return -1;
   }

   if (readSize < 1) {
    break;
   }

   pos = 0;
   do {
    if (! ::WriteFile(fh, &buf[pos], readSize, &wroteSize, NULL)) {
     return -1;
    }

    pos += wroteSize;
    readSize -= wroteSize;
   } while (0 < readSize);
  }

  fh2.close();
 }

 fh.close();
 return 0;
}


static int
writeBinFile(
  const char * destPath,
  void * data,
  size_t dataSize)
{
 unsigned char * buf =(unsigned char *) data;
 HANDLE fileHandle = ::CreateFile(destPath, GENERIC_WRITE,
   0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 if (fileHandle == INVALID_HANDLE_VALUE) {
  return -1;
 }

 WinHandleHolder fh(fileHandle);
 DWORD writeSize = (DWORD) dataSize;
 DWORD wroteSize;
 size_t pos;

 pos = 0;
 do {
  if (! ::WriteFile(fh, &buf[pos], writeSize, &wroteSize, NULL)) {
   return -1;
  }

  pos += wroteSize;
  writeSize -= wroteSize;
 } while (0 < writeSize);

 fh.close();
 return 0;
}


static size_t
getWavFileSize(
  void * wavData)
{
 unsigned char * buf = (unsigned char *) wavData;
 return 8 + ((buf[7] << 24) | (buf[6] << 16) | (buf[5] << 8) | buf[4]);
}


static size_t
getFrqFileSize(
  void * frqData)
{
 unsigned char * buf = (unsigned char *) frqData;
 return 40 + 8 * 2 * ((buf[39] << 24) | (buf[38] << 16) | (buf[37] << 8) | buf[36]);
}


class FileFinder
{
private:
 std::string m_pattern;
 HANDLE m_handle;
 WIN32_FIND_DATA m_wfd;

public:
 FileFinder(std::string const & pattern) :
   m_pattern(pattern), m_handle(INVALID_HANDLE_VALUE) {}

 ~FileFinder()
 {
  close();
 }

 void close()
 {
  if (m_handle != INVALID_HANDLE_VALUE) {
   ::FindClose(m_handle);
   m_handle = INVALID_HANDLE_VALUE;
  }
 }

 bool next()
 {
  if (m_handle == INVALID_HANDLE_VALUE) {
   m_handle = ::FindFirstFile(m_pattern.c_str(), &m_wfd);
   return m_handle != INVALID_HANDLE_VALUE;
  } else {
   return ::FindNextFile(m_handle, &m_wfd) != FALSE;
  }
 }

 WIN32_FIND_DATA const & getInfo() const { return m_wfd; }

private:
 FileFinder(FileFinder const &);
 FileFinder & operator=(FileFinder const &);
};



static bool
deleteRecursive(
  std::string const & path)
{
 std::string delPtn(eraseLastPathSep(path));
 std::string dirPath(getDirName(delPtn));

 VecSpStr files;
 VecSpStr dirs;
 FileFinder ff(delPtn);
 while (ff.next()) {
  SpStr name(new std::string(ff.getInfo().cFileName));
  if (*name != "." && *name != "..") {
   if (ff.getInfo().dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    dirs.push_back(name);
   } else {
    files.push_back(name);
   }
  }
 }

 ff.close();

 bool success = true;

 for (SpStr name : files) {
  std::string targetPath(dirPath);
  targetPath.append(*name);
  if (! ::DeleteFile(targetPath.c_str())) {
   success = false;
  }
 }

 for (SpStr name : dirs) {
  std::string targetPath(dirPath);
  targetPath.append(*name);
  std::string dirPtn(targetPath);
  dirPtn.append("\\*");
  if (deleteRecursive(dirPtn.c_str())) {
   if (! ::RemoveDirectory(targetPath.c_str())) {
    success = false;
   }
  } else {
   success = false;
  }
 }

 return success;
}




typedef std::map< Str, SpStr > MapSpStr;
typedef std::shared_ptr< MapSpStr > SpMapSpStr;

class BatExecInfo
{
protected:
 SpVecSpStr m_batArgs;
 SpMapSpStr m_envVars;
 SpStr m_originalLine;
 SpStr m_expandedLine;
 SpVecSpStr m_parsed;

public:
 BatExecInfo(
   SpVecSpStr batArgs,
   SpMapSpStr envVars,
   SpStr originalLine,
   SpStr expandedLine);

 SpVecSpStr getBatArgs() { return m_batArgs; }
 SpMapSpStr getEnvVars() { return m_envVars; }
 SpStr getOriginalLine() { return m_originalLine; }
 SpStr getExpandedLine() { return m_expandedLine; }
 SpVecSpStr getParsed() { return  m_parsed; }

 void exportEnvVars();

 bool execute(int & exitCode);

public:
 BatExecInfo(BatExecInfo const &);
 BatExecInfo & operator=(BatExecInfo const &);
};


BatExecInfo::BatExecInfo(
  SpVecSpStr batArgs,
  SpMapSpStr envVars,
  SpStr originalLine,
  SpStr expandedLine)
  :
  m_batArgs(batArgs),
  m_envVars(envVars),
  m_originalLine(originalLine),
  m_expandedLine(expandedLine)
{
 m_parsed = parseCommandLine(m_expandedLine->c_str());
}


void
BatExecInfo::exportEnvVars()
{
 for(auto elm : *m_envVars) {
  if (elm.second && 0 < elm.second->size()) {
   ::SetEnvironmentVariable(elm.first.c_str(), elm.second->c_str());
  } else {
   ::SetEnvironmentVariable(elm.first.c_str(), NULL);
  }
 }
}


bool
BatExecInfo::execute(
  int & exitCode)
{
 DWORD status;

 exportEnvVars();

 std::string line;
 for (SpStr elm : *m_parsed) {
  if (0 < line.size()) {
   line.append(1, ' ');
  }
  line.append(cmdLineEncoding(elm->c_str()));
 }

 std::vector< char > cmdLine(line.size() + 1);
 size_t i;
 for (i = 0; i < line.size(); ++ i) {
  cmdLine[i] = line[i];
 }
 cmdLine[i] = '\0';

 std::cout << "> " << &cmdLine[0] << std::endl;


 STARTUPINFO si;
 GetStartupInfo(&si);

 PROCESS_INFORMATION pi;

 if (! ::CreateProcess(
   NULL, &cmdLine[0],
   NULL, NULL, FALSE,
   NORMAL_PRIORITY_CLASS,
   NULL, NULL,
   &si, &pi)) {
  exitCode = -1;
  return false;
 }

 ::WaitForSingleObject(pi.hProcess, INFINITE);
 ::GetExitCodeProcess(pi.hProcess, &status);
 ::CloseHandle(pi.hThread);
 ::CloseHandle(pi.hProcess);

 exitCode = (int)status;
 return true;
}




typedef std::shared_ptr< BatExecInfo > SpBatExecInfo;
typedef std::vector< SpBatExecInfo > VecSpBatExecInfo;
typedef std::shared_ptr< VecSpBatExecInfo > SpVecSpBatExecInfo;


class BatToolSet
{
private:
 std::string m_msg;
 SpVecSpBatExecInfo m_execInfos;
 SpBatExecInfo m_resampler;
 SpBatExecInfo m_wavtool;

public:
 BatToolSet();

 SpVecSpBatExecInfo getExecInfos() { return m_execInfos; }
 bool isEmpty() { return m_execInfos->size() == 0; }

 void addMessage(std::string const & msg);

 bool hasResampler() const { return (bool) m_resampler; }
 SpBatExecInfo getResampler() { return m_resampler; }

 bool hasWavtool() const { return (bool) m_wavtool; }
 SpBatExecInfo getWavtool() { return m_wavtool; }

 bool setExecInfo(SpBatExecInfo execInfo);

 bool execute();

 std::string const & getInputFileName();
 void setInputFileName(std::string const &);

 std::string const & getNoteName();

 void clearLoadModule();
};


BatToolSet::BatToolSet()
  :
  m_execInfos(new VecSpBatExecInfo())
{
}


void
BatToolSet::addMessage(
  std::string const & msg)
{
 m_msg.append(msg);
}


bool
BatToolSet::setExecInfo(
  SpBatExecInfo execInfo)
{
 m_execInfos->push_back(execInfo);

 std::string cmd(*(execInfo->getOriginalLine()));
 ltrim(cmd, "@ \t\v\f\n\r");
 std::string::size_type pos = cmd.find_first_of(" \t\v\f\n\r");
 if (pos != std::string::npos) {
  cmd.erase(pos);
 }

 toUpperCase(cmd);
 if (cmd.find("%RESAMP%") != std::string::npos) {
  m_resampler = execInfo;
 } else if (cmd.find("%TOOL%") != std::string::npos) {
  m_wavtool = execInfo;
  return false;
 }

 return true;
}


bool
BatToolSet::execute()
{
 std::cout << m_msg << std::endl;

 int exitCode;

 for (SpBatExecInfo execInfo : *m_execInfos) {
  if (! execInfo->execute(exitCode)) {
   return false;
  }
 }

 return true;
}


std::string const &
BatToolSet::getInputFileName()
{
 if (hasResampler()) {
  return *((*(getResampler()->getParsed()))[1]);

 } else if (hasWavtool()) {
  std::string helper = getFileName(*(*(getWavtool()->getEnvVars()))["helper"]);
  std::string batName = getFileName(*((*(getWavtool()->getBatArgs()))[0]));
  if (batName != helper) {
   return *((*(getWavtool()->getParsed()))[2]);
  }
 }

 static std::string dmy("");
 return dmy;
}


void
BatToolSet::setInputFileName(std::string const & fileName)
{
 SpStr spFileName(new std::string(fileName));

 if (hasResampler()) {
  (*(getResampler()->getParsed()))[1] = spFileName;

 } else if (hasWavtool()) {
  std::string helper = getFileName(*(*(getWavtool()->getEnvVars()))["helper"]);
  std::string batName = getFileName(*((*(getWavtool()->getBatArgs()))[0]));
  if (batName != helper) {
   (*(getWavtool()->getParsed()))[2] = spFileName;
  }
 }
}


std::string const &
BatToolSet::getNoteName()
{
 if (hasResampler()) {
  return *((*(getResampler()->getParsed()))[3]);
 }

 static std::string dmy("");
 return dmy;
}


void
BatToolSet::clearLoadModule()
{
 SpStr emptyStr(new Str(""));

 if (hasWavtool()) {
  (*(getWavtool()->getEnvVars()))["loadmodule"] = emptyStr;
 }

 if (hasResampler()) {
  (*(getResampler()->getEnvVars()))["loadmodule"] = emptyStr;
 }
}




typedef std::shared_ptr< BatToolSet > SpBatToolSet;
typedef std::vector< SpBatToolSet > VecSpBatToolSet;
typedef std::shared_ptr< VecSpBatToolSet > SpVecSpBatToolSet;

class BatExecList
{
private:
 SpMapSpStr m_envVars;
 SpVecSpBatToolSet m_toolSets;

public:
 BatExecList();

 void setEnvVar(std::string const & name, std::string const & value);
 std::string const & getEnvVar(std::string const & name) const { return *(*m_envVars)[name]; } 
 void addMessage(std::string const & msg);

 void addExecInfo(
   SpVecSpStr batArgs,
   SpStr originalLine,
   SpStr expandedLine);

 bool execute();

 SpVecSpBatToolSet getToolSets() { return m_toolSets; }

};


BatExecList::BatExecList()
  :
  m_envVars(new MapSpStr()),
  m_toolSets(new VecSpBatToolSet())
{
 SpBatToolSet toolSetSp(new BatToolSet());
 m_toolSets->push_back(toolSetSp);
}


void
BatExecList::setEnvVar(std::string const & varName, std::string const & varVal)
{
 SpMapSpStr envVars(new MapSpStr(*m_envVars));
 SpStr spVarVal(new std::string(varVal));
 (*envVars)[varName.c_str()] = spVarVal;
 m_envVars = envVars;
}


void
BatExecList::addMessage(
  std::string const & msg)
{
 m_toolSets->back()->addMessage(msg);
}


void
BatExecList::addExecInfo(
  SpVecSpStr batArgs,
  SpStr originalLine,
  SpStr expandedLine)
{
 SpBatExecInfo execInfo(new BatExecInfo(
   batArgs,
   m_envVars,
   originalLine,
   expandedLine));

 if (! m_toolSets->back()->setExecInfo(execInfo)) {
  SpBatToolSet toolSetSp(new BatToolSet());
  m_toolSets->push_back(toolSetSp);
 }
}


bool
BatExecList::execute()
{
 std::string tempDir = makeTempDir();
 std::cout << "tempDir=" << tempDir << std::endl;
 if (tempDir.size() < 1) {
  return false;
 }

 std::shared_ptr< VoicebankModule > vbmod;
 if (m_envVars->find("loadmodule") != m_envVars->end() && (*m_envVars)["loadmodule"]->size() != 0) {
  std::shared_ptr< VoicebankModule > newVbMod(new VoicebankModule(*(*m_envVars)["loadmodule"]));
  vbmod = newVbMod;
 }

 for (SpBatToolSet toolSet : *m_toolSets) {

  if (toolSet->isEmpty()) {
   break;
  }

  if (vbmod) {
   std::string inWavPath = toolSet->getInputFileName();
   if (0 < inWavPath.size()) {
    std::string tempVbDir = tempDir + getFileName(eraseLastPathSep(getDirName(inWavPath)));
    if (! existsDir(tempVbDir.c_str())) {
     if (! ::CreateDirectory(tempVbDir.c_str(), NULL)) {
      return false;
     }
    }
    tempVbDir.append("\\");

    std::string voiceName = getBaseName(inWavPath);
    std::string noteName = toolSet->getNoteName();
    int noteNumber = 0;
    if (0 < noteName.size()) {
     noteNumber = noteNameToNoteNumber(noteName.c_str());
    }

    (*toolSet->getExecInfos())[0]->exportEnvVars();

    std::string dirBackup(getCurDir());
    setCurDir(vbmod->dirPath());
    vbmod->load();
    void * pcm = vbmod->getPcmData(voiceName.c_str(), noteNumber);
    if (pcm) {
     std::string newWavPath = tempVbDir + voiceName + ".wav";
     size_t wavSize = getWavFileSize(pcm);
     writeBinFile(newWavPath.c_str(), pcm, wavSize);
     vbmod->freePcm(pcm);

     void * frq = vbmod->getFrqData(voiceName.c_str(), noteNumber);
     if (frq) {
      std::string newFrqPath = tempVbDir + voiceName + "_wav.frq";
      size_t frqSize = getFrqFileSize(frq);
      writeBinFile(newFrqPath.c_str(), frq, frqSize);
      vbmod->freeFrq(frq);
     }

     toolSet->setInputFileName(newWavPath);
    }
    vbmod->unload();
    setCurDir(dirBackup);

    toolSet->clearLoadModule();
   }
  }

  if (! toolSet->execute()) {
   return false;
  }
 }

 deleteRecursive(tempDir);
 return true;
}




class BatEnv
{
private:
 SpStr m_batPath;
 SpVecSpStr m_args;
 std::map< std::string, size_t > m_labels;
 size_t m_pc;
 int m_exitCode;

public:
 int callBat(SpVecSpStr args, BatExecList & execList);
 int exitCode() const { return m_exitCode; }

private:
 int evalLine(Str const & line, BatExecList & execList);
 int evalLineBody(
   Str lineBody,
   Str const & origLine,
   BatExecList & execList);

 int doRem(Str & line, Str const & origLine, BatExecList & execList);
 int doSet(Str & line, Str const & origLine, BatExecList & execList);
 int doIf(Str & line, Str const & origLine, BatExecList & execList);
 int doGoto(Str & line, Str const & origLine, BatExecList & execList);
 int doCall(Str & line, Str const & origLine, BatExecList & execList);
 int doEcho(Str & line, Str const & origLine, BatExecList & execList);
 int doDel(Str & line, Str const & origLine, BatExecList & execList);
 int doMkdir(Str & line, Str const & origLine, BatExecList & execList);
 int doCopy(Str & line, Str const & origLine, BatExecList & execList);
 int doExec(Str & line, Str const & origLine, BatExecList & execList);
};


int
BatEnv::doRem(Str & line, Str const & origLine, BatExecList & execList)
{
 return 0;
}


int
BatEnv::doSet(Str & line, Str const & origLine, BatExecList & execList)
{
 line.erase(0, 3);
 ltrim(line);

 std::string::size_type p = line.find("=");
 std::string varName(line, 0, p != std::string::npos ? p : line.size());
 rtrim(varName);
 std::string varVal;
 if (p != std::string::npos) {
  varVal.append(line, p + 1, line.size() - (p + 1));
 }

 execList.setEnvVar(varName, varVal);

 if (0 < varVal.size()) {
  ::SetEnvironmentVariable(varName.c_str(), varVal.c_str());
 } else {
  ::SetEnvironmentVariable(varName.c_str(), NULL);
 }

 return 0;
}


int
BatEnv::doIf(Str & line, Str const & origLine, BatExecList & execList)
{
 bool isNot = false;
 bool done = false;
 bool result = false;

 line.erase(0, 2);
 ltrim(line);
 SpVecSpStr args = parseCommandLine(line.c_str());

 toUpperCase(*(*args)[0]);
 if (*(*args)[0] == "NOT") {
  isNot = true;
  line.erase(0, 3);
  ltrim(line);
  args->erase(args->begin());
 }

 toUpperCase(*(*args)[0]);
 if (*(*args)[0] == "EXIST") {
  line.erase(0, 5);
  ltrim(line);
  args->erase(args->begin());

  FileFinder ff(*(*args)[0]);
  result = ff.next();
  ff.close();

  size_t quoteLen = 0;
  if (line[0] == L'\"') {
   quoteLen = 2;
  }

  line.erase(0, (*args)[0]->size() + quoteLen);
  ltrim(line);
  args->erase(args->begin());

  done = true;
 }

 if (done) {
  if (isNot) {
   result = ! result;
  }

  if (result) {
   evalLineBody(line, origLine, execList);
  }
 }

 return 0;
}


int
BatEnv::doGoto(Str & line, Str const & origLine, BatExecList & execList)
{
 line.erase(0, 4);
 trim(line);

 std::string::size_type p = line.find_first_of(" \t\n\r\v\f");
 std::string label(line, 0, p != std::string::npos ? p : line.size());
 toUpperCase(label);
 m_pc = m_labels[label];
 return 0;
}


int
BatEnv::doEcho(Str & line, Str const & origLine, BatExecList & execList)
{
 line.erase(0, 5);
 execList.addMessage(line);
// std::cout << line << std::endl;
 return 0;
}


int
BatEnv::doCall(Str & line, Str const & origLine, BatExecList & execList)
{
 line.erase(0, 4);
 ltrim(line);
 SpVecSpStr args = parseCommandLine(line.c_str());
 BatEnv env;
 env.callBat(args, execList);
 m_exitCode = env.exitCode();
 return 0;
}


int
BatEnv::doDel(Str & line, Str const & origLine, BatExecList & execList)
{
 SpVecSpStr args = parseCommandLine(line.c_str());

 SpVecSpStr lst(new VecSpStr);

 FileFinder ff(*(*args)[1]);
 while (ff.next()) {
  SpStr fname(new std::string(ff.getInfo().cFileName));
  lst->push_back(fname);
 }

 ff.close();

 for (SpStr fname : *lst) {
  ::DeleteFile(fname->c_str());
 }

 return 0;
}


int
BatEnv::doMkdir(Str & line, Str const & origLine, BatExecList & execList)
{
 SpVecSpStr args = parseCommandLine(line.c_str());
 ::CreateDirectory((*args)[1]->c_str(), NULL);
 return 0;
}


int
BatEnv::doCopy(Str & line, Str const & origLine, BatExecList & execList)
{
/*
 SpVecSpStr args = parseCommandLine(line.c_str());
 SpVecSpStr srcs(new VecSpStr());
 srcs->push_back((*args)[2]);
 srcs->push_back((*args)[5]);

 return concatBinFiles(srcs, (*args)[7]->c_str());
*/
 return 0;
}


int
BatEnv::doExec(
  Str & line,
  Str const & origLine, 
  BatExecList & execList)
{
 SpStr cmdLine(new Str(line));
 SpStr origCmdLine(new Str(origLine));
 execList.addExecInfo(m_args, origCmdLine, cmdLine);
 return 0;
}


int
BatEnv::evalLineBody(
  Str lineBody,
  Str const & origLine,
  BatExecList & execList)
{
 std::string::size_type p = lineBody.find_first_of(" \t\v\f");
 std::string command(lineBody, 0, p != std::string::npos ? p : lineBody.size());
 toUpperCase(command);

 if (command == "REM") {
  return doRem(lineBody, origLine, execList);
 } else if (command == "SET") {
  return doSet(lineBody, origLine, execList);
 } else if (command == "IF") {
  return doIf(lineBody, origLine, execList);
 } else if (command == "GOTO") {
  return doGoto(lineBody, origLine, execList);
 } else if (command == "ECHO") {
  return doEcho(lineBody, origLine, execList);
 } else if (command == "DEL") {
  return doDel(lineBody, origLine, execList);
 } else if (command == "MKDIR") {
  return doMkdir(lineBody, origLine, execList);
 } else if (command == "COPY") {
  return doCopy(lineBody, origLine, execList);
 } else if (command == "CALL") {
  return doCall(lineBody, origLine, execList);
 } else {
  return doExec(lineBody, origLine, execList);
 }
}


int
BatEnv::evalLine(
  Str const & line,
  BatExecList & execList)
{
 SpStr expanded = expandEnvVar(line.c_str(), m_args);
 ltrim(*expanded);
 if (expanded->size() == 0) {
  return 0;
 }

 char tc = (*expanded)[0];
 if (tc == ':') {
  return 0;
 }

 if (tc == '@') {
  expanded->erase(0, 1);
  ltrim(*expanded);
 }

#ifdef DEBUG
 std::cout << "> " << *expanded << std::endl;
#endif

 return evalLineBody(*expanded, line, execList);
}


int
BatEnv::callBat(
  SpVecSpStr args,
  BatExecList & execList)
{
 int retVal = 0;
 try {
  m_exitCode = 0;
  m_pc = 0;
  m_args = args;
  m_batPath = (*args)[0];

#ifdef DEBUG
  std::cout << "enter: " << *m_batPath << std::endl;
#endif

  SpVecSpStr lines(new VecSpStr());

  std::string buf;
  std::ifstream ifs(m_batPath->c_str());
  while (ifs.good() && std::getline(ifs, buf)) {
   SpStr line(new std::string(buf));
   chomp(*line);
   lines->push_back(line);

   Str tmpLine(&buf[0]);
   rtrim(tmpLine);
   ltrim(tmpLine, "@ \t\n\r\v\f");
   if (tmpLine[0] == L':') {
    std::string::size_type p =  tmpLine.find_first_not_of(": \t\n\r\v\f");
    std::string label(
      tmpLine, 1, p != std::string::npos ? p : tmpLine.size() - 1);
    toUpperCase(label);
    m_labels[label] = lines->size() - 1;
   }
  }

  if (ifs.bad()) {
   m_exitCode = -1;
   return -1;
  }

  for (; m_pc < lines->size(); ++ m_pc) {
   evalLine(*(*lines)[m_pc], execList);
  }

 } catch (std::exception e) {
  std::cout << e.what() << std::endl;
  m_exitCode = -1;
  retVal = -1;
 } catch (...) {
  m_exitCode = -1;
  retVal = -1;
 }

#ifdef DEBUG
 std::cout << "leave: " << *m_batPath << std::endl;
#endif

 return retVal;
}


void
removeWavPartialFiles(BatExecList & execList)
{
 Str wav =  execList.getEnvVar("output");
 SpStr whd(new Str(wav + ".whd"));
 SpStr dat(new Str(wav + ".dat"));

 if (existsFile(whd->c_str())) {
  ::DeleteFile(whd->c_str());
 }

 if (existsFile(dat->c_str())) {
  ::DeleteFile(dat->c_str());
 }
}


bool
buildWavFile(BatExecList & execList)
{
 bool success = false;
 Str wav =  execList.getEnvVar("output");
 SpStr whd(new Str(wav + ".whd"));
 SpStr dat(new Str(wav + ".dat"));

 if (existsFile(whd->c_str()) && existsFile(dat->c_str())) {
  SpVecSpStr srcs(new VecSpStr());
  srcs->push_back(whd);
  srcs->push_back(dat);

  success = (concatBinFiles(srcs, wav.c_str()) == 0);
 }

 removeWavPartialFiles(execList);

 return success;
}



int
mainImpl(SpVecSpStr args)
{
 SpVecSpStr newArgs(new VecSpStr());
 for (size_t i = 2; i < args->size(); ++ i) {
  newArgs->push_back((*args)[i]);
 }

 BatExecList execList;
 BatEnv env;
 env.callBat(newArgs, execList);

 removeWavPartialFiles(execList);

 if (execList.execute()) {
  buildWavFile(execList);
  return 0;
 } else {
  return -1;
 }
}


int
main()
{
 int exitCode = -1;
 try {
//  std::setlocale(LC_ALL, "");

  char *cmdLine = ::GetCommandLine();
  std::cout << cmdLine << std::endl;

  SpVecSpStr args(parseCommandLine(cmdLine));

  exitCode = mainImpl(args);

#ifdef DEBUG
  std::stringstream ss;
  ss << "exitCode=" << exitCode;
  ::MessageBox(NULL, ss.str().c_str(), "tlauncher", MB_OK);
#endif

 } catch (std::exception e) {
  std::cout << e.what() << std::endl;
 } catch (...) {
  throw;
 }

 return exitCode;
}


