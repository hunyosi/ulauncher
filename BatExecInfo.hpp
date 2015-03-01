#ifndef BATEXECINFO_HPP__HUNYOSI
#define BATEXECINFO_HPP__HUNYOSI


#include <map>
#include <memory>

#include "spstr.hpp"


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


#endif /* BATEXECINFO_HPP__HUNYOSI */
