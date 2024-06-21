#include <fstream>
#include <filesystem>

#include "disassembler.h"

int main(int argc, char* argv[])
{
    const fs::path p("/home/ldroepe/projects/8080emu/rom/invaders.h");
    std::ifstream inFile(p, std::ios::in);
    inFile >> std::hex;
    std::istream_iterator<byte> in(inFile);
    const std::istream_iterator<byte> last;
    decode(in, last, std::cout);
}
