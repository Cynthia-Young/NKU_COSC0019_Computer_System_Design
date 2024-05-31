#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();

extern size_t fs_filesz(int fd);
extern int fs_open(const char *pathname, int flags, int mode);
extern int fs_close(int fd);
extern ssize_t fs_read(int fd, void *buf, size_t len);

uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  /*
  //v1
  ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  */

  
  //v2
  Log("opening file %s",filename);
  int fd = fs_open(filename, 0, 0);
  int file_size = fs_filesz(fd);
  fs_read(fd, DEFAULT_ENTRY, file_size);
  fs_close(fd);
  Log("successfully open file");
  

  return (uintptr_t)DEFAULT_ENTRY;
}
