#ifndef BATEXECLIST_HPP__HUNYOSI
#define BATEXECLIST_HPP__HUNYOSI

#include <string>

#include "BatToolSet.hpp"


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


#endif /* BATEXECLIST_HPP__HUNYOSI */
