#include <iostream>
#include <windows.h>

#include "cmdlutil.hpp"

#include "BatExecInfo.hpp"


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

