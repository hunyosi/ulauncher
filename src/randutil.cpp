#include <cstdlib>

#include "randutil.hpp"


int
randInt(
  int minVal,
  int maxVal)
{
 int trueMinVal = minVal;
 int trueMaxVal = maxVal;
 if (trueMinVal == trueMaxVal) {
  return trueMinVal;
 } if (trueMaxVal < trueMinVal) {
  trueMinVal = maxVal;
  trueMaxVal = minVal;
 }
 return (int)(((double)std::rand() * (trueMaxVal - trueMinVal)) / RAND_MAX + trueMinVal);
}


char
randChar(
  std::string const & chars)
{
 return chars[randInt(0, (int)chars.size() - 1)];
}


std::string
randStr(
  int minLen,
  int maxLen,
  std::string const & chars,
  std::string const & firstChars,
  std::string const & lastChars)
{
 int len = randInt(minLen, maxLen);
 if (len < 1) {
  return "";
 }

 std::string buf(len, 0);
 if (0 < firstChars.size()) {
  buf[0] = randChar(firstChars);
 } else {
  buf[0] = randChar(chars);
 }
 if (len == 1) {
  return buf;
 }

 int i = 1;
 int len2 = len - 1;
 for (; i < len2; ++ i) {
  buf[i] = randChar(chars);
 }

 if (0 < lastChars.size()) {
  buf[i] = randChar(lastChars);
 } else {
  buf[i] = randChar(chars);
 }

 return buf;
}


