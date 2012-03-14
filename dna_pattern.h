#include <functional>
#include <list>
#include <memory>
#include <string>
#include <tuple>

namespace endo {

  class template_element {};
  typedef std::string dna;

  class pattern_element {
  public:
    virtual dna replace(template_element t, dna const & d) = 0;
    virtual std::string to_string() = 0;
  protected:
    virtual ~pattern_element() {} 
  };

  typedef std::function<std::tuple<unsigned int, dna>(dna const &)> natFn;
  
  typedef std::list<std::shared_ptr<pattern_element> > pattern;

  typedef std::tuple<pattern, dna> pattern_with_dna;

  pattern_with_dna parse_pattern(dna const & d, natFn nat);
}
