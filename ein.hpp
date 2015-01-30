#define EIN_HPP__HUNYOSI
#define EIN_HPP__HUNYOSI


#include <memory>
#include <vector>
#include <iostream>


/* ****************************************************
 *  EIN -- Extended INI
 * ****************************************************/
namespace ein {

 class EinEntry;
 typedef std::shared_ptr< EinEntry > SpEinEntry;
 class EinSection;
 typedef std::shared_ptr< EinSection > SpEinSection;
 class EinNode;
 typedef std::shared_ptr< EinNode > SpEinNode;


 class EinEntry : public std::vector< std::string >
 {
 private:
  SpEinNode m_children;
 public:
  EinEntry();
  SpEinNode children() { return m_children; }
  void children(SpEinNode node) { m_children = node; }
 };


 class EinEntries : public std::vector< SpEinEntry >
 {
 public:
  SpEinEntry
  entry(std::string const & key)
  {
   for (auto entry : *this) {
    for (auto elm : *entry) {
     if (elm == key) {
      return entry;
     }
    }
   }

   SpEinEntry dmy;
   return dmy;
  }
 };


 class EinSection : public EinEntries
 {
 private:
  std::string m_name;

 public:
  std::string const & name() const { return m_name; }
  void name(std::string const & nameVal) { m_name = nameVal; }
 };


 class EinNode : public EinEntries
 {
 private:
  std::vector< SpEinSection > m_sections;

 public:
  std::vector< SpEinSection > & sections() { return m_sections; }

  SpEinSection section(std::string const & key)
  {
   for (auto section : m_sections) {
    if (section->name() == key) {
     return section;
    }
   }

   SpEinSection dmy;
   return dmy;
  }

 };


 inline
 EinEntry::EinEntry()
   :
   m_children(new EinNode())
 {
 }


 class EinParser
 {
 public:
  SpEinNode
  parse(
    std::istream & istrm)
  {
   SpEinNode node(new EinNode());
   return node;
  }
 };

}


#endif /* EIN_HPP__HUNYOSI */
