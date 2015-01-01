#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>




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




typedef std::map<std::string, std::string> MapStr;
typedef std::shared_ptr< MapStr > SpMapStr;

static SpMapStr
readIniFile(
  char const * path)
{
 SpMapStr p;
 SpMapStr mp( new MapStr() );

 std::vector< char > buf(4096);
 std::ifstream ifs(path);
 while (ifs.good() && ! ifs.getline(&buf[0], buf.size()).eof()) {
  std::string line(&buf[0]);
  std::string::size_type p = line.find('=');
  if (p == std::string::npos) {
   continue;
  }

  std::string key(line, 0, p);
  std::string val(line, p + 1, line.size() - (p + 1));
  trim(key);
  trim(val);
  (*mp)[key] = val;
 }

 if (ifs.bad()) {
  return p;
 }

 return mp;
}


static bool
writeIniFile(
  char const * path,
  SpMapStr mp)
{
 std::ofstream ofs(path);
 if (ofs.bad()) {
  return false;
 }

 for (auto pair : *mp) {
  ofs << pair.first << '=' << pair.second << std::endl;
  if (ofs.bad()) {
   return false;
  }
 }

 return true;
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




static PTSTR
expandEnvVar(
  const char* str)
{
 DWORD bufSize = ::ExpandEnvironmentStrings(str, NULL, 0);
 PTSTR buf = new TCHAR[bufSize];
 ::ExpandEnvironmentStrings(str, buf, bufSize);
 return buf;
}


static std::string
getModFileName(
  HINSTANCE modHandle)
{
 DWORD readSize;
 std::vector< TCHAR > buf(4096);

 for (;;) {
  readSize = ::GetModuleFileName(modHandle, &buf[0], buf.size());
  if (readSize == 0) {
   buf[0] = 0;
   break;
  }

  if (readSize < buf.size() - 1) {
   break;
  }

  buf.resize(buf.size() * 2, 0);
 }

 std::string s(&buf[0]);
 return s;
}


static std::string
toFullPathName(
  std::string const & path)
{
 TCHAR *p;
 std::vector< TCHAR > buf(MAX_PATH);
 DWORD s = ::GetFullPathName(path.c_str(), buf.size(), &buf[0], &p);
 if (s < 1) {
  buf[0] = 0;
 }
 if (buf.size() - 1 < s) {
  ::GetFullPathName(path.c_str(), buf.size(), &buf[0], &p);
  if (s < 1) {
   buf[0] = 0;
  }
 }

 std::string str(&buf[0]);
 return str;
}


static std::string
getDirName(
  std::string const & path)
{
 std::string::size_type p = path.find_last_of("\\/");
 if (p == std::string::npos) {
  p = 0;
 }

 std::string dirName(path, 0, p + 1);
 return dirName;
}


static std::string
getUtauPath(
  char const * exeDirPath)
{
 std::string baseDir(exeDirPath);
 std::string iniFile(baseDir + "ulauncher.ini");

 if (existsFile(iniFile.c_str())) {
  SpMapStr cfg = readIniFile(iniFile.c_str());
  return (*cfg)["utauExePath"];
 }

 std::string utauExePath(baseDir + "utau.exe");
 if (existsFile(iniFile.c_str())) {
  return utauExePath;
 }

 PTSTR utauExePath2 = expandEnvVar("%ProgramFiles%\\UTAU\\utau.exe");
 std::string utauExePath2b(utauExePath2);
 delete[] utauExePath2;
 if (existsFile(utauExePath2b.c_str())) {
  return utauExePath;
 }

 return "";
}


int
main(
  int argc,
  char *argv[])
{
 DWORD exitCode = -1;
 LPTSTR cmdLine = ::GetCommandLine();

 std::string exePath = getModFileName(NULL);
 std::string exeFullPath = toFullPathName(exePath);
 std::string exeDirPath = getDirName(exeFullPath);
 std::string utauExePath = getUtauPath(exeDirPath.c_str());
 if (utauExePath.size() == 0 || ! existsFile(utauExePath.c_str())) {
  std::cout << "ulancher: Not found utau.exe" << std::endl;
  return -1;
 }

 std::string tlauncherPath = exeDirPath + "tlauncher.exe";
 if (! existsFile(tlauncherPath.c_str())) {
  std::cout << "ulancher: Not found tlauncher.exe" << std::endl;
  return -1;
 }

 ::SetEnvironmentVariable("ComSpec", tlauncherPath.c_str());

 STARTUPINFO si;
 GetStartupInfo(&si);

 PROCESS_INFORMATION pi;

 if (::CreateProcess(
   utauExePath.c_str(), cmdLine,
   NULL, NULL, FALSE,
   NORMAL_PRIORITY_CLASS,
   NULL, NULL,
   &si, &pi)) {
  ::WaitForSingleObject(pi.hProcess, INFINITE);
  ::GetExitCodeProcess(pi.hProcess, &exitCode);
  ::CloseHandle(pi.hThread);
  ::CloseHandle(pi.hProcess);
 }

 std::stringstream ss;
 ss << "exitCode=" << exitCode;

 return exitCode;
}


