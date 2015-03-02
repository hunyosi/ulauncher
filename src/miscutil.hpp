#ifndef MISCUTIL_HPP__HUNYOSI
#define MISCUTIL_HPP__HUNYOSI

#include <cstddef>

int
noteNameToNoteNumber(
  char const * noteName);

std::size_t
getWavFileSize(
  void * wavData);

std::size_t
getFrqFileSize(
  void * frqData);


#endif /* MISCUTIL_HPP__HUNYOSI */
