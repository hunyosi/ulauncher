#ifndef ENVUTIL_HPP__HUNYOSI
#define ENVUTIL_HPP__HUNYOSI


#include "spstr.hpp"


SpStr
expandEnvVar(
  char const * str);

SpStr
expandEnvVar(
  char const * str,
  SpVecSpStr args);


#endif /* ENVUTIL_HPP__HUNYOSI */
