#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

// RISC-V constants

constexpr int N_REG = 32;
constexpr int N_CSR = 4096;

// xv6 constants

// Memory constants
constexpr uint64_t KERNBASE = 0x80000000;
constexpr uint64_t MEMORY_SIZE = (1024 * 1024 * 128); // 128 MB
constexpr int PAGE_SIZE = 4096;

// UART constants
constexpr uint64_t UART_BASE = 0x10000000;
constexpr uint64_t UART_SIZE = 0x1000;

// VirtIO constants
constexpr uint64_t VIRTIO_BASE = 0x10001000;
constexpr uint64_t VIRTIO_SIZE = 0x1000;

// CLINT constants
constexpr uint64_t CLINT_BASE = 0x2000000;
constexpr uint64_t CLINT_SIZE = 0x10000;

// PLIC constants
constexpr uint64_t PLIC_BASE = 0xc000000;
constexpr uint64_t PLIC_SIZE = 0x4000000;

#endif
