#include <windows.h>
#include <vector>

#include "spstr.hpp"
#include "chrcnv.hpp"

#include "strutil.hpp"
#include "randutil.hpp"

#include "fsutil.hpp"


std::string
getDirName(
  std::string const & path)
{
 SpWStr wPath = convStrToWStr(path.c_str());
 WStr::size_type p = wPath->find_last_of(L"\\/:");
 if (p == WStr::npos) {
  p = 0;
 } else {
  ++ p;
 }

 WStr wDirName(*wPath, 0, p);
 SpStr dirName = convWStrToStr(wDirName.c_str());
 return *dirName;
}


std::string
getFileName(
  std::string const & path)
{
 SpWStr wPath = convStrToWStr(path.c_str());
 WStr::size_type p = wPath->find_last_of(L"\\/:");
 if (p == WStr::npos) {
  p = 0;
 } else {
  ++ p;
 }

 WStr wFileName(*wPath, p, wPath->size() - p);
 SpStr fileName = convWStrToStr(wFileName.c_str());
 return *fileName;
}


std::string
getBaseName(
  std::string const & path)
{
 std::string fileName(getFileName(path));

 std::string::size_type p = fileName.find_last_of(".");
 if (p != std::string::npos) {
  fileName.erase(p);
 }

 return fileName;
}


std::string
getExtName(
  std::string const & path)
{
 std::string fileName(getFileName(path));

 std::string::size_type p = fileName.find_last_of(".");
 if (p == std::string::npos) {
  return "";
 }

 fileName.erase(0, p);
 return fileName;
}


std::string
eraseLastPathSep(
  std::string const & path)
{
 SpWStr wPath = convStrToWStr(path.c_str());
 wchar_t c = (*wPath)[wPath->size() - 1];
 if (c == L'\\' || c == L'/') {
  wPath->erase(wPath->size() - 1);
 }

 SpStr erased = convWStrToStr(wPath->c_str());
 return *erased;
}


std::string
toFullPathName(
  std::string const & path)
{
 TCHAR *p;
 std::vector< TCHAR > buf(MAX_PATH);
 DWORD s;
 s = ::GetFullPathName(path.c_str(), buf.size(), &buf[0], &p);
 if (buf.size() - 1 < s) {
  buf.resize(s, 0);
  s = ::GetFullPathName(path.c_str(), buf.size(), &buf[0], &p);
 }

 if (s < 1) {
  buf[0] = 0;
 }

 std::string str(&buf[0]);
 return str;
}


std::string
getCurDir()
{
 std::vector< TCHAR > buf(MAX_PATH);
 DWORD s;
 s = ::GetCurrentDirectory(buf.size(), &buf[0]);
 if (buf.size() - 1 < s) {
  buf.resize(s, 0);
  s = ::GetCurrentDirectory(buf.size(), &buf[0]);
 }

 if (s < 1) {
  buf[0] = 0;
 }

 std::string str(&buf[0]);
 return str;
}


bool
setCurDir(
 std::string const & path)
{
 return ::SetCurrentDirectory(path.c_str()) != 0;
}


bool
existsFile(char const * path)
{
 DWORD attrs = ::GetFileAttributes(path);
 if (attrs == (DWORD)-1) {
  return false;
 }

 return ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0);
}


bool
existsDir(char const * path)
{
 DWORD attrs = ::GetFileAttributes(path);
 if (attrs == (DWORD)-1) {
  return false;
 }

 return ((attrs & FILE_ATTRIBUTE_DIRECTORY) != 0);
}


int
existsFileOrDir(char const * path)
{
 DWORD attrs = ::GetFileAttributes(path);
 if (attrs == (DWORD)-1) {
  return false;
 }

 return ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0 ? 1 : -1);
}


bool
existsPath(char const * path)
{
 DWORD attrs = ::GetFileAttributes(path);
 if (attrs == (DWORD)-1) {
  return false;
 }

 return true;
}


std::string
getTempDir()
{
 std::vector< TCHAR > buf(MAX_PATH);
 DWORD s;
 s = ::GetTempPath(buf.size(), &buf[0]);
 if (buf.size() - 1 < s) {
  buf.resize(s, 0);
  s = ::GetTempPath(buf.size(), &buf[0]);
 }

 if (s < 1) {
  buf[0] = 0;
 }

 std::string str(&buf[0]);
 return str;
}


std::string
makeTempDir()
{
 std::string baseDir(getTempDir());
 std::string tempDir;
 std::string randDirName;

 do {
  randDirName = randStr();
  tempDir = baseDir + randDirName;
 } while (existsPath(tempDir.c_str()));

 if (::CreateDirectory(tempDir.c_str(), NULL) == 0) {
  return "";
 }

 return tempDir + "\\";
}




class WinHandleHolder
{
private:
 HANDLE m_handle;

public:
 WinHandleHolder(HANDLE handle) : m_handle(handle) {}

 ~WinHandleHolder()
 {
  close();
 }

