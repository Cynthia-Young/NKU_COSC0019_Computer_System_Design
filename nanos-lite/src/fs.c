#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = (_screen.width * _screen.height) * sizeof(uint32_t);
}

size_t fs_filesz(int fd){
  assert(fd >= 0 && fd < NR_FILES);
  return file_table[fd].size;
}

void ramdisk_read(void *, uint32_t, uint32_t);
void ramdisk_write(const void *, uint32_t, uint32_t);
void dispinfo_read(void *buf, off_t offset, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);
size_t events_read(void *buf, size_t len);


int fs_open(const char *pathname, int flags, int mode){
  int i;
  for(i = 0; i < NR_FILES; i++){
    if (strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  Log("pathname:%s\n",pathname);
  panic("File not exists");
  return -1;
}

int fs_close(int fd){
  return 0;
}

ssize_t fs_read(int fd, void *buf, size_t len){
  if(fd == FD_EVENTS) return events_read(buf, len);
  int read_len = file_table[fd].size - file_table[fd].open_offset;   
  if(read_len > len)  read_len = len;
  if(fd == FD_DISPINFO) 
    dispinfo_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, read_len);
  else
    ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, read_len);
  file_table[fd].open_offset += read_len;
  return read_len;

}

ssize_t fs_write(int fd, const void *buf, size_t len){ 

  int write_len = file_table[fd].size - file_table[fd].open_offset;
  if(write_len > len)  write_len = len;
  switch(fd){
    case FD_EVENTS: 
      return len;
    case FD_STDOUT:
    case FD_STDERR:
     for (int i = 0; i < len; i ++) _putc( ((char *)buf)[i] );
     return len;
    case FD_FB:
      fb_write(buf, file_table[fd].open_offset, write_len);
      break;    
    default:
      ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, write_len);
      break;       
  }

  file_table[fd].open_offset += write_len;
  return write_len;  
}

off_t fs_lseek(int fd, off_t offset, int whence){
  off_t src_offset, dst_offset;
  src_offset = file_table[fd].open_offset;
  dst_offset = 0;
  switch(whence){
    case SEEK_SET:
      dst_offset = offset;
      break;
    case SEEK_CUR:
      dst_offset = src_offset + offset;
      break;
    case SEEK_END:
      dst_offset = fs_filesz(fd) + offset;
      break;
    default:
      assert(0);
  }
  if(dst_offset < 0) dst_offset = 0;
  else if(dst_offset > fs_filesz(fd)) dst_offset = fs_filesz(fd);
  
  file_table[fd].open_offset = dst_offset;
  
  return dst_offset;

}


