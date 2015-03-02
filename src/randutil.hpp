#ifndef RANDUTIL_HPP__HUNYOSI
#define RANDUTIL_HPP__HUNYOSI


#include <string>


int
randInt(
  int minVal,
  int maxVal);


char
randChar(
  std::string const & chars);


std::string
randStr(
  int minLen = 4,
  int maxLen = 8,
  std::string const & chars = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
  std::string const & firstChars = "",
  std::string const & lastChars = "");


#endif /* RANDUTIL_HPP__HUNYOSI */
