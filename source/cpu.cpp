#include "cpu.h"
#include <iostream>
#include <map>

// TODO(jrola): MISSING EXTENSIONS to G (F, D, Zicsr, Zifencei)
//              MISSING INSTRUCTIONS IN RV32I: FENCE, ECALL, EBREAK

// instruction mask helpers

template<int start, int end>
static constexpr uint32_t mask(uint32_t instruction)
{
  return (instruction >> start) & ((1 << (end - start + 1)) - 1);
}

template<int start, int end, int dest>
static constexpr uint32_t mask_and_shift(uint32_t instruction)
{
  return ((instruction >> start) & ((1 << (end - start + 1)) - 1)) << dest;
}

template<uint64_t start, uint64_t end>
static constexpr uint64_t mask(uint64_t instruction)
{
  return (instruction >> start) & ((1ULL << (end - start + 1ULL)) - 1ULL);
}

template<uint64_t start, uint64_t end, uint64_t dest>
static constexpr uint64_t mask_and_shift(uint64_t instruction)
{
  return ((instruction >> start) & ((1ULL << (end - start + 1ULL)) - 1ULL)) << dest;
}

constexpr auto sign_extend_12b = [](auto imm) -> int32_t {
  int32_t sign_extended_imm = (imm & 0x800) ? (imm | 0xfffff000) : imm;
  return sign_extended_imm;
};

constexpr auto sign_extend_13b = [](auto imm) -> int32_t {
  int32_t sign_extended_imm = (imm & 0x1000) ? (imm | 0xffffe000) : imm;
  return sign_extended_imm;
};

template<char format>
InstructionFields parse_instruction(const uint32_t instruction)
{
  throw std::runtime_error("Invalid instruction format");
}

template<>
InstructionFields parse_instruction<'R'>(const uint32_t instruction)
{
  return InstructionFields{.opcode = mask<0, 6>(instruction),
                           .rd     = mask<7, 11>(instruction),
                           .funct3 = mask<12, 14>(instruction),
                           .rs1    = mask<15, 19>(instruction),
                           .rs2    = mask<20, 24>(instruction),
                           .funct7 = mask<25, 31>(instruction),
                           .imm = 0};
}

template<>
InstructionFields parse_instruction<'I'>(const uint32_t instruction)
{
  return InstructionFields{.opcode = mask<0, 6>(instruction),
                           .rd     = mask<7, 11>(instruction),
                           .funct3 = mask<12, 14>(instruction),
                           .rs1    = mask<15, 19>(instruction),
                           .rs2    = 0,
                           .funct7 = 0,
                           .imm = mask<20, 31>(instruction)};
}

template<>
InstructionFields parse_instruction<'S'>(const uint32_t instruction)
{
  return InstructionFields{.opcode = mask<0, 6>(instruction),
                           .rd     = 0,
                           .funct3 = mask<12, 14>(instruction),
                           .rs1    = mask<15, 19>(instruction),
                           .rs2    = mask<20, 24>(instruction),
                           .funct7 = 0,
                           .imm = mask<7, 11>(instruction) | mask_and_shift<25, 31, 5>(instruction)};
}

template<>
InstructionFields parse_instruction<'B'>(const uint32_t instruction)
{
  return InstructionFields{.opcode = mask<0, 6>(instruction),
                           .rd     = 0,
                           .funct3 = mask<12, 14>(instruction),
                           .rs1    = mask<15, 19>(instruction),
                           .rs2    = mask<20, 24>(instruction),
                           .funct7 = 0,
                           .imm = mask_and_shift<7, 7, 11>(instruction) | mask_and_shift<8, 11, 1>(instruction)
                                | mask_and_shift<25, 30, 5>(instruction) | mask_and_shift<31, 31, 12>(instruction)};
}

template<>
InstructionFields parse_instruction<'U'>(const uint32_t instruction)
{
  return InstructionFields{.opcode = mask<0, 6>(instruction),
                           .rd     = mask<7, 11>(instruction),
                           .funct3 = 0,
                           .rs1    = 0,
                           .rs2    = 0,
                           .funct7 = 0,
                           .imm    = mask_and_shift<12, 31, 12>(instruction)};
}

