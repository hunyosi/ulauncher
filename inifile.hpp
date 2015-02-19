#ifndef INIFILE_HPP__HUNYOSI
#define INIFILE_HPP__HUNYOSI


#include <string>
#include <memory>
#include <vector>
#include <map>


typedef std::map<std::string, std::vector< std::string > > MapVec;
typedef std::shared_ptr< MapVec > SpMapVec;

SpMapVec
readIniFile(
  char const * path);


#endif /* INIFILE_HPP__HUNYOSI */
