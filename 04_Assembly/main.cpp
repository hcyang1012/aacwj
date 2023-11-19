// Project includes
#include "parser.hpp"
#include "gen_x86.hpp"
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

    std::ofstream output("out.s");
    if (!output) {
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
    }
    my_cpp::CodeGeneratorX86 codegen(output);
    codegen.GenerateCode(*ast);
    return 0;
}