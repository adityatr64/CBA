#pragma once
#include <vector>
#include <cstdint>
#include <string>

class Memory
{
private:
  std::vector<uint8_t> bios;
  std::vector<uint8_t> wram;
  std::vector<uint8_t> iwram;
  std::vector<uint8_t> io;
  std::vector<uint8_t> palette;
  std::vector<uint8_t> vram;
  std::vector<uint8_t> oam;
  std::vector<uint8_t> rom;

public:
  Memory(); // Constructor
  // For ARM mode
  uint32_t readWord(uint32_t address) const;
  void writeWord(uint32_t address, uint32_t value);

  // For Thumb mode
  uint16_t readHalfWord(uint32_t address) const;
  void writeHalfWord(uint32_t address, uint16_t value);

  // For both ARM and Thumb modes
  uint32_t loadROM(const std::string &filename);
  size_t getROMSize() const;
  void dumpROM() const;
};