#include <iostream>

#include "../include/cpu.hpp"
#include "../include/memory.hpp"
int main() {
  Memory memory;
  memory.loadBinFile("./bin/kernel.gba");
  CPU cpu(memory);

  cpu.run();

  return 0;
}