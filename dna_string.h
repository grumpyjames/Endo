#include <deque>
#include <utility>
#include <cstring>
#include <iostream>

typedef std::pair<char const *, char const *> ends;
typedef std::pair<size_t, char const *> location;

class dna_string {
 public: 
 dna_string() : innards(),current_index_(0) {}
 dna_string(std::deque<ends> initial_innards) : innards(initial_innards) { reset();} 
  void operator++() {
    if (current_location_ == innards[current_index_].second)
      current_location_ = innards[++current_index_].first;
    else
      ++current_location_;
  }
  void operator--() {
    if (current_location_ == innards[current_index_].first)
      current_location_ = innards[--current_index_].second;
    else
      --current_location_; 
  }
  void operator+=(unsigned int const to_add) {
    if (current_location_ <= (innards[current_index_].second-to_add)) {
      current_location_+=to_add;
    }
    else {
      size_t moved = 1 + innards[current_index_].second - current_location_;
      current_location_ = innards[++current_index_].first;
      (*this)+=(to_add - moved);
    }
  }
  void operator-=(unsigned int const to_subtract) {
    if (current_location_ >= (innards[current_index_].first+to_subtract)) {
      current_location_-=to_subtract;
    }
    else {
      size_t moved = current_location_ - innards[current_index_].first;
      current_location_ = innards[--current_index_].second;
      (*this)-=(to_subtract - moved);
    }
  }
  char const * beginning() { return innards.front().first; }
  char const * end() { return innards.back().second; }
  char const & get() { return *current_location_; }
  char const * get_char_ptr() { return current_location_; }
  bool const has_next() { return !(current_location_==end()); }
  void substr_from(location const & from, dna_string & target);
  void push_back(ends const & to_push) {
    innards.push_back(to_push);
    if (innards.size() == 1) {
      reset();
    }
  }
  void push_back(char const to_push);
  void push_front(ends const & to_push) { innards.push_front(to_push); }
  void skip_to_first(char const & to_find) {
    while (has_next() && *current_location_!=to_find)
      ++(*this);
  }
  void save_position(location & to_save_to) {
    to_save_to.first = current_index_;
    to_save_to.second = current_location_;
  }
  void load_position(location const & to_load_from) {
    current_index_ = to_load_from.first;
    current_location_ = to_load_from.second;
  }
  void push_to(dna_string & rna, size_t const no_of_chars) {
    if (current_location_+no_of_chars <= innards[current_index_].second) {
      rna.push_back(ends(current_location_,current_location_+no_of_chars-1));
      (*this)+=no_of_chars;
    }
    else {
      rna.push_back(ends(current_location_,innards[current_index_].second));
      size_t chars_done = 1 + innards[current_index_].second - current_location_;
      current_location_ = innards[++current_index_].first;
      push_to(rna, no_of_chars - chars_done);
    }
  }
  void skip_to_first(const char * needle, size_t const length);
  location const current_location() const;
  unsigned int const remaining_length() const;
  unsigned int const innards_size() const;
  void append(dna_string const & to_push);
  void prepend(dna_string const & to_prepend);
  void push_ends_back(ends const & to_push);
  std::deque<ends>::const_iterator begin() const { return innards.begin(); }
  std::deque<ends>::const_iterator end() const { return innards.end(); }
 private:
  void reset() {
    current_index_ = 0;
    current_location_ = innards.front().first;
  }
  std::deque<ends> innards;
  size_t current_index_;
  char const * current_location_;
};
