#include <algorithm>
#include <ctime>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>

using std::placeholders::_1;
using std::shared_ptr;
using std::string;

namespace {
  size_t const chunk_length = 56;

  struct chunk {
    chunk(std::string const & str, size_t length_wanted, size_t from_index) {
      length_ = str.copy(content_, length_wanted, from_index);      
      if (length_ != length_wanted)
	throw std::runtime_error("wtf?");
    }
    chunk() : length_(0) {}
    char operator[](size_t index) {
      return content_[index];
    }
    size_t length_;
    char content_[chunk_length];
  };
  
  class chunked_string {
  private:
    typedef std::list<std::shared_ptr<chunk> > chunk_list;
  public:
    chunked_string(std::string const & str) : location_index_(0) {
      size_t const index_after_full_chunks = add_full_chunks(str);
      if (index_after_full_chunks != str.size()) {
	add_final_chunk(str, index_after_full_chunks);
      }
      location_ = chunks.begin();
      end_ = chunks.end();
      current_chunk_ = **location_;
    }

    std::string to_string() const {
      std::string result;
      result.reserve(total_length());

      auto functor = std::bind(&chunked_string::push_to_string, this, std::ref(result), _1);
      std::for_each(chunks.begin(), chunks.end(), functor);
      return result;
    }

    size_t chunk_count() const {
      return chunks.size();
    }

    bool hasNext() const {
      if (current_chunk_.length_ > location_index_)
	return true;
      load_next_chunk();
      return location_ != end_;
    }

    char next() const {
      // obvious danger here if not called in sync with hasNext
      return current_chunk_.content_[location_index_++];
    }

  private:
    void load_next_chunk() const {
      ++location_;
      location_index_ = 0;
      if (location_ != end_)
	current_chunk_ = **location_;
    }

    size_t total_length() const {
      size_t full_size(0);
      for (chunk_list::const_iterator it=chunks.begin(), end=chunks.end();
	   it != end; ++it) {
	full_size += (**it).length_;
      }
      return full_size;
    }

    void push_to_string(std::string & str, shared_ptr<chunk> chunk) const {
      str.append(chunk->content_, chunk->length_);
    }
		        
    size_t add_full_chunks(std::string const & str) {
      size_t current_index = 0;
      if (str.size() < chunk_length)
	return current_index;

      size_t const full_chunk_max = str.size() - chunk_length;
      while (current_index < full_chunk_max) {
	chunks.push_back(std::make_shared<chunk>(str, chunk_length, current_index));
	current_index += chunk_length;
      }
      return current_index;
    }

    void add_final_chunk(std::string const & str, size_t const index) {
      size_t const chars_left = str.size() - index;
      chunks.push_back(std::make_shared<chunk>(str, chars_left, index));
    }

    chunk_list chunks;
    chunk_list::const_iterator end_;
    mutable chunk_list::const_iterator location_;
    mutable chunk current_chunk_;
    mutable size_t location_index_;
  };

  void check_chunk_count(chunked_string const & chunky, size_t initial_string_length) {
    size_t const max_acceptable_chunk_count = 1 + (initial_string_length / chunk_length);
    size_t const actual_count = chunky.chunk_count();
    if (actual_count > max_acceptable_chunk_count)
      throw std::runtime_error("Chunk count: " + std::to_string(actual_count) +
			       " was greater than max acceptable count: " +
			       std::to_string(max_acceptable_chunk_count));
  }

  void round_trip_test(std::string const & str) {
    chunked_string const chunky(str);
    std::string const round_tripped(std::move(chunky.to_string()));
    if (round_tripped != str)
      throw std::runtime_error(round_tripped + " did not equal " + str);
    check_chunk_count(chunky, str.size());
  }

  void run_iteration_test() {
    char const bang('!');
    std::string long_string(500000000, bang);
    chunked_string const chunks(long_string);

    time_t start = time(NULL);
    while (chunks.hasNext()) {
      char const c(chunks.next());
      if (c != bang)
	throw std::runtime_error("unexpected char!");
    }
    time_t end = time(NULL);
    std::cout << "iteration took " << end - start << " seconds" << std::endl;
  }

  void run_small_iter_test() {
    std::string short_string("The quick brown fox jumped over the lazy dog gate, with his mate geoff, and then they went to the pub and drank copious amounts of booze\n");
    chunked_string const chunks(short_string);
    std::string buffer;
    while (chunks.hasNext()) {
      buffer.append(1, chunks.next());
    }
    if (buffer != short_string)
      throw std::runtime_error(buffer + " was not equal to " + short_string);
  }
}

void run_tests() {
  round_trip_test(std::string("geoff woz here!"));
  round_trip_test(std::string(chunk_length, '!'));
  round_trip_test(std::string(chunk_length + 1, '!'));
  round_trip_test(std::string(chunk_length * 100 + 4, '!')); 
  round_trip_test(std::string(128000, '!'));
  std::cout << "All tests passed" << std::endl;
}

int main(int argc, char * argv[]) {
  std::cout << sizeof(std::string) << std::endl;
  std::cout << sizeof(chunk) << std::endl;
  std::cout << sizeof(std::list<chunk>) << std::endl;
  std::cout << sizeof(chunked_string) << std::endl;

  run_tests();
  run_small_iter_test();
  run_iteration_test();
}
