#ifndef FSUTIL_HPP__HUNYOSI
#define FSUTIL_HPP__HUNYOSI


#include <string>

#include "spstr.hpp"


std::string
getDirName(
  std::string const & path);


std::string
getFileName(
  std::string const & path);


std::string
getBaseName(
  std::string const & path);


std::string
getExtName(
  std::string const & path);


std::string
eraseLastPathSep(
  std::string const & path);


std::string
toFullPathName(
  std::string const & path);


std::string
getCurDir();


bool
setCurDir(
 std::string const & path);


bool
existsFile(char const * path);


bool
existsDir(char const * path);


int
existsFileOrDir(char const * path);


bool
existsPath(char const * path);


std::string
getTempDir();


std::string
makeTempDir();


int
concatBinFiles(
  SpVecSpStr srcFiles,
  const char *destPath);


int
writeBinFile(
  const char * destPath,
  void * data,
  size_t dataSize);


class FileFinder
{
public:
 class IFileFinder {
 public:
  virtual void close() = 0;
  virtual bool next() = 0;
  virtual SpStr getFileName() const = 0;
 };

private:
 IFileFinder * m_body;

public:
 FileFinder(std::string const & pattern);

 ~FileFinder()
 {
  delete m_body;
 }

 void close()
 {
  m_body->close();
 }

 bool next()
 {
  return m_body->next();
 }

 SpStr getFileName() const
 {
  return m_body->getFileName();
 }

private:
 FileFinder(FileFinder const &);
 FileFinder & operator=(FileFinder const &);
};



bool
deleteRecursive(
  std::string const & path);


#endif /* FSUTIL_HPP__HUNYOSI */
