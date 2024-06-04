#include "common.h"
#include "syscall.h"

extern int fs_open(const char *pathname, int flags, int mode);
extern int fs_close(int fd);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern ssize_t fs_write(int fd, const void *buf, size_t len);
extern off_t fs_lseek(int fd, off_t offset, int whence);

extern int mm_brk(uint32_t new_brk);

int sys_write(int fd,void* buf,size_t len) {
  if(fd==1||fd==2){
    for(int i=0;i<len;i++){
      _putc(((char*)buf)[i]);
    }
    return len;
  }
  else return -1;
}


_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:{
      //Log("sys_none");
      SYSCALL_ARG1(r) = 1;
      break;
    }
    case SYS_exit:{
      //Log("sys_exit");
      _halt(a[1]);
      break;
    }
    case SYS_write:{
      //Log("sys_write");
      //SYSCALL_ARG1(r) = sys_write(a[1],(void *)a[2],a[3]);
      SYSCALL_ARG1(r) = fs_write(a[1],(void *)a[2],a[3]);
      break;
    }
    case SYS_brk:{
      //Log("sys_brk");
      SYSCALL_ARG1(r) = mm_brk(a[1]);
      break;
    }
    case SYS_open:{
      //Log("sys_open");
      SYSCALL_ARG1(r) = fs_open((char *)a[1],a[2],a[3]); 
      break;
    }
    case SYS_read:{
      //Log("sys_read");
      SYSCALL_ARG1(r) = fs_read(a[1],(void *)a[2],a[3]);
      break;
    }
    case SYS_close:{
      //Log("sys_close");
      SYSCALL_ARG1(r) = fs_close(a[1]);
      break;
    }
    case SYS_lseek:{
      //Log("sys_lseek");
      SYSCALL_ARG1(r) = fs_lseek(a[1],a[2],a[3]);
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