template<>
InstructionFields parse_instruction<'J'>(const uint32_t instruction)
{
  return InstructionFields{.opcode = mask<0, 6>(instruction),
                           .rd     = mask<7, 11>(instruction),
                           .funct3 = 0,
                           .rs1    = 0,
                           .rs2    = 0,
                           .funct7 = 0,
                           .imm = mask_and_shift<21, 30, 1>(instruction) | mask_and_shift<20, 20, 11>(instruction)
                                | mask_and_shift<12, 19, 12>(instruction) | mask_and_shift<31, 31, 20>(instruction)};
}


const static Instruction instructions[] = {
  // RV32I
  // INSTRUCTIONS IN RV32I: LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU,
  //                        LB, LH, LW, LBU, LHU, SB, SH, SW,
  //                        ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,
  //                        ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND
  {
    .name = "LUI",
    .format = 'U',
    .mask_field = 0x0000007f,
    .instruction_matcher = 0x00000037,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'U'>(instruction);
      cpu.SetReg(fields.rd, static_cast<int32_t>(fields.imm));
    }
  },
  {
    .name = "AUIPC",
    .format = 'U',
    .mask_field = 0x0000007f,
    .instruction_matcher = 0x00000017,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'U'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetPc() + static_cast<uint64_t>(fields.imm) - 4);
    }
  },
  {
    .name = "JAL",
    .format = 'J',
    .mask_field = 0x0000007f,
    .instruction_matcher = 0x0000006f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'J'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetPc());   // PC is incremented by 4 after instruction fetch, no need to add 4
      cpu.SetPc(cpu.GetPc() + static_cast<int32_t>(fields.imm) - 4);  // -4 because PC is incremented by 4 after instruction fetch
    }
  },
  {
    .name = "JALR",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00000067,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetPc());   // PC is incremented by 4 after instruction fetch, no need to add 4
      cpu.SetPc((cpu.GetReg(fields.rs1) + static_cast<int32_t>(fields.imm)) & ~1);
    }
  },
  {
    .name = "BEQ",
    .format = 'B',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00000063,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'B'>(instruction);
      if(cpu.GetReg(fields.rs1) == cpu.GetReg(fields.rs2))
      {
        cpu.SetPc(cpu.GetPc() + sign_extend_13b(fields.imm) - 4);
      }
    }
  },
  {
    .name = "BNE",
    .format = 'B',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00001063,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'B'>(instruction);
      if(cpu.GetReg(fields.rs1) != cpu.GetReg(fields.rs2))
      {
        cpu.SetPc(cpu.GetPc() + sign_extend_13b(fields.imm) - 4);
      }
    }
  },
  {
    .name = "BLT",
    .format = 'B',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00004063,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'B'>(instruction);
      if(static_cast<int64_t>(cpu.GetReg(fields.rs1)) < static_cast<int64_t>(cpu.GetReg(fields.rs2)))
      {
        cpu.SetPc(cpu.GetPc() + sign_extend_13b(fields.imm) - 4);
      }
    }
  },
  {
    .name = "BGE",
    .format = 'B',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00005063,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'B'>(instruction);
      if(static_cast<int64_t>(cpu.GetReg(fields.rs1)) >= static_cast<int64_t>(cpu.GetReg(fields.rs2)))
      {
        cpu.SetPc(cpu.GetPc() + sign_extend_13b(fields.imm) - 4);
      }
    }
  },
  {
    .name = "BLTU",
    .format = 'B',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00006063,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'B'>(instruction);
      if(cpu.GetReg(fields.rs1) < cpu.GetReg(fields.rs2))
      {
        cpu.SetPc(cpu.GetPc() + sign_extend_13b(fields.imm) - 4);
      }
    }
  },
  {
    .name = "BGEU",
    .format = 'B',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00007063,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'B'>(instruction);
      if(cpu.GetReg(fields.rs1) >= cpu.GetReg(fields.rs2))
      {
        cpu.SetPc(cpu.GetPc() + sign_extend_13b(fields.imm) - 4);
      }
    }
  },
  {
    .name = "LB",
    .format = 'I',
		.mask_field = 0x0000707f,
		.instruction_matcher = 0x00000003,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data;
      cpu.Load(addr, 1, data);
      cpu.SetReg(fields.rd, static_cast<int8_t>(data));
    }
  },
  {
    .name = "LH",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00001003,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data;
      cpu.Load(addr, 2, data);
      cpu.SetReg(fields.rd, static_cast<int16_t>(data));
    }
  },
  {
    .name = "LW",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00002003,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data;
      cpu.Load(addr, 4, data);
      cpu.SetReg(fields.rd, static_cast<int32_t>(data));
    }
  },
  {
    .name = "LBU",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00004003,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data;
      cpu.Load(addr, 1, data);
      cpu.SetReg(fields.rd, data);
    }
  },
  {
    .name = "LHU",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00005003,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data;
      cpu.Load(addr, 2, data);
      cpu.SetReg(fields.rd, data);
    }
  },
  {
    .name = "SB",
    .format = 'S',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00000023,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'S'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data = cpu.GetReg(fields.rs2);
      cpu.Store(addr, 1, data);
    }
  },
  {
    .name = "SH",
    .format = 'S',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00001023,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'S'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data = cpu.GetReg(fields.rs2);
      cpu.Store(addr, 2, data);
    }
  },
  {
    .name = "SW",
    .format = 'S',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00002023,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'S'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data = cpu.GetReg(fields.rs2);
      cpu.Store(addr, 4, data);
    }
  },

  {
    .name = "ADDI",
    .format = 'I',
    .mask_field = 0x0000707f,
  .instruction_matcher = 0x00000013,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) + sign_extend_12b(fields.imm));
    }
  },
  {
    .name = "SLTI",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00002013,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) < fields.imm);
    }
  },
  {
    .name = "SLTIU",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00003013,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) < fields.imm);
    }
  },
  {
    .name = "XORI",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00004013,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) ^ fields.imm);
    }
  },
  {
    .name = "ORI",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00006013,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) | fields.imm);
    }
  },
  {
    .name = "ANDI",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00007013,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) & fields.imm);
    }
  },
  {
    .name = "SLLI",
    .format = 'I',
    .mask_field = 0xfc00707f,
    .instruction_matcher = 0x00001013,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) << fields.imm);
    }
  },
  {
    .name = "SRLI",
    .format = 'I',
    .mask_field = 0xfc00707f,
    .instruction_matcher = 0x00005013,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) >> fields.imm);
    }
  },
  {
    .name = "SRAI",
    .format = 'I',
    .mask_field = 0xfc00707f,
    .instruction_matcher = 0x40005013,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) >> fields.imm);
    }
  },
  {
    .name = "ADD",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x00000033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) + cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "SUB",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x40000033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) - cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "SLL",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x00001033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) << cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "SLT",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x00002033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) < cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "SLTU",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x00003033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) < cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "XOR",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x00004033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) ^ cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "SRL",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x00005033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) >> cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "SRA",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x40005033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) >> cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "OR",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x00006033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) | cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "AND",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x00007033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) & cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "FENCE",
    .format = 'I',
    .mask_field = 0xffffffff,
    .instruction_matcher = 0x0000000f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
    }
  },
  {
    .name = "ECALL",
    .format = 'I',
  .mask_field = 0xffffffff,
  .instruction_matcher = 0x00000073,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      switch(cpu.GetMode())
      {
        case PrivilegeMode::MACHINE:
          // implement trap handler
          break;
        case PrivilegeMode::RESERVED:
          // implement trap handler
          break;
        case PrivilegeMode::SUPERVISOR:
          // implement trap handler
          break;
        case PrivilegeMode::USER:
          // implement trap handler
          break;
      }
    }
  },
  {
    .name = "EBREAK",
    .format = 'I',
    .mask_field = 0xffffffff,
    .instruction_matcher = 0x00100073,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      // implement trap handler
    }
  },
  // RV32I
  // ----------------------------------------
  // RV64I
  // INSTRUCTIONS IN RV64I: LWU, LD, SD,
  //                        ADDIW, SLLIW, SRLIW, SRAIW, ADDW, SUBW, SLLW, SRLW, SRAW
  {
    .name = "LWU",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00006003,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data;
      cpu.Load(addr, 4, data);
      cpu.SetReg(fields.rd, data);
    }
  },
  {
    .name = "LD",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00003003,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data;
      cpu.Load(addr, 8, data);
      cpu.SetReg(fields.rd, data);
    }
  },
  {
    .name = "SD",
    .format = 'S',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00003023,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'S'>(instruction);
      const uint64_t addr = cpu.GetReg(fields.rs1) + fields.imm;
      uint64_t data = cpu.GetReg(fields.rs2);
      cpu.Store(addr, 8, data);
    }
  },
  {
    .name = "ADDIW",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x0000001b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      cpu.SetReg(fields.rd, static_cast<int64_t>(static_cast<int32_t>(cpu.GetReg(fields.rs1) + sign_extend_12b(fields.imm))));
    }
  },
  {
    .name = "SLLIW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x0000101b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) << fields.imm);
    }
  },
  {
    .name = "SRLIW",
    .format = 'R',
    .mask_field = 0xfc00707f,
    .instruction_matcher = 0x0000501b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) >> fields.imm);
    }
  },
  {
    .name = "SRAIW",
    .format = 'R',
    .mask_field = 0xfc00707f,
    .instruction_matcher = 0x4000501b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) >> fields.imm);
    }
  },
  {
    .name = "ADDW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x0000003b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) + cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "SUBW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x4000003b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) - cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "SLLW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x0000103b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) << cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "SRLW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x0000503b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) >> cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "SRAW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x4000503b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) >> cpu.GetReg(fields.rs2));
    }
  },
  // RV64I
  // ----------------------------------------
  // RV32/RV64 Zifencei
  // INSTRUCTIONS IN RV32/RV64 Zifencei: FENCE.I
  {
    .name = "FENCE.I",
    .format = 'I',
    .mask_field = 0x0000007f,
    .instruction_matcher = 0x0000000f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      // not implemented
    }
  },
  // RV32/RV64 Zifencei
  // ----------------------------------------
  // RV32/RV64 Zicsr
  // INSTRUCTIONS IN RV32/RV64 Zicsr: CSRRW, CSRRS, CSRRC, CSRRWI, CSRRSI, CSRRCI
  {
    .name = "CSRRW",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00001073,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      uint32_t csr = cpu.GetCsr(fields.imm);
      cpu.SetCsr(fields.imm, cpu.GetReg(fields.rs1));
      cpu.SetReg(fields.rd, csr);
    }
  },
  {
    .name = "CSRRS",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00002073,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      uint32_t csr = cpu.GetCsr(fields.imm);
      cpu.SetCsr(fields.imm, csr | cpu.GetReg(fields.rs1));
      cpu.SetReg(fields.rd, csr);
    }
  },
  {
    .name = "CSRRC",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00003073,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      uint32_t csr = cpu.GetCsr(fields.imm);
      cpu.SetCsr(fields.imm, csr & ~cpu.GetReg(fields.rs1));
      cpu.SetReg(fields.rd, csr);
    }
  },
  {
    .name = "CSRRWI",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00005073,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      uint32_t csr = cpu.GetCsr(fields.imm);
      cpu.SetCsr(fields.imm, fields.rs1);
      cpu.SetReg(fields.rd, csr);
    }
  },
  {
    .name = "CSRRSI",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00006073,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      uint32_t csr = cpu.GetCsr(fields.imm);
      cpu.SetCsr(fields.imm, csr | fields.rs1);
      cpu.SetReg(fields.rd, csr);
    }
  },
  {
    .name = "CSRRCI",
    .format = 'I',
    .mask_field = 0x0000707f,
    .instruction_matcher = 0x00007073,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'I'>(instruction);
      uint32_t csr = cpu.GetCsr(fields.imm);
      cpu.SetCsr(fields.imm, csr & ~fields.rs1);
      cpu.SetReg(fields.rd, csr);
    }
  },
  // RV32/RV64 Zicsr
  // ----------------------------------------
  // RV32M
  // INSTRUCTIONS IN RV32M: MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU
  {
    .name = "MUL",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x02000033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) * cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "MULH",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x02001033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) * cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "MULHSU",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x02002033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) * cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "MULHU",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x02003033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) * cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "DIV",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x02004033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      int64_t dividend = static_cast<int64_t>(cpu.GetReg(fields.rs1));
      int64_t divisor = static_cast<int64_t>(cpu.GetReg(fields.rs2));
      if(divisor == 0)
      {
        cpu.SetReg(fields.rd, -1);
        return;
      }
      else if (dividend == INT64_MIN && divisor == -1)
      {
        cpu.SetReg(fields.rd, dividend);
        return;
      }
      cpu.SetReg(fields.rd, dividend / divisor);
    }
  },
  {
    .name = "DIVU",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x02005033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t dividend = cpu.GetReg(fields.rs1);
      uint64_t divisor = cpu.GetReg(fields.rs2);
      if(divisor == 0)
      {
        cpu.SetReg(fields.rd, -1);
        return;
      }
      cpu.SetReg(fields.rd, dividend / divisor);
    }
  },
  {
    .name = "REM",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x02006033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      int64_t dividend = static_cast<int64_t>(cpu.GetReg(fields.rs1));
      int64_t divisor = static_cast<int64_t>(cpu.GetReg(fields.rs2));
      if(divisor == 0)
      {
        cpu.SetReg(fields.rd, dividend);
        return;
      }
      else if (dividend == INT64_MIN && divisor == -1)
      {
        cpu.SetReg(fields.rd, 0);
        return;
      }
      cpu.SetReg(fields.rd, dividend % divisor);
    }
  },
  {
    .name = "REMU",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x02007033,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t dividend = cpu.GetReg(fields.rs1);
      uint64_t divisor = cpu.GetReg(fields.rs2);
      if(divisor == 0)
      {
        cpu.SetReg(fields.rd, dividend);
        return;
      }
      cpu.SetReg(fields.rd, dividend % divisor);
    }
  },
  // RV32M
  // ----------------------------------------
  // RV64M
  // INSTRUCTION IN RV64M: MULW, DIVW, DIVUW, REMW, REMUW
  {
    .name = "MULW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x0200003b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      cpu.SetReg(fields.rd, cpu.GetReg(fields.rs1) * cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "DIVW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x0200403b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      int32_t dividend = static_cast<int32_t>(cpu.GetReg(fields.rs1));
      int32_t divisor = static_cast<int32_t>(cpu.GetReg(fields.rs2));
      if(divisor == 0)
      {
        cpu.SetReg(fields.rd, -1);
        return;
      }
      else if(dividend == INT32_MIN && divisor == -1)
      {
        cpu.SetReg(fields.rd, dividend);
        return;
      }
      cpu.SetReg(fields.rd, dividend / divisor);
    }
  },
  {
    .name = "DIVUW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x0200503b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint32_t dividend = static_cast<uint32_t>(cpu.GetReg(fields.rs1));
      uint32_t divisor = static_cast<uint32_t>(cpu.GetReg(fields.rs2));
      if(divisor == 0)
      {
        cpu.SetReg(fields.rd, -1);
        return;
      }
      cpu.SetReg(fields.rd, dividend / divisor);
    }
  },
  {
    .name = "REMW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x0200603b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      int32_t dividend = static_cast<int32_t>(cpu.GetReg(fields.rs1));
      int32_t divisor = static_cast<int32_t>(cpu.GetReg(fields.rs2));
      if(divisor == 0)
      {
        cpu.SetReg(fields.rd, dividend);
        return;
      }
      else if (dividend == INT32_MIN && divisor == -1)
      {
        cpu.SetReg(fields.rd, 0);
        return;
      }
      cpu.SetReg(fields.rd, dividend % divisor);
    }
  },
  {
    .name = "REMUW",
    .format = 'R',
    .mask_field = 0xfe00707f,
    .instruction_matcher = 0x0200703b,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint32_t dividend = static_cast<uint32_t>(cpu.GetReg(fields.rs1));
      uint32_t divisor = static_cast<uint32_t>(cpu.GetReg(fields.rs2));
      if(divisor == 0)
      {
        cpu.SetReg(fields.rd, dividend);
        return;
      }
      cpu.SetReg(fields.rd, dividend % divisor);
    }
  },
  // RV64M
  // ----------------------------------------
  // RV32A
  // INSTRUCTIONS IN RV32A: LR.W, SC.W, AMOSWAP.W, AMOADD.W,
  //                        AMOXOR.W, AMOAND.W, AMOOR.W,
  //                        AMOMIN.W, AMOMAX.W, AMOMINU.W, AMOMAXU.W
  {
    .name = "LR.W",
    .format = 'R',
    .mask_field = 0xf9f0707f,
    .instruction_matcher = 0x1000202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data;
      cpu.Load(fields.rs1, 4, data);
      cpu.SetReg(fields.rd, data);
      // TODO(jrola): reservation set
    }
  },
  {
    .name = "SC.W",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x1800202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs2);
      cpu.Store(fields.rs1, 4, data);
      cpu.SetReg(fields.rd, 0);
      // TODO(jrola): reservation set
    }
  },
  {
    .name = "AMOSWAP.W",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x0800202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), cpu.GetReg(fields.rs2));
      cpu.SetReg(fields.rd, data);
    }
  },
  {
    .name = "AMOADD.W",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x0000202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), data + cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "AMOXOR.W",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x2000202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), data ^ cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "AMOAND.W",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x6000202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), data & cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "AMOOR.W",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x4000202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), data | cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "AMOMIN.W",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x8000202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), std::min(data, cpu.GetReg(fields.rs2)));
    }
  },
  {
    .name = "AMOMAX.W",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0xa000202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), std::max(data, cpu.GetReg(fields.rs2)));
    }
  },
  {
    .name = "AMOMINU.W",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0xc000202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), std::min(data, cpu.GetReg(fields.rs2)));
    }
  },
  {
    .name = "AMOMAXU.W",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0xe000202f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), std::max(data, cpu.GetReg(fields.rs2)));
    }
  },
  // RV32A
  // ----------------------------------------
  // RV64A
  // INSTRUCTIONS IN RV64A: LR.D, SC.D, AMOSWAP.D, AMOADD.D,
  //                        AMOXOR.D, AMOAND.D,
  {
    .name = "LR.D",
    .format = 'R',
    .mask_field = 0xf9f0707f,
    .instruction_matcher = 0x1000302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data;
      cpu.Load(fields.rs1, 8, data);
      cpu.SetReg(fields.rd, data);
      // TODO(jrola): reservation set
    }
  },
  {
    .name = "SC.D",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x1800302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs2);
      cpu.Store(fields.rs1, 8, data);
      cpu.SetReg(fields.rd, 0);
      // TODO(jrola): reservation set
    }
  },
  {
    .name = "AMOSWAP.D",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x0800302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(fields.rd, data);
      cpu.SetReg(fields.rs1, cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "AMOADD.D",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x0000302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(fields.rd, data);
      cpu.SetReg(fields.rs1, data + cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "AMOXOR.D",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x2000302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(fields.rd, data);
      cpu.SetReg(fields.rs1, data ^ cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "AMOAND.D",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x6000302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(fields.rd, data);
      cpu.SetReg(fields.rs1, data & cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "AMOOR.D",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x4000302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(fields.rd, data);
      cpu.SetReg(fields.rs1, data | cpu.GetReg(fields.rs2));
    }
  },
  {
    .name = "AMOMIN.D",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0x8000302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), std::min(data, cpu.GetReg(fields.rs2)));
    }
  },
  {
    .name = "AMOMAX.D",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0xa000302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), std::max(data, cpu.GetReg(fields.rs2)));
    }
  },
  {
    .name = "AMOMINU.D",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0xc000302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), std::min(data, cpu.GetReg(fields.rs2)));
    }
  },
  {
    .name = "AMOMAXU.D",
    .format = 'R',
    .mask_field = 0xf800707f,
    .instruction_matcher = 0xe000302f,
    .execute = [](const uint32_t instruction, CPU& cpu) {
      const InstructionFields fields = parse_instruction<'R'>(instruction);
      uint64_t data = cpu.GetReg(fields.rs1);
      cpu.SetReg(cpu.GetReg(fields.rs1), std::max(data, cpu.GetReg(fields.rs2)));
    }
  }
  //RV64A
  // ----------------------------
};

