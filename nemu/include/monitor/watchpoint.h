#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[1024];
  int new_val;
  int old_val;
  bool just_set;
} WP;


int set_wp(char *e);
bool del_wp(int n);
void info_wplist();
WP* scan_wp();
#endif
