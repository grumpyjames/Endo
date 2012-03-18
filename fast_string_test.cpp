#include <iostream>

#include <string>


using std::string;

int main(int argc, char * argv[]) {
  std::string long_string(500000000, '!');
  time_t start = time(NULL);
  for (size_t s = 0; s < 1; ++s) {
    for (char c: long_string) {
      if (c != '!')
	std::cout << "WTF?";
    }
  }
  time_t end = time(NULL);
  std::cout << "iteration took " << end - start << " seconds" << std::endl;
}
