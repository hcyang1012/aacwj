// Project includes
#include "scan.hpp"

// Standard includes
// C++ Standard
#include <fstream>
#include <iostream>

// C Standard

void usage(const char* program_name) {
  std::cerr << "Usage: " << program_name << " <input_file>" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    usage(argv[0]);
    return 1;
  }
  std::ifstream input(argv[1]);
  if (!input) {
    std::cerr << "Failed to open input file" << std::endl;
    return 1;
  }
  auto scanner = my_cpp::utility::MakeScanner(input);
  my_cpp::utility::ScanFile(argv[1]);
  return 0;
}