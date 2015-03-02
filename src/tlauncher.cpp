#include <iostream>
//#include <clocale>
#include <windows.h>

#ifdef DEBUG
#include <sstream>
#endif

#include "spstr.hpp"
#include "strutil.hpp"
#include "fsutil.hpp"
#include "cmdlutil.hpp"

#include "BatExecList.hpp"
#include "BatEnv.hpp"



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


