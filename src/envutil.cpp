#include <windows.h>
#include <memory>
#include <vector>
#include <cstring>
#include <string>

#include "cmdlutil.hpp"
#include "envutil.hpp"


SpStr
expandEnvVar(
  char const * str)
{
 int bufSize = ::ExpandEnvironmentStrings(str, NULL, 0);
 std::vector< char > buf(bufSize);
 ::ExpandEnvironmentStrings(str, &buf[0], bufSize);
 std::shared_ptr< std::string > cmdPath(new std::string(&buf[0]));
 return cmdPath;
}


SpStr
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


std::string
getEnvVarVal(
  char const * envVarName)
{
 char dmyBuf[1];
 DWORD bufSize = ::GetEnvironmentVariable(envVarName, dmyBuf, 0);
 if (bufSize < 1) {
  return "";
 }

 std::vector< char > buf(bufSize);
 bufSize = ::GetEnvironmentVariable(envVarName, &buf[0], bufSize);
 if (bufSize < 1) {
  return "";
 }

 return &buf[0];
}