const Instruction& CPU::Decode(uint32_t instruction)
{
  for(const auto& i : instructions)
  {
    if ((instruction & i.mask_field) == i.instruction_matcher)
    {
      return i;
    }
  }
  throw std::runtime_error("Instruction not supported");
}

int CPU::Execute()
{
  reg_zero = 0;   // zero out register 0, can't be made const
  const uint32_t instruction = Fetch();
  const Instruction& inst = Decode(instruction);
  try
  {
    inst.execute(instruction, *this);
  }
  catch (const std::runtime_error& e)
  {
    std::cerr << e.what() << '\n';
    throw e;
    return -1;
  }
  return 0;
}

uint32_t CPU::Fetch()
{
  uint64_t inst;
  Load(pc, 4, inst);
  pc += 4;
  return inst;
}

void CPU::Run()
{
  while(true)
  {
    Step();
  }
}

int CPU::Step()
{
  try
  {
    Execute();
  }
  catch(const std::runtime_error& e) // TODO(jrola): catch specific exceptions
  {
    std::cerr << e.what() << '\n';
    return -1;
  }
  return 0;
}

void CPU::UpdatePagingMode(uint64_t satp_val)
{
  if(xlen == 64)
  {
    mmu.SetPagingMode(mask<63, 63>(satp_val) ? PagingMode::Sv39 : PagingMode::Bare);
    mmu.SetRootPageTable(mask<0, 43>(satp_val));
  }
  else
  {
    throw std::runtime_error("Unsupported XLEN");
  }
}

