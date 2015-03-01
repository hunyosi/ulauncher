#include <climits>

#include "miscutil.hpp"


int
noteNameToNoteNumber(
  char const * noteName)
{
 static int const nameToNum[] = {9, 11, 0, 2, 4, 5, 7};
 int noteNoBase, numPos, octave;

 if (! noteName || noteName[0] == '\0') {
  return INT_MIN;
 }

 noteNoBase = nameToNum[noteName[0] - 'A'];
 if (noteName[1] == '#') {
  noteNoBase += 1;
  numPos = 2;
 } else {
  numPos = 1;
 }

 if (noteName[numPos] == '-') {
  octave = - (noteName[numPos + 1] - '0');
 } else {
  octave = noteName[numPos] - '0';
 }

 return (octave + 1) * 12 + noteNoBase;
}


std::size_t
getWavFileSize(
  void * wavData)
{
 unsigned char * buf = (unsigned char *) wavData;
 return 8 + ((buf[7] << 24) | (buf[6] << 16) | (buf[5] << 8) | buf[4]);
}


std::size_t
getFrqFileSize(
  void * frqData)
{
 unsigned char * buf = (unsigned char *) frqData;
 return 40 + 8 * 2 * ((buf[39] << 24) | (buf[38] << 16) | (buf[37] << 8) | buf[36]);
}
