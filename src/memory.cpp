#include "memory.hpp"
#include "cpu.hpp"
#include <iostream>
#include <fstream>

// i Fucking hate little edian
// I also hate myself

Memory::Memory() : bios(BIOS_SIZE),
                   wram(WRAM_SIZE),
                   iwram(IWRAM_SIZE),
                   io(IO_END - IO_START + 1),
                   palette(PALETTE_SIZE),
                   vram(VRAM_SIZE),
                   oam(OAM_SIZE),
                   rom() // ROM size will be determined when loading
{
}

uint32_t Memory::readWord(uint32_t address) const
{
  // std::cout << "readWord: Address: 0x" << std::hex << address << std::endl;

  size_t regionSize = 0;
  const std::vector<uint8_t> *region = nullptr; // Pointer to the correct vector

  // Step 1. Determine the memory region and its size:

  size_t offset = 0;

  if (address >= BIOS_START && address <= BIOS_END)
  {
    regionSize = BIOS_SIZE;
    region = &bios;
    offset = address - BIOS_START;
  }
  else if (address >= WRAM_START && address <= WRAM_END)
  {
    regionSize = WRAM_SIZE;
    region = &wram;
    offset = address - WRAM_START;
  }
  else if (address >= IWRAM_START && address <= IWRAM_END)
  {
    regionSize = IWRAM_SIZE;
    region = &iwram;
    offset = address - IWRAM_START;
  }
  else if (address >= IO_START && address <= IO_END)
  {
    regionSize = IO_END - IO_START + 1;
    region = &io;
    offset = address - IO_START;
  }
  else if (address >= PALETTE_START && address <= PALETTE_END)
  {
    regionSize = PALETTE_SIZE;
    region = &palette;
    offset = address - PALETTE_START;
  }
  else if (address >= VRAM_START && address <= VRAM_END)
  {
    regionSize = VRAM_SIZE;
    region = &vram;
    offset = address - VRAM_START;
  }
  else if (address >= OAM_START && address <= OAM_END)
  {
    regionSize = OAM_SIZE;
    region = &oam;
    offset = address - OAM_START;
  }
  else if (address >= ROM_START && address <= ROM_END)
  {
    regionSize = rom.size();
    region = &rom;
    offset = address - ROM_START;
  }
  else
  {
    std::cerr << "readWord: Invalid address: 0x" << std::hex << address << std::endl;
    throw std::out_of_range("Memory::readWord: Invalid address");
  }

  // Hopefully this will never happen

  if (offset + 3 >= regionSize)
  {
    std::cerr << "readWord: OUT OF RANGE! Offset: 0x" << std::hex << offset << " is beyond region size: 0x" << std::hex << regionSize << std::endl;
    throw std::out_of_range("Memory::readWord: Address out of range");
  }

  // Step 2. Read the 4 bytes starting from the offset
  uint8_t byte0 = (*region)[offset];
  uint8_t byte1 = (*region)[offset + 1];
  uint8_t byte2 = (*region)[offset + 2];
  uint8_t byte3 = (*region)[offset + 3];

  // std::cout << "Bytes: 0x" << std::hex << (int)byte0 << " 0x" << (int)byte1 << " 0x" << (int)byte2 << " 0x" << (int)byte3 << std::endl;

  uint32_t word = (uint32_t)(byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24));

  // std::cout << "Word: 0x" << std::hex << word << std::endl;

  return word;
}

