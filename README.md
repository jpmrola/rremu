# RREMU

**WIP** 64-bit RISC-V emulator following the [xv6 RISC-V book](https://github.com/mit-pdos/xv6-riscv) hardware specifications, written in C++.

Implements the RV64I base ISA and M, A, Zicsr, privileged ISA extensions.

At the moment, it can only run in bare-metal mode, taking ELF files as input. E.g. [riscv-tests](https://github.com/riscv-software-src/riscv-tests).
The ability to boot and run xv6 is being worked on.

### Acknowledgements

This emulator was inspired and includes some logic from the following other RISC-V emulators:
* [semu](https://github.com/jserv/semu)
* [rvemu](https://github.com/d0iasm/rvemu)
* [riscv-rust](https://github.com/takahirox/riscv-rust)
