#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>

#include "strutil.hpp"
#include "fsutil.hpp"
#include "envutil.hpp"
#include "inifile.hpp"


static std::string
getModFileName(
  HINSTANCE modHandle)
{
 DWORD readSize;
 std::vector< TCHAR > buf(MAX_PATH);

 for (;;) {
  readSize = ::GetModuleFileName(modHandle, &buf[0], buf.size());
  if (readSize == 0) {
   buf[0] = 0;
   break;
  }

  if (readSize < buf.size() - 2) {
   break;
  }

  buf.resize(buf.size() * 2, 0);
 }

 std::string s(&buf[0]);
 return s;
}


static std::string
getUtauPath(
  char const * exeDirPath)
{
 std::string baseDir(exeDirPath);
 std::string iniFile(baseDir + "ulauncher.ini");

 std::string utauPath(getEnvVarVal("UTAU_PATH"));
 if (0 < utauPath.size()) {
  return utauPath;
 }

 if (existsFile(iniFile.c_str())) {
  SpMapStr cfg = readIniFileSimple(iniFile.c_str());
  if (0 < (*cfg)["utauExePath"].size()) {
   return (*cfg)["utauExePath"];
  }
 }

 std::string utauExePath1(baseDir + "utau.exe");
 if (existsFile(utauExePath1.c_str())) {
  return utauExePath1;
 }

 SpStr utauExePath2 = expandEnvVar("%ProgramFiles%\\UTAU\\utau.exe");
 if (existsFile(utauExePath2->c_str())) {
  return *utauExePath2;
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


