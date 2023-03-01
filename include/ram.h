#include <cstdint>
#include <array>
#include <memory>
#include <iostream>
#include <cstring>
#include "base_device.h"

template <uint64_t base_addr_mem, uint64_t size_mem>
class RAM : public BaseDevice
{
  public:

    RAM(const std::shared_ptr<std::vector<uint8_t>>& binary) : mem()
    {
      std::copy(binary->begin(), binary->end(), mem.begin());
    }

    void Load(uint64_t addr, int size, uint64_t& data) override;
    void Store(uint64_t addr, int size, uint64_t data) override;

    constexpr uint64_t get_base_addr() override { return base_addr; }
    constexpr uint64_t get_size() override { return size; }

    private:

    std::array<uint8_t, size_mem> mem;
    static constexpr uint64_t base_addr = base_addr_mem;
    static constexpr uint64_t size = size_mem;

};

template<uint64_t base_addr_mem, uint64_t size_mem>
void RAM<base_addr_mem, size_mem>::Load(uint64_t addr, int size, uint64_t& data)
{
  auto index = addr - base_addr;
  switch(size)
  {
    case 1:
    case 2:
    case 4:
    case 8:
    {
      data = 0;
      std::memcpy(&data, &mem[index], size);
      break;
    }
    default:
    {
      throw std::runtime_error("Invalid size");
      break;
    }
  }
}

template<uint64_t base_addr_mem, uint64_t size_mem>
void RAM<base_addr_mem, size_mem>::Store(uint64_t addr, int size, uint64_t data)
{
  auto index = addr - base_addr;
  switch(size)
  {
    case 1:
    case 2:
    case 4:
    case 8:
    {
      std::memcpy(&mem[index], &data, size);
      break;
    }
    default:
    {
      throw std::runtime_error("Invalid size");
      break;
    }
  }
}
