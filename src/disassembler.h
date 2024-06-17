#ifndef EIGHTY80_EMU_DISASSEMBLER_H_
#define EIGHTY80_EMU_DISASSEMBLER_H_

#include <array>
#include <format>
#include <iterator>
#include <iostream>

using byte = uint8_t;
using address = size_t;
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

template <typename InIt>
void decode(InIt ip, InIt eof, std::ostream& os)
{
    address memory_address = 0;
    while(ip != eof)
    {
        const byte b = *ip;
        os << std::format("{:#08x}\t", memory_address);

        if(b == 0x00) { os << "NOP"; }
        else if(is_mov(b)) { 
            os << std::format(
                "MOV {:s} {:s}", 
                reg_name[dest(b)], 
                reg_name[src(b)]);
        }
        else if(is_mvi(b))
        {
            os << std::format(
                "MVI {:s} {:#x}",
                reg_name[dest(b)],
                *++ip);
            memory_address += 8; // advance past data
        }
        else { os << "UNKNOWN"; }
    
        os << '\n';
        ++ip;
        memory_address += 8;
    }
}

#endif
