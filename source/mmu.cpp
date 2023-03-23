#include <mmu.h>
#include <iostream>
#include "trap.h"

void MMU::Load(uint64_t addr, int size, uint64_t& data)
{
  uint64_t physical_addr = Translate(addr);
  if(ram.IsValidAddr(physical_addr))
  {
    ram.Load(physical_addr, size, data);
    return;
  }
  else
  {
    if(uart.IsValidAddr(physical_addr))
    {
      uart.Load(physical_addr, size, data);
      return;
    }
    else if(virtio.IsValidAddr(physical_addr))
    {
      virtio.Load(physical_addr, size, data);
      return;
    }
    else if(clint.IsValidAddr(physical_addr))
    {
      clint.Load(physical_addr, size, data);
      return;
    }
    else if(plic.IsValidAddr(physical_addr))
    {
      plic.Load(physical_addr, size, data);
      return;
    }
    else if(virtio.IsValidAddr(physical_addr))
    {
      virtio.Load(physical_addr, size, data);
      return;
    }
  throw CPUTrapException(trap_value::LoadAccessFault);
  }
}

void MMU::Store(uint64_t addr, int size, uint64_t data)
{
  uint64_t physical_addr = Translate(addr);
  if(ram.IsValidAddr(physical_addr))
  {
    ram.Store(physical_addr, size, data);
    return;
  }
  else
  {
    if(uart.IsValidAddr(physical_addr))
    {
      uart.Store(physical_addr, size, data);
      return;
    }
    else if(virtio.IsValidAddr(physical_addr))
    {
      virtio.Store(physical_addr, size, data);
      return;
    }
    else if(clint.IsValidAddr(physical_addr))
    {
      clint.Store(physical_addr, size, data);
      return;
    }
    else if(plic.IsValidAddr(physical_addr))
    {
      plic.Store(physical_addr, size, data);
      return;
    }
    else if(virtio.IsValidAddr(physical_addr))
    {
      virtio.Store(physical_addr, size, data);
      return;
    }
  throw CPUTrapException(trap_value::StoreAMOAccessFault);
  }
}

Sv39PageTableEntry MMU::ParsePageTableEntry(uint64_t pte)
{
  Sv39PageTableEntry entry;
  entry.v = (pte >> 0) & 0x1;
  entry.r = (pte >> 1) & 0x1;
  entry.w = (pte >> 2) & 0x1;
  entry.x = (pte >> 3) & 0x1;
  entry.u = (pte >> 4) & 0x1;
  entry.g = (pte >> 5) & 0x1;
  entry.a = (pte >> 6) & 0x1;
  entry.d = (pte >> 7) & 0x1;
  entry.rsw = (pte >> 8) & 0x3;
  entry.ppn0 = (pte >> 10) & 0x1FF;
  entry.ppn1 = (pte >> 19) & 0x1FF;
  entry.ppn2 = (pte >> 28) & 0x3FFFFFF;
  entry.reserved = (pte >> 54) & 0x3FF;
  return entry;
}

// Implements the Virtual Address Translation Algorithm from RISC-V Privileged ISA Manual
uint64_t MMU::Translate(uint64_t virtual_addr)
{
  if(paging_mode == Bare)
  {
    return virtual_addr;
  }
  else if(paging_mode != Sv39)
  {
    throw CPUTrapException(trap_value::InstructionPageFault);
  }

  switch(privilege_mode)
  {
    case MACHINE: // TODO(jrola): implement machine mode memory protection
    case SUPERVISOR:
    case USER:
    {
      std::array<uint64_t, 3> vpn = {
        (virtual_addr >> 12) & 0x1FF,
        (virtual_addr >> 21) & 0x1FF,
        (virtual_addr >> 30) & 0x1FF
      };
      uint64_t offset = virtual_addr & 0xFFF;
      int levels = vpn.size();
      int i = levels - 1;

      uint64_t a = root_page_table * page_size;
      uint64_t pte_raw = 0;
      Sv39PageTableEntry pte;

      for(; i >= 0; i--)
      {
        Load(a + vpn[i] * 8, 8, pte_raw);
        pte = ParsePageTableEntry(pte_raw);
        if(pte.v == 0 || (pte.r == 0 && pte.w == 1))
        {
          throw CPUTrapException(trap_value::InstructionPageFault);
        }
        if(pte.r == 1 || pte.x == 1) // leaf page TODO(jrola): implement memory protection
        {
          break;
        }
        else if(i == 0) // no leaf page found
        {
          throw CPUTrapException(trap_value::InstructionPageFault);
        }
        a = (pte.ppn2 << 30) | (pte.ppn1 << 21) | (pte.ppn0 << 12) * page_size;
      }
      switch(i)
      {
        case 0:
          return pte.ppn0 << 12 | offset;
          break;
        case 1:
          return pte.ppn2 << 30 | (pte.ppn1 << 21) | (vpn[0] << 12) | offset;
          break;
        case 2:
          return pte.ppn2 << 30 | (vpn[1] << 21) | (vpn[0] << 12) | offset;
          break;
        default:
          throw CPUTrapException(trap_value::InstructionPageFault);
          break;
      }
    }
      break;
    default:
      throw CPUTrapException(trap_value::InstructionPageFault);
      break;
  }
}
