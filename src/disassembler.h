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

enum opcode : byte
{
    NOP    = 0x00,  //0b00000000
    STAX_B = 0x02,  //0b00000010
    STAX_D = 0x12,  //0b00010010
    LDAX_B = 0x0a,  //0b00001010
    LDAX_D = 0x1a,  //0b00011010
    STA    = 0x32,  //0b00110010
    LDA    = 0x3a,  //0b00111010
};

// 0x07 = 0b00000111
constexpr reg dest(byte b) { return static_cast<reg>((b >> 3) & 0x07); }
constexpr reg src(byte b) { return static_cast<reg>(b & 0x07); }

// MOV r1, r2 --> 0b01DDDSSS
constexpr bool is_mov(byte b) { return (b >> 6) == 1; }
// MVI r --> 0b00DDD110
constexpr bool is_mvi(byte b) { return ((b >> 6 == 0) && src(b) == reg::Memory); }

template <typename InIt>
constexpr uint16_t direct_value(InIt& ip)
// Return the value used in a "direct" opcode, e.g. LDA, STA
// Where the following 2 bytes are a little endian memory address / value
{
    const byte low_byte = *ip++;
    const byte high_byte = *ip;
    const uint16_t value = ((high_byte << 8) | low_byte);
    return value;
}

template <typename InIt>
void decode(InIt ip, InIt eof, std::ostream& os)
{
    address memory_address = 0;
    while(ip != eof)
    {
        const byte b = *ip;
        os << std::format("{:#08x}\t", memory_address);

        switch(b)
        {
            case opcode::NOP:
                os << "NOP";
                break;
            case opcode::STAX_B: // Store A indirect
            {
                os << "STAX B";
                break;
            }
            case opcode::STAX_D: // Store A indirect
            {
                os << "STAX D";
                break;
            }
            case opcode::LDAX_B: // Load A indirect
            {
                os << "LDAX B";
                break;
            }
            case opcode::LDAX_D: // Load A indirect
            {
                os << "LDAX D";
                break;
            }
            case opcode::STA: // Store A direct
            {
                os << "STA ";
                ++ip; // advance past STA
                const uint16_t memory_location = direct_value(ip);
                os << std::format("{:#06x}", memory_location);
                memory_address += 16; // high & low byte
                break;
            }
            case opcode::LDA: // Load A direct
            {
                os << "LDA ";
                ++ip; // advance past LDA
                const uint16_t value = direct_value(ip);
                os << std::format("{:#06x}", value);
                memory_address += 16; // high & low byte
                break;
            }
            default:
            {
                os << "UNKNOWN";
                break;
            }
        }

        os << '\n';
        ++ip;
        memory_address += 8;
    }
}

#endif
