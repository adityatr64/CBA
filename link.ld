OUTPUT_FORMAT("arm-none-eabi-binary")
ENTRY(_start)

MEMORY
{
  rom (rx) : ORIGIN = 0x08000000, LENGTH = 32M
		iwram	: ORIGIN = 0x03000000, LENGTH = 32K

}
SECTIONS
{
  .text : { *(.text*) } > rom
  .data : { *(.word*) } > rom
}