void Memory::writeWord(uint32_t address, uint32_t value)
{
  std::cout << "writeWord: Address: 0x" << std::hex << address << " Value: 0x" << std::hex << value << std::endl;

  // 1. Determine the memory region and its size:
  size_t regionSize = 0;
  std::vector<uint8_t> *region = nullptr; // Pointer to the correct vector

  size_t offset = 0;
  if (address >= BIOS_START && address <= BIOS_END)
  {
    regionSize = BIOS_SIZE;
    region = &bios;
    offset = address - BIOS_START;
  }
  else if (address >= WRAM_START && address <= WRAM_END)
  {
    regionSize = WRAM_SIZE;
    region = &wram;
    offset = address - WRAM_START;
  }
  else if (address >= IWRAM_START && address <= IWRAM_END)
  {
    regionSize = IWRAM_SIZE;
    region = &iwram;
    offset = address - IWRAM_START;
  }
  else if (address >= IO_START && address <= IO_END)
  {
    regionSize = IO_END - IO_START + 1;
    region = &io;
    offset = address - IO_START;
  }
  else if (address >= PALETTE_START && address <= PALETTE_END)
  {
    regionSize = PALETTE_SIZE;
    region = &palette;
    offset = address - PALETTE_START;
  }
  else if (address >= VRAM_START && address <= VRAM_END)
  {
    regionSize = VRAM_SIZE;
    region = &vram;
    offset = address - VRAM_START;
  }
  else if (address >= OAM_START && address <= OAM_END)
  {
    regionSize = OAM_SIZE;
    region = &oam;
    offset = address - OAM_START;
  }
  else if (address >= ROM_START && address <= ROM_END)
  {
    regionSize = rom.size();
    region = &rom;
    offset = address - ROM_START;
  }
  else
  {
    std::cerr << "writeWord: Invalid address: 0x" << std::hex << address << std::endl;
    throw std::out_of_range("Memory::writeWord: Invalid address");
  }
  // Hopefully this will never happen
  if (offset + 3 >= regionSize)
  {
    std::cerr << "writeWord: OUT OF RANGE! Offset: 0x" << std::hex << offset << " is beyond region size: 0x" << std::hex << regionSize << std::endl;
    throw std::out_of_range("Memory::writeWord: Address out of range");
  }

  (*region)[offset] = value & 0xFF;
  (*region)[offset + 1] = (value >> 8) & 0xFF;
  (*region)[offset + 2] = (value >> 16) & 0xFF;
  (*region)[offset + 3] = (value >> 24) & 0xFF;
}

uint32_t Memory::loadROM(const std::string &filename)
{
  std::cout << "Loading ROM file: " << filename << std::endl;
  // Open the file in binary mode
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open())
  {
    throw std::runtime_error("Memory::loadROM: Failed to open file");
  }

  // Read the entire ROM into memory
  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  // Resize the 'rom' vector and read the file data
  rom.resize(size);
  file.read(reinterpret_cast<char *>(&rom[0]), size);
  file.close();

  // Read the entry point address from the ROM header (offset 0xAC)
  uint32_t entryPoint = rom[0xAC] | (rom[0xAD] << 8) | (rom[0xAE] << 16) | (rom[0xAF] << 24);
  // std::cout << "Entry point: 0x" << std::hex << entryPoint << std::endl;

  return entryPoint;
}
size_t Memory::getROMSize() const
{
  return rom.size();
}

void Memory::dumpROM() const
{
  std::cout << "Dumping ROM:" << std::endl;
  for (size_t i = 0; i < rom.size(); i += 16)
  {
    std::cout << "0x" << std::hex << i << ": ";
    for (size_t j = 0; j < 16; ++j)
    {
      if (i + j < rom.size())
      {
        std::cout << std::hex << (int)rom[i + j] << " ";
      }
      else
      {
        std::cout << "   ";
      }
    }
    std::cout << std::endl;
  }
}

