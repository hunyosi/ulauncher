#include <windows.h>
#include <vector>

#include "chrcnv.hpp"


SpWStr
convStrToWStr(
  char const * s)
{
 int bufSize = ::MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
 if (bufSize == 0) {
  throw ConvStrToWStrError();
 }

 std::vector< wchar_t > buf(bufSize);
 if (! ::MultiByteToWideChar(CP_ACP, 0, s, -1, &buf[0], bufSize)) {
  throw ConvStrToWStrError();
 }

 SpWStr str(new WStr(&buf[0]));
 return str;
}


SpStr
convWStrToStr(
  wchar_t const * ws)
{
 int bufSize = ::WideCharToMultiByte(CP_ACP, 0, ws, -1, NULL, 0, NULL, NULL);
 if (bufSize == 0) {
  throw ConvWStrToStrError();
 }

 std::vector< char > buf(bufSize);
 if (! ::WideCharToMultiByte(CP_ACP, 0, ws, -1, &buf[0], bufSize, NULL, NULL)) {
  throw ConvWStrToStrError();
 }

 SpStr str(new Str(&buf[0]));
 return str;
}


