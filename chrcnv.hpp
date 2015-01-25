#ifndef CHRCNV_HPP__HUNYOSI
#define CHRCNV_HPP__HUNYOSI

#include <stdexcept>

#include "spstr.hpp"


class ConvStrToWStrError : public std::runtime_error
{
public:
 explicit ConvStrToWStrError() :
   std::runtime_error("ConvStrToWStrError") {}
};

SpWStr
convStrToWStr(
  char const * s);


class ConvWStrToStrError : public std::runtime_error
{
public:
 explicit ConvWStrToStrError() :
   std::runtime_error("ConvWStrToStrError") {}
};

SpStr
convWStrToStr(
  wchar_t const * ws);


#endif /* CHRCNV_HPP__HUNYOSI */

