#ifndef TRAP_H
#define TRAP_H

#include <iostream>
#include <stdexcept>

enum trap_value : uint8_t
{
  InstructionAddressMisaligned = 0x0,
  InstructionAccessFault = 0x1,
  IllegalInstruction = 0x2,
  Breakpoint = 0x3,
  LoadAddressMisaligned = 0x4,
  LoadAccessFault = 0x5,
  StoreAMOAddressMisaligned = 0x6,
  StoreAMOAccessFault = 0x7,
  EnvironmentCallFromUMode = 0x8,
  EnvironmentCallFromSMode = 0x9,
  EnvironmentCallFromMMode = 0xB,
  InstructionPageFault = 0xC,
  LoadPageFault = 0xD,
  StoreAMOPageFault = 0xF
};

class CPUTrapException : public std::exception
{
  public:
    CPUTrapException(const trap_value trap) : trap(trap) {}
    virtual const char* what() const noexcept override
    {
      switch(trap)
      {
        case InstructionAddressMisaligned:
          return "Instruction Address Misaligned";
        case InstructionAccessFault:
          return "Instruction Access Fault";
        case IllegalInstruction:
          return "Illegal Instruction";
        case Breakpoint:
          return "Breakpoint";
        case LoadAddressMisaligned:
          return "Load Address Misaligned";
        case LoadAccessFault:
          return "Load Access Fault";
        case StoreAMOAddressMisaligned:
          return "Store/AMO Address Misaligned";
        case StoreAMOAccessFault:
          return "Store/AMO Access Fault";
        case EnvironmentCallFromUMode:
          return "Environment Call from U-Mode";
        case EnvironmentCallFromSMode:
          return "Environment Call from S-Mode";
        case EnvironmentCallFromMMode:
          return "Environment Call from M-Mode";
        case InstructionPageFault:
          return "Instruction Page Fault";
        case LoadPageFault:
          return "Load Page Fault";
        case StoreAMOPageFault:
          return "Store/AMO Page Fault";
        default:
          return "Unknown Trap";
      }
    }
    trap_value GetTrap() const { return trap; }
  private:
    const trap_value trap;
};

#endif