// DEBUG FUNCTIONS
// TODO (jrola): move to tests directory

std::map<int, std::string> RegisterNames =
{
  {0, "zero"},
  {1, "ra"},
  {2, "sp"},
  {3, "gp"},
  {4, "tp"},
  {5, "t0"},
  {6, "t1"},
  {7, "t2"},
  {8, "s0"},
  {9, "s1"},
  {10, "a0"},
  {11, "a1"},
  {12, "a2"},
  {13, "a3"},
  {14, "a4"},
  {15, "a5"},
  {16, "a6"},
  {17, "a7"},
  {18, "s2"},
  {19, "s3"},
  {20, "s4"},
  {21, "s5"},
  {22, "s6"},
  {23, "s7"},
  {24, "s8"},
  {25, "s9"},
  {26, "s10"},
  {27, "s11"},
  {28, "t3"},
  {29, "t4"},
  {30, "t5"},
  {31, "t6"}
};

int CPU::RunInstruction(uint32_t instruction)
{
  Instruction inst = Decode(instruction);
  CPU::DumpInstruction(inst);
  inst.execute(instruction, *this);
//  CPU::DumpInstructionFields(parse_instruction<inst.format>(instruction, inst.format));
  CPU::DumpRegs();
  pc += 4;
  return 0;
}

