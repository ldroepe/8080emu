#include <vector>
#include <gtest/gtest.h>

#include "../src/disassembler.h"

TEST(DecodeTest, TestNOP) 
{
    std::vector<byte> input = {0x00, 0x00, 0x00};
    std::ostringstream oss;
    decode(input.cbegin(), input.cend(), oss);

    const auto decoded = oss.str();
    const auto end = std::cend(decoded);
    const char* const nop = "NOP";

    const auto find_nop = [&nop, nopl=nop+3](auto first, auto last) {
        return std::search(first, last, nop, nopl);
    };
    auto nop_pos = find_nop(std::cbegin(decoded), end);
    int nop_count = 0;
    while(nop_pos != end)
    {
        ++nop_count;
        nop_pos = find_nop(nop_pos+1, end);
    }

    EXPECT_EQ(nop_count, 3);
}
