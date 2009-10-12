#include "dna_string.h"
#include <algorithm>
#include <functional>
#include <boost/bind.hpp>

using namespace std;

char const I = 'I';
char const C = 'C';
char const F = 'F';
char const P = 'P';
ends const eI(&I, &I);
ends const eC(&C, &C);
ends const eF(&F, &F);
ends const eP(&P, &P);

void dna_string::prepend(dna_string const & to_prepend)
{
  std::deque<ends>::const_iterator meh = to_prepend.end();
  while (meh!=to_prepend.begin()) {
    push_front(*meh);
    ++meh;
  }
}

void dna_string::push_ends_back(ends const & to_push)
{
  push_back(to_push);
}

void dna_string::push_back(char const & to_push)
{
  switch (to_push) {
  case 'I':
    push_back(eI);
    break;
  case 'C':
    push_back(eC);
    break;
  case 'F':
    push_back(eF);
    break;
  case 'P':
    push_back(eP);
    break;
  }
}

void dna_string::substr_from(location const & from, dna_string & to_copy_to)
{
  location seeker(from);
  while (seeker.first!=current_index_ && seeker.second!=current_location_) {
    to_copy_to.push_back(ends(seeker.second, innards[from.first].second));
    seeker = location(1+from.first, innards[1+from.first].first);
  }
}

unsigned int const dna_string::length() const {
  unsigned int to_return(0);
  for (size_t s(0); s < innards.size(); ++s) {
    to_return += (innards[s].second - innards[s].first);
  }
  return to_return + innards.size();
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
