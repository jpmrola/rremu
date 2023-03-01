#include <cstdint>
#include <memory>
#include "cpu.h"

class DeviceFactory
{
  public:

    // Two configurations are supported:
    // 0: Creates a machine with the devices needed for xv6
    // 1: Creates a machine with the devices needed for running a bare-metal RISC-V ELF program
    std::unique_ptr<CPU> create_devices(const std::shared_ptr<std::vector<uint8_t>>& binary, const int config);

  private:

    std::unique_ptr<CPU> create_devices_xv6(const std::shared_ptr<std::vector<uint8_t>>& binary);
    std::unique_ptr<CPU> create_devices_bare_metal(const std::shared_ptr<std::vector<uint8_t>>& binary);

};