void CPU::DumpRegs()
{
  std::cout << "--- REGISTERS ---" << std::endl;
  for (int i = 0; i < N_REG; i++)
  {
    std::cout << RegisterNames[i] << ": " << std::hex << regs[i] << std::endl;
  }
}

void CPU::DumpCsrs()
{
  for (int i = 0; i < N_CSR; i++)
  {
    std::cout << "csr" << i << ": " << std::hex << csrs[i] << std::endl;
  }
}

void CPU::DumpInstruction(const Instruction& inst)
{
  std::cout << "--- INSTRUCTION ---" << std::endl;
  std::cout << "name: " << inst.name << std::endl;
  std::cout << "format: " << inst.format << std::endl;
  std::cout << "mask_field: " << std::hex << inst.mask_field << std::endl;
  std::cout << "instruction_matcher: " << std::hex << inst.instruction_matcher << std::endl;
  std::cout << "pc: " << std::hex << pc << std::endl;
}

void CPU::DumpInstructionFields(InstructionFields inst)
{
  std::cout << "--- INSTRUCTION FIELDS ---" << std::endl;
  std::cout << "opcode: " << std::hex << inst.opcode << std::endl;
  std::cout << "rd: " << std::hex << inst.rd << std::endl;
  std::cout << "funct3: " << std::hex << inst.funct3 << std::endl;
  std::cout << "rs1: " << std::hex << inst.rs1 << std::endl;
  std::cout << "rs2: " << std::hex << inst.rs2 << std::endl;
  std::cout << "funct7: " << std::hex << inst.funct7 << std::endl;
  std::cout << "imm: " << std::hex << inst.imm << std::endl;
}
