#include <elf.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <fstream>
#include "config.h"

// Load each section to the correct address
std::vector<uint8_t> LoadELF64(const std::string& file, uint64_t& entry_point)
{
  std::ifstream in(file, std::ios::binary);
  if(!in.is_open())
  {
      throw std::runtime_error("Could not open file");
  }
  in.seekg(0, std::ios::end);
  auto size = in.tellg();
  in.seekg(0, std::ios::beg);
  std::vector<uint8_t> binary(size);
  in.read(reinterpret_cast<char*>(&binary[0]), size);
  in.close();

  if(binary.size() < sizeof(Elf64_Ehdr) || binary[0] != ELFMAG0 || binary[1] != ELFMAG1 || binary[2] != ELFMAG2 || binary[3] != ELFMAG3)
  {
      throw std::runtime_error("Not an ELF file");
  }
  Elf64_Ehdr *header = reinterpret_cast<Elf64_Ehdr*>(&binary[0]);

  if(header->e_ident[EI_CLASS] != ELFCLASS64 || header->e_machine != EM_RISCV)
  {
      throw std::runtime_error("Not a 64-bit RISC-V ELF file");
  }

  entry_point = header->e_entry;

  std::vector<uint8_t> mem(MEMORY_SIZE, 0);

  Elf64_Phdr *phdrs = reinterpret_cast<Elf64_Phdr*>(&binary[header->e_phoff]);
  for(int i = 0; i < header->e_phnum; i++)
  {
    if(phdrs[i].p_type == PT_LOAD)
    {
      if(phdrs[i].p_filesz > phdrs[i].p_memsz)
      {
          throw std::runtime_error("Invalid ELF file");
      }
      if(phdrs[i].p_filesz > 0)
      {
        std::memcpy(&mem[phdrs[i].p_vaddr], &binary[phdrs[i].p_offset], phdrs[i].p_filesz);
      }
      if(phdrs[i].p_memsz > phdrs[i].p_filesz)
      {
        std::memset(&mem[phdrs[i].p_vaddr + phdrs[i].p_filesz], 0, phdrs[i].p_memsz - phdrs[i].p_filesz);
      }
    }
  }

  return mem;
}
