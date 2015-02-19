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

#include "spstr.hpp"
#include "chrcnv.hpp"
#include "strutil.hpp"
#include "fsutil.hpp"
#include "vbmod.hpp"
#include "cmdlutil.hpp"
#include "envutil.hpp"
#include "inifile.hpp"




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


