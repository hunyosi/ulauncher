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
    std::string v(*(*args)[nc - L'0']);
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

     argsExpanded.append(v2);
    } else {
     argsExpanded.append(v);
    }

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




class BatEnv
{
private:
 SpStr m_batPath;
 SpVecSpStr m_args;
 std::map< std::string, size_t > m_labels;
 size_t m_pc;
 int m_exitCode;

public:
 int callBat(SpVecSpStr args);
 int exitCode() const { return m_exitCode; }

private:
 int evalLine(Str const & line);
 int evalLineBody(Str lineBody);

 int doRem(Str & line);
 int doSet(Str & line);
 int doIf(Str & line);
 int doGoto(Str & line);
 int doEcho(Str & line);
 int doDel(Str & line);
 int doMkdir(Str & line);
 int doCopy(Str & line);
 int doCall(Str & line);
 int doExec(Str & line);
};


int
BatEnv::doRem(Str & line)
{
 return 0;
}


int
BatEnv::doSet(Str & line)
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

 if (0 < varVal.size()) {
  ::SetEnvironmentVariable(varName.c_str(), varVal.c_str());
 } else {
  ::SetEnvironmentVariable(varName.c_str(), NULL);
 }

 return 0;
}


int
BatEnv::doIf(Str & line)
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
   evalLineBody(line);
  }
 }

 return 0;
}


int
BatEnv::doGoto(Str & line)
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
BatEnv::doEcho(Str & line)
{
 line.erase(0, 5);
 std::cout << line << std::endl;
 return 0;
}


int
BatEnv::doDel(Str & line)
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
BatEnv::doMkdir(Str & line)
{
 SpVecSpStr args = parseCommandLine(line.c_str());
 ::CreateDirectory((*args)[1]->c_str(), NULL);
 return 0;
}


int
BatEnv::doCopy(Str & line)
{
 SpVecSpStr args = parseCommandLine(line.c_str());
 SpVecSpStr srcs(new VecSpStr());
 srcs->push_back((*args)[2]);
 srcs->push_back((*args)[5]);

 return concatBinFiles(srcs, (*args)[7]->c_str());
}


int
BatEnv::doCall(Str & line)
{
 line.erase(0, 4);
 ltrim(line);
 SpVecSpStr args = parseCommandLine(line.c_str());
 BatEnv env;
 env.callBat(args);
 m_exitCode = env.exitCode();
 return 0;
}


int
BatEnv::doExec(
  Str & line)
{
 DWORD exitCode;

 ltrim(line);

 std::vector< char > cmdLine(line.size() + 1);
 size_t i;
 for (i = 0; i < line.size(); ++ i) {
  cmdLine[i] = line[i];
 }
 cmdLine[i] = '\0';

 STARTUPINFO si;
 GetStartupInfo(&si);

 PROCESS_INFORMATION pi;

 if (! ::CreateProcess(
   NULL, &cmdLine[0],
   NULL, NULL, FALSE,
   NORMAL_PRIORITY_CLASS,
   NULL, NULL,
   &si, &pi)) {
  m_exitCode = -1;
  return -1;
 }

 ::WaitForSingleObject(pi.hProcess, INFINITE);
 ::GetExitCodeProcess(pi.hProcess, &exitCode);
 ::CloseHandle(pi.hThread);
 ::CloseHandle(pi.hProcess);

 m_exitCode = (int)exitCode;
 return 0;
}


int
BatEnv::evalLineBody(
  Str lineBody)
{
 std::string::size_type p = lineBody.find_first_of(" \t\v\f");
 std::string command(lineBody, 0, p != std::string::npos ? p : lineBody.size());
 toUpperCase(command);

 if (command == "REM") {
  return doRem(lineBody);
 } else if (command == "SET") {
  return doSet(lineBody);
 } else if (command == "IF") {
  return doIf(lineBody);
 } else if (command == "GOTO") {
  return doGoto(lineBody);
 } else if (command == "ECHO") {
  return doEcho(lineBody);
 } else if (command == "DEL") {
  return doDel(lineBody);
 } else if (command == "MKDIR") {
  return doMkdir(lineBody);
 } else if (command == "COPY") {
  return doCopy(lineBody);
 } else if (command == "CALL") {
  return doCall(lineBody);
 } else {
  return doExec(lineBody);
 }
}


int
BatEnv::evalLine(
  Str const & line)
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

 return evalLineBody(*expanded);
}


int
BatEnv::callBat(
  SpVecSpStr args)
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
   evalLine(*(*lines)[m_pc]);
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


int
mainImpl(SpVecSpStr args)
{
 SpVecSpStr newArgs(new VecSpStr());
 for (size_t i = 2; i < args->size(); ++ i) {
  newArgs->push_back((*args)[i]);
 }

 BatEnv env;
 env.callBat(newArgs);

 return env.exitCode();
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


