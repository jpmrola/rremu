#ifndef BASE_DEVICE_H
#define BASE_DEVICE_H

#include <cstdint>

class BaseDevice
{
  public:

    BaseDevice() = default;
    virtual ~BaseDevice() = default;

    virtual void Load(uint64_t addr, int size, uint64_t& data) = 0;
    virtual void Store(uint64_t addr, int size, uint64_t data) = 0;

    virtual constexpr uint64_t GetBaseAddr() = 0;
    virtual constexpr uint64_t GetSize() = 0;
    virtual constexpr bool IsValidAddr(uint64_t addr) = 0;

};

#endif
