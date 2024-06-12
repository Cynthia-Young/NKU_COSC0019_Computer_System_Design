#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

extern void switch_game();

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  if(key == _KEY_NONE){
    unsigned long event_time = _uptime();
    return snprintf(buf, len, "t %d\n", event_time) - 1;
  }
  else{
    char kd_ku;
    if(key & 0x8000) {kd_ku = 'd'; key = key ^ 0x8000;}
    else {
      kd_ku = 'u';
      if(key == _KEY_F12) {
        switch_game();
        Log("F12--switch game!");
      }
    }
    return snprintf(buf, len, "k%c %s\n", kd_ku, keyname[key]) - 1;
  }
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  off_t Size = offset/sizeof(uint32_t);
  _draw_rect(buf, Size % _screen.width, Size / _screen.width, len / sizeof(uint32_t), 1);  
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH: %d\nHEIGHT: %d", _screen.width, _screen.height);
}
