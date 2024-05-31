#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  rtl_push(&cpu.EFLAGS);
  rtl_push(&cpu.CS);
  rtl_push(&ret_addr);
  
  vaddr_t Gate = cpu.IDTR.Base + 8 * NO;
  
  uint32_t low_addr = vaddr_read(Gate,4);
  uint32_t high_addr = vaddr_read(Gate+4,4);
  uint32_t target_addr = (low_addr & 0x0000ffff) | (high_addr & 0xffff0000);
  
  decoding.is_jmp = 1;
  decoding.jmp_eip = target_addr;
}

void dev_raise_intr() {
}
