#include "dna_string.h"

void dna_string::substr_from(location const & from, dna_string & to_copy_to)
{
  location seeker(from);
  while (seeker.first!=current_index_ && seeker.second!=current_location_) {
    to_copy_to.push_back(ends(seeker.second, innards[from.first].second));
    seeker = location(1+from.first, innards[1+from.first].first);
  }
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

location const dna_string::current_location() {
  return location(current_index_, current_location_);
} 