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
    std::cout << "xv6 mode not implemented" << std::endl;
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

  auto cpu = std::make_unique<CPU>(binary, entry_point);
  if(cpu == nullptr)
  {
    return -1;
  }

  std::chrono::time_point<std::chrono::system_clock> start, end;
  auto except = false;

  // Step mode
  std::cout << "PC: " << std::hex << cpu->GetPc() << std::endl;
  while(true)
  {
    except = false;
    start = std::chrono::system_clock::now();
    try
    {
      cpu->Step();
    }
    catch(const CPUTrapException& e)
    {
      end = std::chrono::system_clock::now();
      std::cout << "Trap: " << e.what() << std::endl;
      except = true;
    }
    if(!except)
    {
      end = std::chrono::system_clock::now();
    }
    std::cin.get();
    std::cout << std::dec << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us" << std::endl;
    std::cout << "PC: " << std::hex << cpu->GetPc() << std::endl;
  }

  return 0;
}
