#include <iostream>
#include <fstream>
#include <windows.h>

#include "cmdlutil.hpp"
#include "envutil.hpp"
#include "fsutil.hpp"
#include "strutil.hpp"

#include "BatEnv.hpp"


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
  SpStr fname(ff.getFileName());
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

