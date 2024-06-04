
#include <array>
#include <format>
#include <iostream>
#include <iterator>
#include <filesystem>

using byte = uint8_t;
using reg_id_t = byte;
namespace fs = std::filesystem;

enum reg: reg_id_t
{
    B,          //0b000
    C,          //0b001
    D,          //0b010
    E,          //0b011
    H,          //0b100
    L,          //0b101
    Memory,     //0b110
    A           //0b111
};
constexpr std::array<const char*, 8> reg_name = 
    {"B", "C", "D", "E", "H", "L", "Memory", "A"};

// 0x07 = 0b00000111
constexpr reg dest(byte b) { return static_cast<reg>((b >> 3) & 0x07); }
constexpr reg src(byte b) { return static_cast<reg>(b & 0x07); }

// MOV r1, r2 --> 0b01DDDSSS
constexpr bool is_mov(byte b) { return (b >> 6) == 1; }
// MVI r --> 0b00DDD110
constexpr bool is_mvi(byte b) { return ((b >> 6 == 0) && src(b) == reg::Memory); }

template <typename InIt, typename OutIt>
void decode(InIt ip, InIt eof, OutIt out)
{
    size_t memory_address = 0;
    while(ip != eof)
    {
        const byte b = *ip;
        std::format_to(out, "{:#08x}", memory_address);

        if(b == 0x00) { *out = "NOP\n"; }
        else if(is_mov(b)) { 
            std::format_to(out, 
                "MOV {:s} {:s}", 
                reg_name[dest(b)], 
                reg_name[src(b)]);
        }
        else if(is_mvi(b))
        {
            std::format_to(out,
                "MVI {:s} {:#x}",
                reg_name[dest(b)],
                *++ip);
        }
    
        *out = '\n';
        ++out;
        ++ip;
        memory_address += 8;
    }
}

int main(int argc, char* argv[])
{
    const fs::path p("/home/ldroepe/emu/space_invaders/rom/invaders.h");
    std::ifstream inFile(p, "r");
    inFile >> std::hex;
    std::istream_iterator<byte> in(inFile);
    const std::istream_iterator<byte> last = std::next(in, 5);
    std::ostream_iterator<const char*> out(std::cout);
    decode(in, last, out);
}
