#include "strutil.hpp"


void
rtrim(
  std::string& str,
  const std::string& trimChrs)
{
 std::string::size_type p = str.find_last_not_of(trimChrs);
 if (p == std::string::npos) {
  str.erase(0);
  return;
 }

 ++ p;
 str.erase(p);
}


void
ltrim(
  std::string& str,
  const std::string& trimChrs)
{
 std::string::size_type p = str.find_first_not_of(trimChrs);
 str.erase(0, p);
}


void
trim(
  std::string& str,
  const std::string& trimChrs)
{
 rtrim(str, trimChrs);
 ltrim(str, trimChrs);
}


void
chomp(
  std::string& str)
{
 std::string::size_type p = str.find_last_not_of("\r\n");
 if (p == std::string::npos) {
  return;
 }

 ++ p;
 str.erase(p);
}

void
toUpperCase(
  std::string& str)
{
 for (size_t i = 0, z = str.size(); i < z; ++ i) {
  char c = str[i];
  if ('a' <= c && c <= 'z') {
   str[i] = 'A' + (c - 'a');
  }
 }
}


