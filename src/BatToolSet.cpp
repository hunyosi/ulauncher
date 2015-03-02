#include <iostream>

#include "fsutil.hpp"
#include "strutil.hpp"

#include "BatToolSet.hpp"


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