uint16_t Memory::readHalfWord(uint32_t address) const
{
  // std::cout << "readHalfWord: Address: 0x" << std::hex << address << std::endl;

  size_t regionSize = 0;
  const std::vector<uint8_t> *region = nullptr; // Pointer to the correct vector

  // Step 1. Determine the memory region and its size:

  size_t offset = 0;

  if (address >= BIOS_START && address <= BIOS_END)
  {
    regionSize = BIOS_SIZE;
    region = &bios;
    offset = address - BIOS_START;
  }
  else if (address >= WRAM_START && address <= WRAM_END)
  {
    regionSize = WRAM_SIZE;
    region = &wram;
    offset = address - WRAM_START;
  }
  else if (address >= IWRAM_START && address <= IWRAM_END)
  {
    regionSize = IWRAM_SIZE;
    region = &iwram;
    offset = address - IWRAM_START;
  }
  else if (address >= IO_START && address <= IO_END)
  {
    regionSize = IO_END - IO_START + 1;
    region = &io;
    offset = address - IO_START;
  }
  else if (address >= PALETTE_START && address <= PALETTE_END)
  {
    regionSize = PALETTE_SIZE;
    region = &palette;
    offset = address - PALETTE_START;
  }
  else if (address >= VRAM_START && address <= VRAM_END)
  {
    regionSize = VRAM_SIZE;
    region = &vram;
    offset = address - VRAM_START;
  }
  else if (address >= OAM_START && address <= OAM_END)
  {
    regionSize = OAM_SIZE;
    region = &oam;
    offset = address - OAM_START;
  }
  else if (address >= ROM_START && address <= ROM_END)
  {
    regionSize = rom.size();
    region = &rom;
    offset = address - ROM_START;
  }
  else
  {
    std::cerr << "readHalfWord: Invalid address: 0x" << std::hex << address << std::endl;
    throw std::out_of_range("Memory::readHalfWord: Invalid address");
  }

  // Hopefully this will never happen

  if (offset + 1 >= regionSize)
  {
    std::cerr << "readHalfWord: OUT OF RANGE! Offset: 0x" << std::hex << offset << " is beyond region size: 0x" << std::hex << regionSize << std::endl;
    throw std::out_of_range("Memory::readHalfWord: Address out of range");
  }

  // Step 2. Read the 2 bytes starting from the offset
  uint8_t byte0 = (*region)[offset];
  uint8_t byte1 = (*region)[offset + 1];

  // std::cout << "Bytes: 0x" << std::hex << (int)byte0 << " 0x" << std::hex << (int)byte1 << std::endl;

  uint16_t halfWord = (uint16_t)(byte0 | (byte1 << 8));

  // std::cout << "HalfWord: 0x" << std::hex << halfWord << std::endl;

  return halfWord;
}

void Memory::writeHalfWord(uint32_t address, uint16_t value)
{
  std::cout << "writeHalfWord: Address: 0x" << std::hex << address << " Value: 0x" << std::hex << value << std::endl;

  // 1. Determine the memory region and its size:

  size_t regionSize = 0;
  std::vector<uint8_t> *region = nullptr; // Pointer to the correct vector

  size_t offset = 0;

  if (address >= BIOS_START && address <= BIOS_END)
  {
    regionSize = BIOS_SIZE;
    region = &bios;
    offset = address - BIOS_START;
  }
  else if (address >= WRAM_START && address <= WRAM_END)
  {
    regionSize = WRAM_SIZE;
    region = &wram;
    offset = address - WRAM_START;
  }
  else if (address >= IWRAM_START && address <= IWRAM_END)
  {
    regionSize = IWRAM_SIZE;
    region = &iwram;
    offset = address - IWRAM_START;
  }
  else if (address >= IO_START && address <= IO_END)
  {
    regionSize = IO_END - IO_START + 1;
    region = &io;
    offset = address - IO_START;
  }
  else if (address >= PALETTE_START && address <= PALETTE_END)
  {
    regionSize = PALETTE_SIZE;
    region = &palette;
    offset = address - PALETTE_START;
  }
  else if (address >= VRAM_START && address <= VRAM_END)
  {
    regionSize = VRAM_SIZE;
    region = &vram;
    offset = address - VRAM_START;
  }
  else if (address >= OAM_START && address <= OAM_END)
  {
    regionSize = OAM_SIZE;
    region = &oam;
    offset = address - OAM_START;
  }
  else if (address >= ROM_START && address <= ROM_END)
  {
    regionSize = rom.size();
    region = &rom;
    offset = address - ROM_START;
  }
  else
  {
    std::cerr << "writeHalfWord: Invalid address: 0x" << std::hex << address << std::endl;
    throw std::out_of_range("Memory::writeHalfWord: Invalid address");
  }

  // Hopefully this will never happen
  if (offset + 1 >= regionSize)
  {
    std::cerr << "writeHalfWord: OUT OF RANGE! Offset: 0x" << std::hex << offset << " is beyond region size: 0x" << std::hex << regionSize << std::endl;
    throw std::out_of_range("Memory::writeHalfWord: Address out of range");
  }

  (*region)[offset] = value & 0xFF;
  (*region)[offset + 1] = (value >> 8) & 0xFF;

  std::cout << "writeHalfWord: Value written: 0x" << std::hex << (int)value << std::endl;
}
