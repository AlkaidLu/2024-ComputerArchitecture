#include "Multicycle.h"
/*
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <program_file>" << std::endl;
        return 1;
    }
    MultiCycleCPU cpuSim(1024);

    cpuSim.load_memory(argv[1]); // Load the program into memory
    cpuSim.save_memory("instruction_data.txt");
    //printf("1\n");
    cpuSim.run(); // Start executing instructions
    //printf("1\n");
    cpuSim.save_memory("result.txt");
    return 0;
}
*/
int main() {

    MultiCycleCPU cpuSim(1024);

    cpuSim.load_memory("code.txt"); // Load the program into memory
    cpuSim.save_memory("instruction_data.txt");
    cpuSim.run(); // Start executing instructions
    cpuSim.save_memory("result.txt");
    return 0;
}