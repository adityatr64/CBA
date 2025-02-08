#include "cpu.hpp"
#include "memory.hpp"
#include <iostream>

int main()
{
  Memory memory;
  memory.loadROM("bin/test.gba");
  CPU cpu(memory);

  cpu.run();

  return 0;
}