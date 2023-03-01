#include <cstdint>
#include <vector>
#include <memory>

std::vector<uint8_t> LoadELF64(const std::string& file, uint64_t& entry_point);