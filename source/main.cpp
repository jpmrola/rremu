#include "config.h"
#include "cpu.h"
#include "elf_parser.h"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

// Parse options for loading an elf file or an xv6 image
int ParseOptions(int argc, char** argv)
{
  if(argc != 3)
  {
    std::cout << "Usage: " << argv[0] << " [option]" << " <binary>" << std::endl;
    return -1;
  }
  else if(std::string(argv[1]) == "-xv6")
  {
    return 0;
  }
  else if(std::string(argv[1]) == "-elf")
  {
    return 1;
  }
  else
  {
    std::cout << "Usage: " << argv[0] << " [option]" << " <binary>" << '\n';
    std::cout << "Options: -xv6, -elf" << std::endl;
    return -1;
  }
}

int main(int argc, char** argv)
{

  int option = ParseOptions(argc, argv);
  if(option == -1)
  {
    return -1;
  }
  else if(option == 0)
  {
    std::cout << "XV6 mode not implemented" << std::endl;
    return -1;
  }
  else if(option == 1)
  {
    std::cout << "ELF mode" << std::endl;
  }
  uint64_t entry_point;
  auto binary = std::make_shared<std::vector<uint8_t>>(LoadELF64(argv[2], entry_point));
  if(static_cast<uint64_t>(binary->size()) > MEMORY_SIZE)
  {
    std::cerr << "Binary too large" << std::endl;
    return -1;
  }
  else if(binary->empty() == true)
  {
    std::cerr << "Failure while reading or binary empty" << std::endl;
    return -1;
  }

  auto ram = std::make_unique<RAM<KERNBASE,MEMORY_SIZE>>(binary);
  auto mmu = std::make_unique<MMU<RAM<KERNBASE, MEMORY_SIZE>>>(*ram);
  auto cpu = std::make_unique<CPU<MMU<RAM<KERNBASE,MEMORY_SIZE>>>>(*mmu);

  cpu->SetPc(entry_point);

  std::chrono::time_point<std::chrono::system_clock> start, end;

  // Step mode
  std::cout << "PC: " << std::hex << cpu->GetPc() << std::endl;
  while(true)
  {
    start = std::chrono::system_clock::now();
    cpu->Step();
    end = std::chrono::system_clock::now();
    std::cin.get();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us" << std::endl;
  }

  return 0;
}
