#ifndef BATTOOLSET_HPP__HUNYOSI
#define BATTOOLSET_HPP__HUNYOSI


#include <memory>
#include <string>
#include <vector>

#include "BatExecInfo.hpp"


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


#endif /* BATTOOLSET_HPP__HUNYOSI */
