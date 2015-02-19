#ifndef CMDLUTIL_HPP__HUNYOSI
#define CMDLUTIL_HPP__HUNYOSI


#include <stdexcept>
#include <string>

#include "spstr.hpp"


std::string
cmdLineEncoding(
 char const *cmdLine);


class ParseCommandLineError : public std::runtime_error
{
public:
 explicit ParseCommandLineError() :
   std::runtime_error("ParseCommandLineError") {}
};


SpVecSpStr
parseCommandLine(
  char const * cmdLine);


#endif /* CMDLUTIL_HPP__HUNYOSI */
