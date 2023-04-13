#ifndef UART_H
#define UART_H

#include <thread>
#include "base_device.h"
#include "config.h"

enum UART_REG : uint64_t
{
  rhr = UART_BASE + 0x0,
  thr = UART_BASE + 0x0,
  ier = UART_BASE + 0x1,
  fcr = UART_BASE + 0x2,
  isr = UART_BASE + 0x2,
  lcr = UART_BASE + 0x3,
  lsr = UART_BASE + 0x5
};

enum UART_BITS : uint8_t
{
  ier_rx_enable   = 1 << 0,
  ier_tx_enable   = 1 << 1,
  fcr_fifo_enable = 1 << 0,
  fcr_fifo_clear  = 3 << 1,
  lcr_eight_bits  = 3 << 0,
  lcr_baud_latch  = 1 << 7,
  lsr_rx_ready    = 1 << 0,
  lsr_tx_idle     = 1 << 5
};

template <uint64_t base_addr_mem, uint64_t size_mem>
class UART : public BaseDevice
{
  public:

    UART() = default;

    void Load(uint64_t addr, int size, uint64_t& data) override;
    void Store(uint64_t addr, int size, uint64_t data) override;

    constexpr uint64_t GetBaseAddr() override { return base_addr; }
    constexpr uint64_t GetSize() override { return size; }
    constexpr bool IsValidAddr(uint64_t addr) override { return addr >= base_addr && addr < base_addr + size; }

  private:

    static constexpr uint64_t base_addr = base_addr_mem;
    static constexpr uint64_t size = size_mem;

    std::array<char, 128> buffer;
    std::thread tx_thread;
    std::thread rx_thread;
    // need to synchronize the threads so they can
    // access the UART data

};

template <uint64_t base_addr_mem, uint64_t size_mem>
void UART<base_addr_mem, size_mem>::Load(uint64_t addr, int size, uint64_t& data)
{
  if(size != 1)
  {
    throw CPUTrapException(trap_value::LoadAccessFault);
  }

  if(addr == rhr)
  {
    buffer[lsr - UART_BASE] &= ~lsr_rx_ready;
    // data is in the holding register, needs to be read
    // transfer control to rx_thread
  }
  auto index = base_addr - addr;
  data = 0;
  std::memcpy(&buffer[index], &data, size);
  return;
};

template <uint64_t base_addr_mem, uint64_t size_mem>
void UART<base_addr_mem, size_mem>::Store(uint64_t addr, int size, uint64_t data)
{
  if(size != 1)
  {
    throw CPUTrapException(trap_value::StoreAMOAccessFault);
  }

  if(addr == thr)
  {
    // implement send logic before thr empty bit
    // transfer control to tx_thread
    buffer[lsr - UART_BASE] |= lsr_tx_idle;
  }

}

#endif
