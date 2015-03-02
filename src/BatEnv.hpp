#ifndef BATENV_HPP__HUNYOSI
#define BATENV_HPP__HUNYOSI


#include <map>
#include <string>

#include "spstr.hpp"

#include "BatExecList.hpp"


class BatEnv
{
private:
 SpStr m_batPath;
 SpVecSpStr m_args;
 std::map< std::string, size_t > m_labels;
 size_t m_pc;
 int m_exitCode;

public:
 int callBat(SpVecSpStr args, BatExecList & execList);
 int exitCode() const { return m_exitCode; }

private:
 int evalLine(Str const & line, BatExecList & execList);
 int evalLineBody(
   Str lineBody,
   Str const & origLine,
   BatExecList & execList);

 int doRem(Str & line, Str const & origLine, BatExecList & execList);
 int doSet(Str & line, Str const & origLine, BatExecList & execList);
 int doIf(Str & line, Str const & origLine, BatExecList & execList);
 int doGoto(Str & line, Str const & origLine, BatExecList & execList);
 int doCall(Str & line, Str const & origLine, BatExecList & execList);
 int doEcho(Str & line, Str const & origLine, BatExecList & execList);
 int doDel(Str & line, Str const & origLine, BatExecList & execList);
 int doMkdir(Str & line, Str const & origLine, BatExecList & execList);
 int doCopy(Str & line, Str const & origLine, BatExecList & execList);
 int doExec(Str & line, Str const & origLine, BatExecList & execList);
};

#endif /* BATENV_HPP__HUNYOSI */
