#include <elf.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <fstream>
#include <iostream>
#include "config.h"

std::vector<uint8_t> ReadIntoVector(const std::string &file)
{
  std::ifstream in(file, std::ios::binary | std::ios::ate);
  if (!in.is_open())
  {
    throw std::runtime_error("Could not open file");
  }
  auto size = in.tellg();
  in.seekg(0, std::ios::beg);
  std::vector<uint8_t> binary(size);
  in.read(reinterpret_cast<char *>(binary.data()), size);
  in.close();
  return binary;
}

std::unique_ptr<Elf64_Ehdr> GetHeader(std::vector<uint8_t>& binary)
{
  if (binary.size() < sizeof(Elf64_Ehdr) || binary[0] != ELFMAG0 || binary[1] != ELFMAG1 || binary[2] != ELFMAG2 || binary[3] != ELFMAG3)
  {
    throw std::runtime_error("Not an ELF file");
  }

  auto header = std::make_unique<Elf64_Ehdr>(*reinterpret_cast<Elf64_Ehdr *>(binary.data()));

  if (header->e_ident[EI_CLASS] != ELFCLASS64 || header->e_machine != EM_RISCV)
  {
    throw std::runtime_error("Not a 64-bit RISC-V ELF file");
  }
  return header;
}

std::vector<uint8_t> LoadSegments(const std::vector<uint8_t>& binary, std::unique_ptr<Elf64_Ehdr> header)
{
  std::vector<uint8_t> mem(MEMORY_SIZE, 0);
  std::vector<Elf64_Phdr> phdrs(header->e_phnum);
  std::memcpy(phdrs.data(), &binary.at(header->e_phoff), header->e_phnum * sizeof(Elf64_Phdr));

  for (const auto &segment : phdrs)
  {
    if (segment.p_type == PT_LOAD)
    {
      auto segment_offset = segment.p_vaddr - KERNBASE;
      if (segment_offset + segment.p_memsz > mem.size())
      {
        throw std::runtime_error("Segment too large");
      }
      std::memcpy(&mem.at(segment_offset), &binary.at(segment.p_offset), segment.p_filesz);
    }
  }
  return mem;
}

std::vector<uint8_t> LoadELF64(const std::string &file, uint64_t &entry_point)
{
  try
  {
    std::vector<uint8_t> binary = ReadIntoVector(file);
    auto header = GetHeader(binary);
    entry_point = header->e_entry;
    auto mem = LoadSegments(binary, std::move(header));
    return mem;
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return {};
  }
}
