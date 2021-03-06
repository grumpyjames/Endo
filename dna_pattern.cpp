#include "dna_pattern.h"

#include <iostream>
#include <stdexcept>

namespace {
  using namespace endo;

  using std::get;
  using std::make_shared;
  using std::shared_ptr;
  using std::string;

  class raw_bases : public pattern_element {
  public:
    explicit raw_bases(string const & bases) : raw_bases_(bases) {}
    dna replace(template_element t, dna const & d) {
      return d;
    }
    string to_string() {
      return raw_bases_;
    }
  private:
    string const raw_bases_;
  };

  class skip : public pattern_element {
  public:
    explicit skip(unsigned int length) : length_(length) {}
    dna replace(template_element t, dna const & d) {
      return d;
    }
    string to_string() {
      return string("!") + std::to_string(length_);
    }
  private:
    unsigned int const length_;
  };

  class search : public pattern_element {
  public:
    explicit search(string && target) : target_(std::move(target)) {}
    dna replace(template_element t, dna const & d) {
      return d;
    }
    string to_string() {
      return string("?") + target_;
    }
  private:
    string const target_;
  };

  class open_brace : public pattern_element {
  public:
    dna replace(template_element t, dna const & d) {
      return d;
    }
    string to_string() {
      return string("(");
    }
  };

  class close_brace : public pattern_element {
  public:
    dna replace(template_element t, dna const & d) {
      return d;
    }
    string to_string() {
      return string(")");
    }
  };

  string make_string(pattern const & p) {
    std::string result;
    for (pattern::const_iterator it = p.begin(), ee=p.end(); it != ee; ++it)
      result += (**it).to_string();
    return result;
  }

  void push(pattern & p, std::string const & c) {
    
  }

  void push(pattern & p, unsigned int skip_length) {
    p.push_back(make_shared<skip>(skip_length));
  }

  std::tuple<unsigned int, dna> fake_nat(dna const & d) {
    return std::tuple<unsigned int, dna>(4, dna());
  }

  std::tuple<unsigned int, dna> another_fake_nat(dna const & d) {
    return std::tuple<unsigned int, dna>(4, dna("CFPIC"));
  }

  static size_t const char_limit = 64;

  // Buffer raw_bases such that we collect them as char_limit length strings at most
  class pattern_buffer {
  public:
    pattern_buffer() {
      buffer_.reserve(char_limit);
    }
    void push(char c) {
      if (buffer_.size() == char_limit)
	flush();
      buffer_ += c;
    }
    void push(shared_ptr<pattern_element> fragment) {
      if (buffer_.size() > 0)
	flush();
      p_.push_back(fragment);
    }
    pattern build() {
      flush();
      return p_;
    }
  private:
    void flush() {
      p_.push_back(make_shared<raw_bases>(buffer_));
      buffer_.clear();
    }   
    std::string buffer_;
    pattern p_;
  };

  pattern_with_dna inner_parse_pattern(pattern_buffer & p, dna const & dna, natFn nat) {
    std::string buffer;
    for (char c: dna) {
      if (buffer.length() > 0) {
	buffer += c;
	if (buffer == "IC") {
	  p.push('P');
	  buffer.clear();
	} else if (buffer == "IP") {
	  auto nat_result = nat(dna);
	  p.push(make_shared<skip>(get<0>(nat_result)));
	  return inner_parse_pattern(p, get<1>(nat_result), nat);
	}
      } else if (c == 'C') {
	p.push('I');
      } else if (c == 'F') {
	p.push('C');
      } else if (c == 'P') {
	p.push('F');
      } else {
	buffer += c;
      }
    }
    return endo::pattern_with_dna(p.build(), dna);
  }

  std::string repeat(std::string const & base, size_t const repetitions) {
    std::string builder;    
    for (size_t s(0); s < repetitions; ++s)
      builder += base;
    return builder;
  }

  void check_length(pattern const & p, size_t expected_size) {
    if (p.size() != expected_size)
      throw std::runtime_error("p has size " + std::to_string(p.size())
			       + " but was supposed to have size " + std::to_string(expected_size));
  }

  void check_equal(string const & expected, string const & actual) {
    if (expected != actual)
      throw std::runtime_error(expected + " was not equal to " + actual);
  }

  void check_equal(string const & expected_pattern, pattern const & p) {
    string const pattern_as_string(make_string(p));
    check_equal(expected_pattern, pattern_as_string);
  }
}

endo::pattern_with_dna endo::parse_pattern(endo::dna const & dna, endo::natFn nat) {
  pattern_buffer builder;  
  return inner_parse_pattern(builder, dna, nat);
}


int main(int argc, char * argv[]) {
  check_equal("ICFP", get<0>(parse_pattern(dna("CFPIC"), fake_nat)));
  check_equal("PFCI", get<0>(parse_pattern(dna("ICPFC"), fake_nat)));
  check_equal("PFCI", get<0>(parse_pattern(dna("ICPFCII"), fake_nat)));  
  check_equal("IC!4", get<0>(parse_pattern(dna("CFIP"), fake_nat)));
  check_equal("IC!4ICFP", get<0>(parse_pattern(dna("CFIP"), another_fake_nat)));
  
  
  auto result = get<0>(parse_pattern(dna(repeat("CFPIC", 17)), another_fake_nat));
  check_equal(repeat("ICFP", 17), result);
  check_length(result, 2);

  auto result_with_leftover = parse_pattern(dna("CFPICIICCFPI"), fake_nat);
  check_equal("ICFP", get<0>(result_with_leftover));
  check_equal("CFPI", get<1>(result_with_leftover));

  std::cout << "All tests passed" << std::endl;
}
