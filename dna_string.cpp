#include "dna_string.h"

#include <algorithm>
#include <functional>
#include <boost/bind.hpp>
#include <ostream>
#include <sstream>

ends::ends(char const * s, char const * e, int protection_level)
  : first(s), second(e), protection_level_(protection_level) {}

dna_string::dna_string()
  : current_index_(0)
{}

dna_string::dna_string(std::deque<ends> initial_innards) 
: innards(initial_innards)
{
  reset();
}

namespace {
  void quote_then_display(char c, int level, std::ostream & to_display_to) {
    if (level==0) {
      to_display_to << c;
      return;
    }
    switch (c) {
    case 'I':
      quote_then_display('C', level-1, to_display_to);
      break;
    case 'C':
      quote_then_display('F', level-1, to_display_to);
      break;
    case 'F':
      quote_then_display('P', level-1, to_display_to);
      break;
    case 'P':
      quote_then_display('I', level-1, to_display_to);
      quote_then_display('C', level-1, to_display_to);
      break;
    }
  }
}

void ends::display(std::ostream & out) const {
  char * cit = const_cast<char *>(first);
  while (cit!=second) {
    quote_then_display(*cit, protection_level_, out);
    ++cit;
  }
  quote_then_display(*cit, protection_level_, out);
}

//FIXME for protection 0 only
std::string ends::to_s() const {
  assert(protection_level_ == 0);
  char * cit = const_cast<char *>(first);
  std::stringstream ss;
  while (cit!=second) {
    ss << *cit;
    ++cit;
  }
  ss << *cit;
  return ss.str();
}


void dna_string::display(std::ostream & out) {
  ends piece(current_location_, innards[current_index_].second, innards[current_index_].protection_level());
  piece.display(out);
  for (size_t i(current_index_+1); i< innards.size(); ++i) {
    innards[i].display(out);
  }
  out << '\n';
}

unsigned int const dna_string::innards_size() const {
  return innards.size();
}

void dna_string::prepend(dna_string const & to_prepend)
{
  for (size_t s(0); s<current_index_; ++s)
    innards.pop_front();
  if (at_end_)
    innards.pop_front();
  else
    innards.front().first = current_location_;
  std::deque<ends>::const_reverse_iterator meh = to_prepend.rbegin();
  while (meh!=to_prepend.rend()) {
    push_front(*meh);
    ++meh;
  }
  reset();
}

void dna_string::push_ends_back(ends const & to_push) {
  this->push_back(to_push);
}

void dna_string::push_back(char const & to_push)
{
  char const * icfp = &to_push; 
  push_ends_back(ends(icfp,icfp, 0));
}

void dna_string::protect(int level) {
  unsigned int index_traverser = current_index_;
  while (index_traverser < innards.size()) {
    innards[index_traverser].protect(level);
    ++index_traverser;
  }
}

void dna_string::substr_from(location const & from, dna_string & to_copy_to)
{
  location seeker(from);
  while (seeker.first<current_index_) {
    to_copy_to.push_back(ends(seeker.second, innards[seeker.first].second, innards[seeker.first].protection_level()));
    size_t next_index(seeker.first + 1);
    seeker = location(next_index, innards[next_index].first);
  }
  if (seeker.second==current_location_)
    return;
  to_copy_to.push_back(ends(seeker.second, current_location_ - 1, innards[current_index_].protection_level()));
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

char const & dna_string::get() const {
  return *current_location_;
}

void dna_string::skip_to_first(const char * needle, size_t const length_of_needle)
{
  location start_position;
  save_position(start_position);
  bool matching(false);
  size_t substring_index(0);
  location potential_match;
  while (has_next()) {
    char const needle_piece=needle[substring_index];
    bool const match_found(get() == needle_piece);
    if (match_found) {
      if (!matching) {
	save_position(potential_match);
	matching = true;
      }
      ++substring_index;
      //if (substring_index > 4)
      //std::cerr << substring_index << '\n';
      if (substring_index == length_of_needle) {
	++(*this); //we want the first position *after* the match
	return;
      }
      //we're done!
    }
    else if (matching) {
	load_position(potential_match);
	matching = !matching;
	substring_index = 0;
    }
    ++(*this);
  }
  // No substring found, reset to initial position
  load_position(start_position);
}

location const dna_string::current_location() const {
  return location(current_index_, current_location_);
} 

void dna_string::append(dna_string const & to_append) 
{
  for_each(to_append.begin(), to_append.end(), boost::bind(&dna_string::push_ends_back, this, _1));
  if (at_end_) {
    //FIXME action required?
  }
}

unsigned int dna_string::move(unsigned int desired_distance) {
  if (innards[current_index_].second >= current_location_ + desired_distance) {
    current_location_+=desired_distance;
    return desired_distance;
  } else {
    unsigned int will_move = innards[current_index_].second - current_location_ + 1;
    current_location_ = innards[++current_index_].first;
    return will_move;
  }
}

void dna_string::operator+=(unsigned int to_add) {
  //FIXME what if to_add is > remaining length?
  if (to_add < remaining_length()) {
    while (to_add > 0)
      to_add -= move(to_add);
  }
  else if (to_add > remaining_length()) {
    //throw_something
  }
  else {
    //we're at the end.
    at_end_ = true;
  }
}
