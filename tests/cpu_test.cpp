#include "gtest/gtest.h"
#include "cpu.h"
#include "mmu.h"
#include "ram.h"
#include "virtio.h"
#include "uart.h"
#include "clint.h"
#include "plic.h"
#include "config.h"

class CPUTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mmu = std::make_unique<MMU>();
        mmu->add_device(std::make_unique<RAM>(0, MEMORY_SIZE));
        mmu->add_device(std::make_unique<UART>(UART_BASE, UART_SIZE));
        mmu->add_device(std::make_unique<VIRTIO>(VIRTIO_BASE, VIRTIO_SIZE));
        mmu->add_device(std::make_unique<CLINT>(CLINT_BASE, CLINT_SIZE));
        mmu->add_device(std::make_unique<PLIC>(PLIC_BASE, PLIC_SIZE));
        cpu = std::make_unique<CPU>(std::move(mmu));
    }

    std::unique_ptr<MMU> mmu;
    std::unique_ptr<CPU> cpu;
};

TEST_F(CPUTest, InstructionTestADDI)
{
  cpu->RunInstruction(0x00400093); // addi x1, x0, 0x004
  cpu->RunInstruction(0x00408093); // addi x1, x1, 0x004
}

TEST_F(CPUTest, InstructionTestStore)
{
  cpu->RunInstruction(0x00400093); // addi x1, x0, 0x004
  cpu->RunInstruction(0x00408093); // addi x1, x1, 0x004
  // Store
  cpu->RunInstruction(0x10102023); // sw x1, 0x100
  // Load
  cpu->RunInstruction(0x10002103); // lw x2, 0x100

}