 operator HANDLE() const { return m_handle; }

 void close()
 {
  if (m_handle) {
   ::CloseHandle(m_handle);
   m_handle = NULL;
  }
 }

private:
 WinHandleHolder(WinHandleHolder const &);
 WinHandleHolder & operator=(WinHandleHolder const &);
};




int
concatBinFiles(
  SpVecSpStr srcFiles,
  const char *destPath)
{
 HANDLE fileHandle = ::CreateFile(destPath, GENERIC_WRITE,
   0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 if (fileHandle == INVALID_HANDLE_VALUE) {
  return -1;
 }

 WinHandleHolder fh(fileHandle);
 std::vector< unsigned char > buf(4096);
 DWORD readSize;
 DWORD wroteSize;
 size_t pos;

 for (SpStr srcFile : *srcFiles) {
  HANDLE fileHandle2 = ::CreateFile(srcFile->c_str(), GENERIC_READ,
    0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (fileHandle2 == INVALID_HANDLE_VALUE) {
   return -1;
  }

  WinHandleHolder fh2(fileHandle2);

  for (;;) {
   if (! ::ReadFile(fh2, &buf[0], buf.size(), &readSize, NULL)) {
    return -1;
   }

   if (readSize < 1) {
    break;
   }

   pos = 0;
   do {
    if (! ::WriteFile(fh, &buf[pos], readSize, &wroteSize, NULL)) {
     return -1;
    }

    pos += wroteSize;
    readSize -= wroteSize;
   } while (0 < readSize);
  }

  fh2.close();
 }

 fh.close();
 return 0;
}


int
writeBinFile(
  const char * destPath,
  void * data,
  size_t dataSize)
{
 unsigned char * buf =(unsigned char *) data;
 HANDLE fileHandle = ::CreateFile(destPath, GENERIC_WRITE,
   0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 if (fileHandle == INVALID_HANDLE_VALUE) {
  return -1;
 }

 WinHandleHolder fh(fileHandle);
 DWORD writeSize = (DWORD) dataSize;
 DWORD wroteSize;
 size_t pos;

 pos = 0;
 do {
  if (! ::WriteFile(fh, &buf[pos], writeSize, &wroteSize, NULL)) {
   return -1;
  }

  pos += wroteSize;
  writeSize -= wroteSize;
 } while (0 < writeSize);

 fh.close();
 return 0;
}



class FileFinderImpl : public FileFinder::IFileFinder
{
private:
 std::string m_pattern;
 HANDLE m_handle;
 WIN32_FIND_DATA m_wfd;

public:
 FileFinderImpl(
   std::string const & pattern)
   :
   m_pattern(pattern),
   m_handle(INVALID_HANDLE_VALUE)
 {
 }


 ~FileFinderImpl()
 {
  close();
 }


 virtual void
 close()
 {
  if (m_handle != INVALID_HANDLE_VALUE) {
   ::FindClose(m_handle);
   m_handle = INVALID_HANDLE_VALUE;
  }
 }


 virtual bool
 next()
 {
  if (m_handle == INVALID_HANDLE_VALUE) {
   m_handle = ::FindFirstFile(m_pattern.c_str(), &m_wfd);
   return m_handle != INVALID_HANDLE_VALUE;
  } else {
   return ::FindNextFile(m_handle, &m_wfd) != FALSE;
  }
 }


 virtual SpStr
 getFileName() const
 {
  SpStr fname(new std::string(m_wfd.cFileName));
  return fname;
 }


 WIN32_FIND_DATA const & getInfo() const { return m_wfd; }


private:
 FileFinderImpl(FileFinder const &);
 FileFinderImpl & operator=(FileFinder const &);
};


FileFinder::FileFinder(
  std::string const & pattern)
  :
  m_body(new FileFinderImpl(pattern))
{
}



bool
deleteRecursive(
  std::string const & path)
{
 std::string delPtn(eraseLastPathSep(path));
 std::string dirPath(getDirName(delPtn));

 VecSpStr files;
 VecSpStr dirs;
 FileFinderImpl ff(delPtn);
 while (ff.next()) {
  SpStr name(new std::string(ff.getInfo().cFileName));
  if (*name != "." && *name != "..") {
   if (ff.getInfo().dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    dirs.push_back(name);
   } else {
    files.push_back(name);
   }
  }
 }

 ff.close();

 bool success = true;

 for (SpStr name : files) {
  std::string targetPath(dirPath);
  targetPath.append(*name);
  if (! ::DeleteFile(targetPath.c_str())) {
   success = false;
  }
 }

 for (SpStr name : dirs) {
  std::string targetPath(dirPath);
  targetPath.append(*name);
  std::string dirPtn(targetPath);
  dirPtn.append("\\*");
  if (deleteRecursive(dirPtn.c_str())) {
   if (! ::RemoveDirectory(targetPath.c_str())) {
    success = false;
   }
  } else {
   success = false;
  }
 }

 return success;
}


