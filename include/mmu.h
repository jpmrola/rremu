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

class MMU
{
  public:

    MMU(const std::shared_ptr<std::vector<uint8_t>>& binary) :
    paging_mode(PagingMode::Bare),
    privilege_mode(PrivilegeMode::MACHINE),
    page_size(PAGE_SIZE),
    ram(RAM<KERNBASE, MEMORY_SIZE>(binary)) {}

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
    RAM<KERNBASE, MEMORY_SIZE> ram;
    UART<UART_BASE, UART_SIZE> uart;
    VIRTIO<VIRTIO_BASE, VIRTIO_SIZE> virtio;
    CLINT<CLINT_BASE, CLINT_SIZE> clint;
    PLIC<PLIC_BASE, PLIC_SIZE> plic;

};

#endif
