#ifndef MMU_H
#define MMU_H

#include <cstdint>
#include <vector>
#include <array>
#include <memory>
#include "base_mmu.h"
#include "config.h"
#include "ram.h"
#include "uart.h"
#include "virtio.h"
#include "clint.h"
#include "plic.h"

// Acts as the MMU and bus for the CPU
// Connects the devices and maps them in virtual memory
// Devices are represented as BaseDevice objects

template<typename BaseDevice>
concept Device = requires(BaseDevice dev, uint64_t addr, int size, uint64_t data)
{
  { dev.Load(addr, size, data) } -> std::same_as<void>;
  { dev.Store(addr, size, data) } -> std::same_as<void>;
};

template<Device... Devices>
class MMU : public BaseMMU
{
  public:

    MMU(Devices&... device_list) :
    paging_mode(PagingMode::Bare),
    privilege_mode(PrivilegeMode::MACHINE),
    page_size(PAGE_SIZE),
    devices(device_list...)
    {}

    void Load(uint64_t addr, int size, uint64_t& data) override;
    void Store(uint64_t addr, int size, uint64_t data) override;

    uint64_t Translate(uint64_t virtual_addr);
    Sv39PageTableEntry ParsePageTableEntry(uint64_t pte);

    void SetPagingMode(PagingMode mode) override { paging_mode = mode; }
    void SetRootPageTable(uint64_t page_table) override { root_page_table = page_table; }
    void SetPrivilegeMode(PrivilegeMode mode) { privilege_mode = mode; }

  private:

    PagingMode paging_mode;
    PrivilegeMode privilege_mode;
    int page_size;
    uint64_t root_page_table;
    // Devices
    std::tuple<Devices...> devices;

};

#endif
