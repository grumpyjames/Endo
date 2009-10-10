#include <deque>
#include <utility>
#include <cstring>

typedef std::pair<char const *, char const *> ends;
typedef std::pair<size_t, char const *> location;

class dna_string {
 public:
  dna_string() {};
 dna_string(std::deque<ends> initial_innards) : innards(initial_innards) { reset();} 
  void operator++() {
    if (current_location == innards[current_index].second)
      current_location = innards[++current_index].first;
    else
      ++current_location;
  }
  void operator--() {
    if (current_location == innards[current_index].first)
      current_location = innards[--current_index].second;
    else
      --current_location; 
  }
  void operator-=(unsigned int const to_subtract) {
    if (current_location >= (innards[current_index].first+to_subtract)) {
      current_location-=to_subtract;
    }
    else {
      size_t moved = current_location - innards[current_index].first;
      current_location = innards[--current_index].second;
      (*this)-=(to_subtract - moved);
    }
  }
  char const * beginning() { return innards.front().first; }
  char const * end() { return innards.back().second; }
  char const & get() { return *current_location; }
  char const * get_char_ptr() { return current_location; }
  bool const has_next() { return !(current_location==end()); }
  void push_back(ends const & to_push) { 
    innards.push_back(to_push);
    if (innards.size() == 1) {
      current_location = innards.front().first;
      current_index = 0;
    }
  }
  void push_front(ends const & to_push) { innards.push_front(to_push); }
  void skip_to_first(char const & to_find) {
    while (has_next() && *current_location!=to_find)
      ++(*this);
  }
  void save_position(location & to_save_to) {
    to_save_to.first = current_index;
    to_save_to.second = current_location;
  }
  void load_position(location & to_load_from) {
    current_index = to_load_from.first;
    current_location = to_load_from.second;
  }
  void push_to(dna_string & rna, size_t const no_of_chars) {
    if (current_location+7 <= innards[current_index].second) {
      rna.push_back(ends(current_location,current_location+7));
      current_location+=7;
    }
    else {
      rna.push_back(ends(current_location,innards[current_index].second));
      size_t chars_done = innards[current_index].second - current_location;
      current_location = innards[++current_index].first;
      push_to(rna, no_of_chars - chars_done);
    }
  }
 private:
  void reset() {
    current_index = 0;
    current_location = innards.front().first;
  }
  std::deque<ends> innards;
  size_t current_index;
  char const * current_location;
};
