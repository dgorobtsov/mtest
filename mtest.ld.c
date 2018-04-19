/* Modified linker script for application needs */
/* Copyright (C) 2014-2017 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

#include "config.h"

OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv",
	      "elf32-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)
SEARCH_DIR("/home/dmitry/stage/riscv/tools/riscv32-unknown-elf/lib");

MEMORY
{
    SRAM : ORIGIN = SRAM_MAP_ADDR, LENGTH = SRAM_SIZE
    DRAM : ORIGIN = DRAM_MAP_ADDR, LENGTH = DRAM_SIZE
}

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  PROVIDE (__executable_start = SEGMENT_START("text-segment", 0x10000)); 
  
  .text           : 
  {
    . = SEGMENT_START("text-segment", 0x10000) + SIZEOF_HEADERS;
     *(.text*) 
  } > SRAM

  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);

  .rodata         : { *(.rodata*) } > SRAM
  .data           : { *(.data*) } > SRAM
  
  .sdata          :
  {
    __global_pointer$ = . + 0x800;
    *(.srodata*) *(.sdata*)
  } > SRAM
  _edata = .; PROVIDE (edata = .);
  . = .;
  __bss_start = .;
  .sbss           : { *(.sbss*) } > SRAM
  .bss            : { *(.bss*) *(COMMON) } > SRAM
  . = ALIGN(32 / 8);
  _end = .; PROVIDE (end = .);

  _dram_start = ORIGIN(DRAM);
  .dram_bss        : {} > DRAM
  _dram_end = ORIGIN(DRAM) + LENGTH(DRAM);
}

