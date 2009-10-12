#include "dna_string.h"
#include <algorithm>
#include <functional>
#include <boost/bind.hpp>

void dna_string::prepend(dna_string const & to_prepend)
{
  for (size_t s(0); s<current_index_; ++s)
    innards.pop_front();
  innards.front().first = current_location_;
  std::deque<ends>::const_iterator meh = to_prepend.end();
  while (meh!=to_prepend.begin()) {
    push_front(*meh);
    --meh;
  }
  reset();
}

void dna_string::push_ends_back(ends const & to_push) {
  this->push_back(to_push);
}

void dna_string::push_back(char const to_push)
{
  push_back(ends(&to_push, &to_push));
}

void dna_string::substr_from(location const & from, dna_string & to_copy_to)
{
  location seeker(from);
  while (seeker.first<current_index_) {
    to_copy_to.push_back(ends(seeker.second, innards[from.first].second));
    size_t next_index(seeker.first + 1);
    seeker = location(next_index, innards[next_index].first);
  }
  to_copy_to.push_back(ends(innards[current_index_].first, current_location_));
}

unsigned int const dna_string::remaining_length() const {
  //find length from current_position_ to end 
  unsigned int to_return(innards[current_index_].second - current_location_);
  ++to_return;
  size_t current_index_copy(current_index_ + 1);
  while (current_index_copy < innards.size()) {
    to_return += (innards[current_index_copy].second - innards[current_index_copy].first);
    ++to_return;
    ++current_index_copy;
  }
  return to_return;
}

void dna_string::skip_to_first(const char * needle, size_t const length_of_needle)
{
  location start_position;
  save_position(start_position);
  bool found(false),matching(false);
  size_t substring_index(0);
  location potential_match;
  while (!found) {
    char const needle_piece=needle[substring_index];
    bool const match_found(get() == needle_piece);
    if (match_found) {
      if (!matching) {
	potential_match.first = current_index_;
	potential_match.second = current_location_;
	matching = true;
      }
      ++substring_index;
      if (substring_index == length_of_needle) {
	return;
      }
      //we're done!
    }
    else if (matching) {
	load_position(potential_match);
	matching = !matching;
    }
    ++(*this);
  }
}

location const dna_string::current_location() const {
  return location(current_index_, current_location_);
} 

void dna_string::append(dna_string const & to_append) 
{
  for_each(to_append.begin(), to_append.end(), boost::bind(&dna_string::push_ends_back, this, _1));
}
