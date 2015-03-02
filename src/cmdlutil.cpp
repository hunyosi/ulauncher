#include <windows.h>
#include <memory>

#include "chrcnv.hpp"
#include "cmdlutil.hpp"


std::string
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


SpVecSpStr
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


