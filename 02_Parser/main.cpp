// Project includes
#include "scan.hpp"
#include "parser.hpp"
// Standard includes
// C++ Standard
#include <fstream>
#include <iostream>

// C Standard

void usage(const char *program_name) {
    std::cerr << "Usage: " << program_name << " <input_file>" << std::endl;
}

int main(int argc, char *argv[]) {
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
    scanner->Scan();
    auto parser = std::make_unique<my_cpp::Parser>(std::move(scanner));
    auto ast = parser->Parse();
    std::cout << my_cpp::utility::Evaluate(*ast) << std::endl;
    return 0;
}