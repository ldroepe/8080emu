#include <vector>
#include <numeric>
#include <algorithm>
#include <gtest/gtest.h>

#include "../src/disassembler.h"

struct disassembled_code
{
    // NOTE -- these vectors implicitly share an index. addresses[i] corresponds
    // to the memory address that opcode[i] has been mapped to
    std::vector<address> addresses;
    std::vector<std::string> opcodes;
};

[[nodiscard]]
disassembled_code parse_disassembled_output(const std::string& code)
{
    disassembled_code dis;
    std::istringstream in_code(code);
    for(std::string line; std::getline(in_code, line);)
    {
        const auto hex_marker = line.find('x');
        const auto gap = line.find('\t', hex_marker);

        std::istringstream hex_value(line.substr(hex_marker+1, (gap-hex_marker)));
        uint64_t memory_location = 0;
        hex_value >> std::hex >> memory_location;
        
        dis.addresses.push_back(memory_location);
        dis.opcodes.push_back(line.substr(gap+1));
    }
    return dis;
}

TEST(DecodeTest, TestParseDissassembledOutput)
{

    std::vector<byte> input(100, 0x00);
    std::ostringstream oss;
    decode(input.cbegin(), input.cend(), oss);
    const auto decoded = oss.str();
    const disassembled_code nop_dis = parse_disassembled_output(decoded);

    EXPECT_GT(nop_dis.addresses.size(), 0);
    EXPECT_GT(nop_dis.opcodes.size(), 0);
}

TEST(DecodeTest, TestNOP) 
{
    std::vector<byte> input(100, 0x00);
    std::ostringstream oss;
    decode(input.cbegin(), input.cend(), oss);
    const auto decoded = oss.str();
    const disassembled_code nop_dis = parse_disassembled_output(decoded);
    
    // opcodes are correctly disassembled
    const auto is_nop = [](const std::string& opcode) {
        EXPECT_EQ(opcode, "NOP");
    };
    std::for_each(std::cbegin(nop_dis.opcodes), std::cend(nop_dis.opcodes), is_nop);
    
    const auto contiguous_memory = [](byte loc) { EXPECT_EQ(loc, 0x08); };
    std::vector<address> memory_diffs;
    memory_diffs.reserve(nop_dis.addresses.size());

    // memory addresses are adjacent
    std::adjacent_difference(
        std::cbegin(nop_dis.addresses), 
        std::cend(nop_dis.addresses), 
        std::back_inserter(memory_diffs));

    std::for_each(
        std::next(std::cbegin(memory_diffs)), // adjacent_difference includes 1st element
        std::cend(memory_diffs), 
        contiguous_memory);
}
