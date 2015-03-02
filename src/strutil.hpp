#ifndef STRUTIL_HPP__HUNYOSI
#define STRUTIL_HPP__HUNYOSI


#include <string>


void
rtrim(
  std::string& str,
  const std::string& trimChrs = " \t\n\r\v\f");


void
ltrim(
  std::string& str,
  const std::string& trimChrs = " \t\n\r\v\f");


void
trim(
  std::string& str,
  const std::string& trimChrs = " \t\n\r\v\f");


void
chomp(
  std::string& str);


void
toUpperCase(
  std::string& str);


#endif /* STRUTIL_HPP__HUNYOSI */
