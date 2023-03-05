#ifndef CLINT_H
#define CLINT_H

#include <base_device.h>

template <uint64_t base_addr_mem, uint64_t size_mem>
class CLINT : public BaseDevice
{
  public:

    CLINT() = default;

    void Load(uint64_t addr, int size, uint64_t& data) override {};
    void Store(uint64_t addr, int size, uint64_t data) override {};

    constexpr uint64_t GetBaseAddr() override { return base_addr; }
    constexpr uint64_t GetSize() override { return size; }
    constexpr bool IsValidAddr(uint64_t addr) override { return addr >= base_addr && addr < base_addr + size; }

  private:

    static constexpr uint64_t base_addr = base_addr_mem;
    static constexpr uint64_t size = size_mem;

};

#endif
