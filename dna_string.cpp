#include "dna_string.h"
#include <algorithm>
#include <functional>
#include <boost/bind.hpp>


ends::ends(char const * s, char const * e)
: first(s), second(e) {
  if (second < first)
    std::cerr << "ARGH" << '\n';
}


dna_string::dna_string()
  : current_index_(0)
{
  
}

dna_string::dna_string(std::deque<ends> initial_innards) 
: innards(initial_innards)
{
  reset();
}

unsigned int const dna_string::innards_size() const {
  return innards.size();
}

void dna_string::prepend(dna_string const & to_prepend)
{
  for (size_t s(0); s<current_index_; ++s)
    innards.pop_front();
  innards.front().first = current_location_;
  std::deque<ends>::const_reverse_iterator meh = to_prepend.rbegin();
  while (meh!=to_prepend.rend()) {
    push_front(*meh);
    ++meh;
  }
  reset();
}

void dna_string::push_ends_back(ends const & to_push) {
  if (to_push.second < to_push.first)
    std::cerr << "ARARGHAGRHARGH" << '\n';
  this->push_back(to_push);
}

void dna_string::push_back(char const & to_push)
{
  char const * icfp = &to_push; 
  push_ends_back(ends(icfp,icfp));
}

void dna_string::substr_from(location const & from, dna_string & to_copy_to)
{
  location seeker(from);
  while (seeker.first<current_index_) {
    to_copy_to.push_back(ends(seeker.second, innards[seeker.first].second));
    size_t next_index(seeker.first + 1);
    seeker = location(next_index, innards[next_index].first);
  }
  if (seeker.second==current_location_)
    return;
  to_copy_to.push_back(ends(seeker.second, current_location_ - 1));
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
  while (to_add > 0)
    to_add -= move(to_add);
}
