#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <gtest/gtest.h>

#include "../src/disassembler.h"

using instruction_set = std::vector<byte>;

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

[[nodiscard]]
disassembled_code disassemble_instruction_set(const instruction_set& is)
{
    std::ostringstream oss;
    decode(std::cbegin(is), std::cend(is), oss);
    const auto decoded = oss.str();
    return parse_disassembled_output(decoded);
}

TEST(InfraTest, parse_disassembled_output)
{

    const instruction_set input(100, 0x00);
    const disassembled_code nop_dis = disassemble_instruction_set(input);

    EXPECT_EQ(nop_dis.addresses.size(), 100);
    EXPECT_EQ(nop_dis.opcodes.size(), 100);
}

TEST(MemoryTest, single_byte_instructions_are_adjacent)
{
    // opcodes which perform their full function from a single byte in memory
    const std::vector<opcode> single_byte_opcodes = {
        opcode::NOP, opcode::STAX_B, opcode::STAX_D, opcode::LDAX_B,
        opcode::LDAX_D
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> which_opcode(0, single_byte_opcodes.size()-1);

    const auto choose_opcode = [&]() { return single_byte_opcodes[which_opcode(gen)]; };

    constexpr int n = 100;
    instruction_set input;
    input.reserve(n);
    std::generate_n(std::back_inserter(input), n, choose_opcode);

    const disassembled_code single_byte_dis = disassemble_instruction_set(input);

    const auto contiguous_memory = [](byte loc) { EXPECT_EQ(loc, 0x08); };
    std::vector<address> memory_diffs;
    memory_diffs.reserve(single_byte_dis.addresses.size());

    std::adjacent_difference(
        std::cbegin(single_byte_dis.addresses), 
        std::cend(single_byte_dis.addresses), 
        std::back_inserter(memory_diffs));
    std::for_each(
        std::next(std::cbegin(memory_diffs)), // adjacent_difference includes 1st element
        std::cend(memory_diffs), 
        contiguous_memory);
}

TEST(NOPTest, many_nops) 
{
    const instruction_set input(100, 0x00);
    const disassembled_code nop_dis = disassemble_instruction_set(input);
    
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

TEST(STATest, simple_direct_acc_store_decode)
{
    const instruction_set sta_input = {opcode::STA, 0xef, 0xbe};
    const disassembled_code sta_dis = disassemble_instruction_set(sta_input);

    EXPECT_EQ(sta_dis.addresses.size(), 1) << "Single instruction --> single address";
    ASSERT_EQ(sta_dis.opcodes.size(), 1) << "STA instruction (0x32) didn't yield 1 opcode";

    const std::string sta_opcode = sta_dis.opcodes[0];
    EXPECT_TRUE(sta_opcode.starts_with("STA")) << "0x32 (0b00110010) not decoded into STA";

    EXPECT_TRUE(sta_opcode.ends_with("beef")) << "{0x32, 0xbe, 0xef} --> " << sta_opcode;
}

TEST(STATest, simple_direct_acc_store_addresses)
{
    const instruction_set sta_input = {opcode::NOP, opcode::STA, 0xef, 0xbe, opcode::NOP};
    const disassembled_code sta_dis = disassemble_instruction_set(sta_input);

    EXPECT_EQ(sta_dis.addresses.size(), 3) << "STA Input didn't yield 3 addresses";
    ASSERT_EQ(sta_dis.opcodes.size(), 3) << "STA Input didn't yield 3 opcodes";

    EXPECT_EQ(sta_dis.addresses[1] - sta_dis.addresses[0], 8)
        << "NOP & STA should be adjacent";
    EXPECT_EQ(sta_dis.addresses[2] - sta_dis.addresses[1], 24)
        << "STA should take up 3 bytes of memory";

    EXPECT_TRUE(sta_dis.opcodes[1].starts_with("STA"));
    EXPECT_TRUE(sta_dis.opcodes[1].ends_with("beef"));
}

TEST(STATest, data_and_opcodes_are_separate)
// Ensure that the address portion of the STA opcode is not interpreted as an
// opcode (NOP in this case)
{
    const instruction_set sta_input = {opcode::NOP, opcode::STA, 0x00, 0x00, opcode::NOP};
    const disassembled_code sta_dis = disassemble_instruction_set(sta_input);

    EXPECT_EQ(sta_dis.addresses.size(), 3) << "STA input didn't yield 3 addresses";
    ASSERT_EQ(sta_dis.opcodes.size(), 3) << "STA input didn't yield 3 opcodes";

    EXPECT_EQ(sta_dis.opcodes[0], "NOP");
    EXPECT_EQ(sta_dis.opcodes[2], "NOP");

    const auto& sta = sta_dis.opcodes[1];
    EXPECT_TRUE(sta.starts_with("STA"));
    EXPECT_TRUE(sta.ends_with("0000"));
}

TEST(STAXBTest, simple_stax_b_decode)
{
    const instruction_set stax_input = {opcode::STAX_B};
    const disassembled_code stax_dis = disassemble_instruction_set(stax_input);

    EXPECT_EQ(stax_dis.addresses.size(), 1) << "STAX B input didn't yield 1 address";
    ASSERT_EQ(stax_dis.opcodes.size(), 1) << "STAX B input didn't yield 1 opcode";

    EXPECT_EQ(stax_dis.opcodes[0], "STAX B");
}

TEST(STAXDTest, simple_stax_d_decode)
{
    const instruction_set stax_input = {opcode::STAX_D};
    const disassembled_code stax_dis = disassemble_instruction_set(stax_input);

    EXPECT_EQ(stax_dis.addresses.size(), 1) << "STAX D input didn't yield 1 address";
    ASSERT_EQ(stax_dis.opcodes.size(), 1) << "STAX D input didn't yield 1 opcode";

    EXPECT_EQ(stax_dis.opcodes[0], "STAX D");
}

TEST(LDAXBTest, simple_ldax_b_decode)
{
    const instruction_set ldax_input = {opcode::LDAX_B};
    const disassembled_code ldax_dis = disassemble_instruction_set(ldax_input);

    EXPECT_EQ(ldax_dis.addresses.size(), 1) << "LDAX B input didn't yield 1 address";
    ASSERT_EQ(ldax_dis.opcodes.size(), 1) << "LDAX B input didn't yield 1 opcode";

    EXPECT_EQ(ldax_dis.opcodes[0], "LDAX B");
}

TEST(LDAXDTest, simple_ldax_d_decode)
{
    const instruction_set ldax_input = {opcode::LDAX_D};
    const disassembled_code ldax_dis = disassemble_instruction_set(ldax_input);

    EXPECT_EQ(ldax_dis.addresses.size(), 1) << "LDAX D input didn't yield 1 address";
    ASSERT_EQ(ldax_dis.opcodes.size(), 1) << "LDAX D input didn't yield 1 opcode";

    EXPECT_EQ(ldax_dis.opcodes[0], "LDAX D");
}
