#ifndef SPSTR_HPP__HUNYOSI
#define SPSTR_HPP__HUNYOSI

#include <string>
#include <vector>
#include <memory>


typedef std::wstring WStr;
typedef std::shared_ptr< WStr > SpWStr;
typedef std::vector< SpWStr > VecSpWStr;
typedef std::shared_ptr< VecSpWStr > SpVecSpWStr;


typedef std::string Str;
typedef std::shared_ptr< Str > SpStr;
typedef std::vector< SpStr > VecSpStr;
typedef std::shared_ptr< VecSpStr > SpVecSpStr;


#endif /* SPSTR_HPP__HUNYOSI */
