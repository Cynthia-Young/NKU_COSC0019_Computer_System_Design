#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  //PA1-1

  {"si", "single-step execution", cmd_si},
  {"info", "Print information", cmd_info},
  {"p", "Evaluate the expression", cmd_p},
  {"x", "Scan memory", cmd_x},
  {"w", "Set the monitoring points", cmd_w},
  {"d", "Remove the monitoring points", cmd_d}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
    char *arg = strtok(args, " ");
    int N;
    if(arg == NULL) cpu_exec(1);
    else{
        if(sscanf(arg, "%d", &N) == 1) cpu_exec(N);
        else printf("Error parameter. \n");
    }
    return 0;
}

static int cmd_info(char *args) {
    char *arg = strtok(args, " ");
    if(arg == NULL) printf("Lack of parameter. \n");
    else {
        if(strcmp(arg, "r") == 0) {
            printf("----information of regs----\n");
            printf(" eax : 0x%08x \n", cpu.eax);
            printf(" ecx : 0x%08x \n", cpu.ecx);
            printf(" edx : 0x%08x \n", cpu.edx);
            printf(" ebx : 0x%08x \n", cpu.ebx);
            printf(" esp : 0x%08x \n", cpu.esp);
            printf(" ebp : 0x%08x \n", cpu.ebp);
            printf(" esi : 0x%08x \n", cpu.esi);
            printf(" edi : 0x%08x \n", cpu.edi);
            printf(" eip : 0x%08x \n", cpu.eip);
            printf("--------------------------\n");
        }
        else if (strcmp(arg,"w") == 0)
            info_wplist();
        else printf("Error parameter. \n");
    }
    return 0;
}

static int cmd_x(char *args)
{
    char *arg1 = strtok(args, " ");
    char *arg2 = strtok(NULL, "");
    if (arg1 == NULL||arg2 == NULL) printf("Lack of parameter. \n");  
    else {
        int N;
        vaddr_t addr;
        if(sscanf(arg1,"%d",&N) == 1){
            int i;
            bool* success = false;
            int val = expr(arg2, success);
            addr = val;
            if(N > 0){
                for (i = 0; i < N; i ++) {
                    printf("0x%06x : 0x%06x \n",addr,vaddr_read(addr,4));
                    addr = addr + 4;
                }
            }
            else printf("Error parameter. \n");
        }
        else printf("Error parameter. \n");
    } 
    return 0;
}

static int cmd_p(char *args){
    bool *success = false;
    int val = expr(args, success);
    printf("%d( 0x%06x )\n",val, val);
    return 0;
}

static int cmd_w(char *args){
    int no = set_wp(args);
    printf("Set Watch Point NO.%d Successfully!\n",no);
    return 0;
}

static int cmd_d(char *args){
    char *arg = strtok(args, " ");
    int N;
    if (arg == NULL) {
        printf("Lack of parameter. \n");
    }
    else {
        if(sscanf(arg,"%d",&N) == 1 && N >= 0){
            del_wp(N);
        }
        else printf("Error parameter. \n");
    }
    return 0; 
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
