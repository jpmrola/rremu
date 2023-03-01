#include <base_device.h>

template <uint64_t base_addr_mem, uint64_t size_mem>
class CLINT : public BaseDevice
{
  public:

    CLINT() = default;

    void Load(uint64_t addr, int size, uint64_t& data) override {};
    void Store(uint64_t addr, int size, uint64_t data) override {};

    constexpr uint64_t get_base_addr() override { return base_addr; }
    constexpr uint64_t get_size() override { return size; }

  private:

    static constexpr uint64_t base_addr = base_addr_mem;
    static constexpr uint64_t size = size_mem;

};