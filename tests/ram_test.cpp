#include <gtest/gtest.h>
#include <ram.h>

TEST(RAMTest, StoreAndLoad)
{
    auto ram = std::make_unique<RAM>(0, 0x1000);
    uint64_t data;
    // Test 1 byte
    ram->Store(0, 1, 0xAA);
    ram->Load(0, 1, data);
    EXPECT_EQ(data, 0xAA);
    // Test 2 bytes
    ram->Store(0, 2, 0xBBCC);
    ram->Load(0, 2, data);
    EXPECT_EQ(data, 0xBBCC);
    // Test 4 bytes
    ram->Store(0, 4, 0xDDCCBBAA);
    ram->Load(0, 4, data);
    EXPECT_EQ(data, 0xDDCCBBAA);
    // Test 8 bytes
    ram->Store(0, 8, 0x8877665544332211);
    ram->Load(0, 8, data);
    EXPECT_EQ(data, 0x8877665544332211);
}

TEST(RAMTest, CheckIfZeroInitialized)
{
    auto ram = std::make_unique<RAM>(0, 0x1000);
    uint64_t data;
    for (int i = 0; i < 0x1000; i++)
    {
        ram->Load(i, 1, data);
        EXPECT_EQ(data, 0);
    }
}

TEST(RAMTest, CheckException)
{
    auto ram = std::make_unique<RAM>(0, 0x1000);
    uint64_t data;
    EXPECT_THROW(ram->Load(0x1000, 5, data), std::runtime_error);
    EXPECT_THROW(ram->Store(0x1000, 3, 0xAA), std::runtime_error);
}