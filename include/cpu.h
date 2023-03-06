#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>
#include <memory>
#include <string>
#include "mmu.h"
#include "config.h"

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

enum CSR : uint16_t
{
  // Supervisor CSRs
  sstatus = 0x100,
  sie = 0x104,
  stvec = 0x105,
  sscratch = 0x140,
  sepc = 0x141,
  scause = 0x142,
  stval = 0x143,
  sip = 0x144,
  satp = 0x180,
  // Machine CSRs
  mstatus = 0x300,
  medeleg = 0x302,
  mideleg = 0x303,
  mie = 0x304,
  mtvec = 0x305,
  mscratch = 0x340,
  mepc = 0x341,
  mcause = 0x342,
  mtval = 0x343,
  mip = 0x344,
};

// Machine Interrupt Register (MIP)
enum MIP : uint64_t
{
  ssip = 1ULL << 1,
  msip = 1ULL << 3,
  stip = 1ULL << 5,
  mtip = 1ULL << 7,
  seip = 1ULL << 9,
  meip = 1ULL << 11
};

// Forward declarations for structs used in the CPU class
typedef struct Instruction Instruction;
typedef struct InstructionFields InstructionFields;

class CPU
{
  public:

    CPU(const std::shared_ptr<std::vector<uint8_t>> binary) :
    pc(KERNBASE),
    priv_mode(PrivilegeMode::MACHINE),
    mmu(MMU(binary))
    {}

    CPU(const std::shared_ptr<std::vector<uint8_t>> binary, const uint64_t entry_point) :
    pc(entry_point),
    priv_mode(PrivilegeMode::MACHINE),
    mmu(MMU(binary))
    {}

    const Instruction& Decode(uint32_t instruction);
    int Execute();
    uint32_t Fetch();
    void Run();
    int Step();

    void UpdatePagingMode(uint64_t satp);

    inline void Store(uint64_t addr, int size, uint64_t data) { mmu.Store(addr, size, data); }
    inline void Load(uint64_t addr, int size, uint64_t& data) { mmu.Load(addr, size, data); }

    PrivilegeMode GetMode() const { return priv_mode; }
    void SetMode(PrivilegeMode mode) { priv_mode = mode; }

    uint64_t GetPc() const { return pc; }
    void SetPc(uint64_t addr) { pc = addr; }

    uint64_t GetReg(int reg) const { return regs[reg]; }
    void SetReg(int reg, uint64_t val) { regs[reg] = val; }

    uint64_t GetCsr(int csr) const { return csrs[csr]; }
    void SetCsr(int csr, uint64_t val)
    {
      csrs[csr] = val;
      if(csr == CSR::satp)
      {
        UpdatePagingMode(val);
      }
    }

    int RunInstruction(uint32_t instruction_bits); // For testing
    void DumpRegs(); // For testing
    void DumpCsrs(); // For testing
    void DumpInstruction(const Instruction& instruction); // For testing
    void DumpInstructionFields(InstructionFields fields); // For testing

  private:

    const uint64_t xlen = 64;  // hardcoded 64-bit
    uint64_t pc;
    std::array<uint64_t, N_REG> regs {0};
    std::array<uint64_t, N_CSR> csrs {0};
    PrivilegeMode priv_mode;
    uint64_t& reg_zero = regs[0];
    MMU mmu;
};

typedef struct Instruction
{
  const std::string name;
  const char format;
  const uint32_t mask_field;
  const uint32_t instruction_matcher;
  void (*const execute)(const uint32_t instruction, CPU& cpu);
} Instruction;

typedef struct InstructionFields
{
  const uint32_t opcode;
  const uint32_t rd;
  const uint32_t funct3;
  const uint32_t rs1;
  const uint32_t rs2;
  const uint32_t funct7;
  const uint32_t imm;
} InstructionFields;


#endif
