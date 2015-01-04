#include <windows.h>
#include <string>
#include <stdexcept>
#include <memory>
#include <clocale>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <map>




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
  if (L'a' <= c && c <= L'z') {
   str[i] = L'A' + (c - L'a');
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

 std::string v(cmdLine);
 if (v.find_first_of(" \t\r\n\v\f") != std::string::npos) {
  std::string v2;
  v2.append(1, L'\"');

  char prevvc = 0;
  for (char vc : v) {
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

  return v2;
 } else {
  return v;
 }
}




static bool
existsFile(char const * path)
{
 DWORD attrs = ::GetFileAttributes(path);
 if (attrs == (DWORD)-1) {
  return false;
 }

 return ((attrs & FILE_ATTRIBUTE_DEVICE) == 0);
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




/*
class VoicebankModule
{

};
*/




typedef std::wstring WStr;
typedef std::shared_ptr< WStr > SpWStr;
typedef std::vector< SpWStr > VecSpWStr;
typedef std::shared_ptr< VecSpWStr > SpVecSpWStr;


typedef std::string Str;
typedef std::shared_ptr< Str > SpStr;
typedef std::vector< SpStr > VecSpStr;
typedef std::shared_ptr< VecSpStr > SpVecSpStr;




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
 if (str[0] != L'\0') {
  char c, nc;
  for (int i = 0; str[i + 1] != L'\0'; ++ i) {
   c = str[i];
   nc = str[i + 1];
   if (c == L'%' && L'0' <= nc && nc <= L'9') {
    argsExpanded.append(cmdLineEncoding((*args)[nc - L'0']->c_str()));
    ++ i;
   } else {
    argsExpanded.append(1, c);
   }
  }

  if (nc != L'\0') {
   argsExpanded.append(1, nc);
  }
 }

 int bufSize = ::ExpandEnvironmentStrings(argsExpanded.c_str(), NULL, 0);
 std::vector< char > buf(bufSize);
 ::ExpandEnvironmentStrings(argsExpanded.c_str(), &buf[0], bufSize);
 std::shared_ptr< std::string > cmdPath(new std::string(&buf[0]));
 return cmdPath;
}


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
 size_t bufSize = std::mbstowcs(NULL, s, 0);
 if (bufSize == (size_t)-1) {
  throw ConvStrToWStrError();
 }

 bufSize += 1;
 std::vector< wchar_t > buf(bufSize);
 std::mbstowcs(&buf[0], s, bufSize);

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
 size_t bufSize = std::wcstombs(NULL, ws, 0);
 if (bufSize == (size_t)-1) {
  throw ConvWStrToStrError();
 }

 bufSize += 1;
 std::vector< char > buf(bufSize);
 std::wcstombs(&buf[0], ws, bufSize);

 SpStr str(new Str(&buf[0]));
 return str;
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

 WIN32_FIND_DATA getInfo() const { return m_wfd; }

private:
 FileFinder(FileFinder const &);
 FileFinder & operator=(FileFinder const &);
};




typedef std::map<std::string, std::string > MapStr;
typedef std::shared_ptr< MapStr > SpMapStr;

class BatExecInfo
{
protected:
 SpVecSpStr m_batArgs;
 SpMapStr m_envVars;
 SpStr m_originalLine;
 SpStr m_expandedLine;
 SpVecSpStr m_parsed;

public:
 BatExecInfo(
   SpVecSpStr batArgs,
   SpMapStr envVars,
   SpStr originalLine,
   SpStr expandedLine);

 SpVecSpStr getBatArgs() const { return m_batArgs; }
 SpMapStr getEnvVars() const { return m_envVars; }
 SpStr getOriginalLine() const { return m_originalLine; }
 SpStr getExpandedLine() const { return m_expandedLine; }
 SpVecSpStr getParsed() const { return  m_parsed; }

 bool execute(int & exitCode);

public:
 BatExecInfo(BatExecInfo const &);
 BatExecInfo & operator=(BatExecInfo const &);
};


BatExecInfo::BatExecInfo(
  SpVecSpStr batArgs,
  SpMapStr envVars,
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


bool
BatExecInfo::execute(
  int & exitCode)
{
 DWORD status;

 for(auto elm : *m_envVars) {
  if (0 < elm.second.size()) {
   ::SetEnvironmentVariable(elm.first.c_str(), elm.second.c_str());
  } else {
   ::SetEnvironmentVariable(elm.first.c_str(), NULL);
  }
 }

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

 bool isEmpty() { return m_execInfos->size() == 0; }

 void addMessage(std::string const & msg);

 bool hasResampler() const { return (bool) m_resampler; }
 SpBatExecInfo getResampler() const { return m_resampler; }

 bool hasWavtool() const { return (bool) m_wavtool; }
 SpBatExecInfo getWavtool() const { m_wavtool; }

 bool setExecInfo(SpBatExecInfo execInfo);

 bool execute();
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




typedef std::shared_ptr< BatToolSet > SpBatToolSet;
typedef std::vector< SpBatToolSet > VecSpBatToolSet;
typedef std::shared_ptr< VecSpBatToolSet > SpVecSpBatToolSet;

class BatExecList
{
private:
 SpMapStr m_envVars;
 SpVecSpBatToolSet m_toolSets;

public:
 BatExecList();

 void setEnvVar(std::string const & name, std::string const & value);
 std::string const & getEnvVar(std::string const & name) const { return (*m_envVars)[name]; } 
 void addMessage(std::string const & msg);

 void addExecInfo(
  SpVecSpStr batArgs,
  SpStr originalLine,
  SpStr expandedLine);

 bool execute();
};


BatExecList::BatExecList()
  :
  m_envVars(new MapStr()),
  m_toolSets(new VecSpBatToolSet())
{
 SpBatToolSet toolSetSp(new BatToolSet());
 m_toolSets->push_back(toolSetSp);
}


void
BatExecList::setEnvVar(std::string const & varName, std::string const & varVal)
{
 SpMapStr envVars(new MapStr(*m_envVars));
 (*envVars)[varName.c_str()] = varVal.c_str();
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
 for (SpBatToolSet toolSet : *m_toolSets) {
  if (! toolSet->execute()) {
   return false;
  }
 }

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
 }  //else {
  std::cout << "> " << *expanded << std::endl;
 //}

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

  std::cout << "enter: " << *m_batPath << std::endl;

  SpVecSpStr lines(new VecSpStr());

  std::vector< char > buf(4096);
  std::ifstream ifs(m_batPath->c_str());
  while (ifs.good() && ! ifs.getline(&buf[0], buf.size()).eof()) {
   SpStr line(new Str(&buf[0]));
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

 std::cout << "leave: " << *m_batPath << std::endl;

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

  std::stringstream ss;
  ss << "exitCode=" << exitCode;
  ::MessageBox(NULL, ss.str().c_str(), "tlauncher", MB_OK);

 } catch (std::exception e) {
  std::cout << e.what() << std::endl;
 } catch (...) {
  throw;
 }

 return exitCode;
}


