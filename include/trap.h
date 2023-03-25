#ifndef TRAP_H
#define TRAP_H

#include <iostream>
#include <stdexcept>

static constexpr uint64_t interrupt_bit = 0x8000000000000000;
// static constexpr uint32_t interrupt_bit = 0x80000000; if xlen is 32 bit TODO(jrola): implement

enum trap_value : uint64_t
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
  StoreAMOPageFault = 0xF,
  UserSoftwareInterrupt = interrupt_bit,
  SupervisorSoftwareInterrupt = interrupt_bit + 1,
  MachineSoftwareInterrupt = interrupt_bit + 3,
  UserTimerInterrupt = interrupt_bit + 4,
  SupervisorTimerInterrupt = interrupt_bit + 5,
  MachineTimerInterrupt = interrupt_bit + 7,
  UserExternalInterrupt = interrupt_bit + 8,
  SupervisorExternalInterrupt = interrupt_bit + 9,
  MachineExternalInterrupt = interrupt_bit + 11
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
        case UserSoftwareInterrupt:
          return "UserSoftwareInterrupt";
        case SupervisorSoftwareInterrupt:
          return "SupervisorSoftwareInterrupt";
        case MachineSoftwareInterrupt:
          return "MachineSoftwareInterrupt";
        case UserTimerInterrupt:
          return "UserTimerInterrupt";
        case SupervisorTimerInterrupt:
          return "SupervisorTimerInterrupt";
        case MachineTimerInterrupt:
          return "MachineTimerInterrupt";
        case UserExternalInterrupt:
          return "UserExternalInterrupt";
        case SupervisorExternalInterrupt:
          return "SupervisorExternalInterrupt";
        case MachineExternalInterrupt:
          return "MachineExternalInterrupt";
        default:
          return "Unknown Trap";
      }
    }
    trap_value GetTrap() const { return trap; }
  private:
    const trap_value trap;
};

#endif
