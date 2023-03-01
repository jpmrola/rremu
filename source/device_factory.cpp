#include <iostream>
#include "device_factory.h"
#include "elf_parser.h"
#include "config.h"
#include "mmu.h"
#include "ram.h"
#include "virtio.h"
#include "uart.h"
#include "clint.h"
#include "plic.h"

std::unique_ptr<CPU> DeviceFactory::create_devices(const std::shared_ptr<std::vector<uint8_t>>& binary, const int config)
{
  if(config == 0)
  {
    return create_devices_xv6(binary);
  }
  else if(config == 1)
  {
    return create_devices_bare_metal(binary);
  }
  else
  {
    std::cerr << "Invalid configuration" << std::endl;
    return nullptr;
  }
}

std::unique_ptr<CPU> DeviceFactory::create_devices_xv6(const std::shared_ptr<std::vector<uint8_t>>& binary)
{
  // Create devices and attach them to the mmu
  auto mmu = std::make_unique<MMU>();
  mmu->add_device(std::make_unique<RAM<KERNBASE, MEMORY_SIZE>>(binary));
  mmu->add_device(std::make_unique<UART<UART_BASE, UART_SIZE>>());
  mmu->add_device(std::make_unique<VIRTIO<VIRTIO_BASE, VIRTIO_SIZE>>());
  mmu->add_device(std::make_unique<CLINT<CLINT_BASE, CLINT_SIZE>>());
  mmu->add_device(std::make_unique<PLIC<PLIC_BASE, PLIC_SIZE>>());
  // create cpu and attach mmu
  auto cpu = std::make_unique<CPU>(std::move(mmu));
  return cpu;
}

std::unique_ptr<CPU> DeviceFactory::create_devices_bare_metal(const std::shared_ptr<std::vector<uint8_t>>& binary)
{
    // Create devices and attach them to the mmu
    auto mmu = std::make_unique<MMU>();
    mmu->add_device(std::make_unique<RAM<0, MEMORY_SIZE>>(binary));
    // create cpu and attach mmu
    auto cpu = std::make_unique<CPU>(std::move(mmu));
    return cpu;
}
