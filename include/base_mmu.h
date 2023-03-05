#include <cstdint>

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

class BaseMMU
{
  public:

    BaseMMU() = default;
    virtual ~BaseMMU() = default;

    virtual void Store(uint64_t addr, int size, uint64_t data) = 0;
    virtual void Load(uint64_t addr, int size, uint64_t& data) = 0;
    virtual void SetPagingMode(PagingMode mode) = 0;
    virtual void SetRootPageTable(uint64_t page_table) = 0;
};