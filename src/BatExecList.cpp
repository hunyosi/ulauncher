#include <iostream>
#include <memory>
#include <windows.h>

#include "fsutil.hpp"
#include "miscutil.hpp"

#include "BatExecInfo.hpp"
#include "VoicebankModule.hpp"

#include "BatExecList.hpp"


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

