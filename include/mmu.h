#ifndef MMU_H
#define MMU_H

#include <cstdint>
#include <vector>
#include <array>
#include <memory>
#include "config.h"
#include "ram.h"
#include "uart.h"
#include "virtio.h"
#include "clint.h"
#include "plic.h"

typedef enum AccessType
{
    Execute,
    Load,
    Store,
} AccessType;

typedef enum PagingMode
{
    Bare,
    Sv32,
    Sv39,
    Sv48,
    Sv57,
} PagingMode;

typedef struct Sv39PageTableEntry
{
    uint8_t v : 1;
    uint8_t r : 1;
    uint8_t w : 1;
    uint8_t x : 1;
    uint8_t u : 1;
    uint8_t g : 1;
    uint8_t a : 1;
    uint8_t d : 1;
    uint8_t rsw : 2;
    uint16_t ppn0 : 9;
    uint16_t ppn1 : 9;
    uint32_t ppn2 : 26;
    uint16_t reserved : 10;
} Sv39PageTableEntry;

typedef enum
{
  USER = 0x0,
  SUPERVISOR = 0x1,
  RESERVED=0x2,
  MACHINE = 0x3
} PrivilegeMode;

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
class MMU
{
  public:

    MMU(Devices&... device_list) :
    paging_mode(PagingMode::Bare),
    privilege_mode(PrivilegeMode::MACHINE),
    page_size(PAGE_SIZE),
    devices(device_list...)
    {}

    void Load(uint64_t addr, int size, uint64_t& data);
    void Store(uint64_t addr, int size, uint64_t data);

    uint64_t Translate(uint64_t virtual_addr);
    Sv39PageTableEntry ParsePageTableEntry(uint64_t pte);

    void SetPagingMode(PagingMode mode) { paging_mode = mode; }
    void SetRootPageTable(uint64_t page_table) { root_page_table = page_table; }
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
