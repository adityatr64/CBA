OUTPUT_FORMAT("arm-none-eabi-binary")
ENTRY(_start)

MEMORY
{
  rom (rx) : ORIGIN = 0x08000000, LENGTH = 32M

}
SECTIONS
{
  .text : { *(.text*) } > rom
  .data : { *(.word*) } > rom
}