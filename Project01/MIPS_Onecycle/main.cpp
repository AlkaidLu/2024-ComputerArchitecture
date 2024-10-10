#include <iostream>
#include "MIPSSimulator.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <program_file>" << std::endl;
        return 1;
    }
    MIPSSimulator simulator(1024); // Initialize simulator with 1024 bytes of memory
    simulator.load_memory(argv[1]); // Load the program into memory
    simulator.save_memory("instruction_data.txt");
    simulator.run(); // Start executing instructions
    simulator.save_memory("result.txt");
    return 0;
}