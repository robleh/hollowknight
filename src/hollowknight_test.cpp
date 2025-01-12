#include "hollowknight.hpp"
#include <al/gtest/pic.hpp>

struct HollowKnightTest : public PICTest<HollowKnightTest> {
    inline static std::string_view path = HOLLOWKNIGHT_PIC_PATH;
    inline static unsigned long permissions = PAGE_EXECUTE_READWRITE;
    hollowknight_t m_pic = nullptr;
};

TEST_F(HollowKnightTest, PositionIndependent) {
    EXPECT_EQ(ERROR_SUCCESS, m_pic());
}