#include <iostream>
#include <fstream>

#include "strutil.hpp"

#include "inifile.hpp"


SpMapVec
readIniFile(
  char const * path)
{
 SpMapVec p;
 SpMapVec mp( new MapVec() );
 std::string prevKey = "";

 std::vector< char > buf(4096);
 std::ifstream ifs(path);
 while (ifs.good() && ! ifs.getline(&buf[0], buf.size()).eof()) {
  std::string line(&buf[0]);
  trim(line);
  if (line[0] == '#') {
   continue;
  }

  std::string::size_type p = line.find('=');
  if (p == std::string::npos) {
   std::vector< std::string > & vec = (*mp)[prevKey];
   vec.push_back(line);
   continue;

  } else {
   std::string key(line, 0, p);
   std::string val(line, p + 1, line.size() - (p + 1));
   trim(key);
   trim(val);
   std::vector< std::string > & vec = (*mp)[key];
   if (0 < val.size()) {
    vec.push_back(val);
   }

   prevKey = key;
  }
 }

 if (ifs.bad()) {
  return p;
 }

 return mp;
}
