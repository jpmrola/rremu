#include "gtest/gtest.h"
#include "mmu.h"
#include "ram.h"
#include "virtio.h"
#include "uart.h"
#include "clint.h"
#include "plic.h"
#include "config.h"

class MMUTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mmu = std::make_unique<MMU>();
        mmu->add_device(std::make_unique<RAM>(KERNBASE, MEMORY_SIZE));
        mmu->add_device(std::make_unique<UART>(UART_BASE, UART_SIZE));
        mmu->add_device(std::make_unique<VIRTIO>(VIRTIO_BASE, VIRTIO_SIZE));
        mmu->add_device(std::make_unique<CLINT>(CLINT_BASE, CLINT_SIZE));
        mmu->add_device(std::make_unique<PLIC>(PLIC_BASE, PLIC_SIZE));
    }

    std::unique_ptr<MMU> mmu;
};

TEST_F(MMUTest, CheckException)
{
    uint64_t data;
    // invalid address
    EXPECT_THROW(mmu->Store(0x9000000000000, 8, 0xAA), std::runtime_error);
    EXPECT_THROW(mmu->Load(0x9000000000000, 8, data), std::runtime_error);
    // invalid size
    EXPECT_THROW(mmu->Store(KERNBASE + 1, 3, 0xAA), std::runtime_error);
    EXPECT_THROW(mmu->Load(KERNBASE + 1, 5, data), std::runtime_error);
}