#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  if(is_mmio(addr) == -1) 
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  else 
    return mmio_read(addr, len, is_mmio(addr));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  if(is_mmio(addr) == -1)  
    memcpy(guest_to_host(addr), &data, len);
  else
    mmio_write(addr, len, data, is_mmio(addr));
}

paddr_t page_translate(vaddr_t addr, bool is_write) {
  if(!cpu.cr0.paging) return addr;
  paddr_t paddr = addr;
  
  PDE pde, *pgdir;
  pgdir = (PDE *)(intptr_t)(cpu.cr3.page_directory_base << 12);
  pde.val = paddr_read((intptr_t)&pgdir[(addr >> 22) & 0x3ff], 4);
  assert(pde.present);
  pde.accessed = 1;
  
  PTE pte, *pgtab;
  pgtab = (PTE *)(intptr_t)(pde.page_frame << 12);
  pte.val = paddr_read((intptr_t)&pgtab[(addr >> 12) & 0x3ff], 4);
  assert(pte.present);
  pte.accessed = 1;
  
  if(is_write) pte.dirty=1;
  paddr = (pte.page_frame << 12) | (addr & PAGE_MASK);
  
  return paddr;
}

bool CrossBoundary(vaddr_t addr, int len){
  vaddr_t end_addr=addr+len-1;
  if((end_addr&(~PAGE_MASK))!=(addr&(~PAGE_MASK)))
    return true;
  return false;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  paddr_t paddr;
  if (CrossBoundary(addr, len)) {
    /* data cross the page boundary */
    //assert(0);
    union{
      uint8_t bytes[4];
      uint32_t dword;
    } data = {0};
    for(int i=0; i < len; i++){
      paddr =  page_translate(addr+i, false);
      data.bytes[i] = (uint8_t)paddr_read(paddr, 1);
    }
    return data.dword;
  } 
  else {
    paddr = page_translate(addr, false);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  paddr_t paddr;
  if (CrossBoundary(addr, len)) {
    /* data cross the page boundary */
    //assert(0);
    for(int i=0; i < len; i++){
      paddr =  page_translate(addr, true);
      paddr_write(paddr, 1, data);
      data >>= 8;
      addr++;
    }
  } 
  else {
    paddr = page_translate(addr, true);
    paddr_write(paddr, len, data);
  }
}
