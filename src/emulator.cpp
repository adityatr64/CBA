#include "../headers/cpu.hpp"
#include <iostream>
#include "../headers/memory.hpp"
int main()
{
  Memory memory;
  memory.loadBinFile("./bin/kernel.gba");
  CPU cpu(memory);

  cpu.run();

  return 0;
}