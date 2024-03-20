#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

int wpNUM = 0;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp()
{
    if(wpNUM == NR_WP){
        printf("Insufficient Memory\n");
        assert(0);
    }

    int no = free_->NO;
    free_ = free_->next;
    wp_pool[no].next = head;
    head = &wp_pool[no];
    wpNUM++;
    return head;
}

void free_wp(WP* wp){
    if(wp == head){
        head = head->next;
        wp->next = free_;
    }
    else{
        WP* p;
        p = head;
        while(p->next){
            if(p->next->NO == wp->NO){
                p->next = wp->next;
                wp->next = free_;  
                break;
            }
            p = p->next;
        }
    }
    free_ = wp;
    wpNUM--;
}

int set_wp(char *e){
    WP* New_Wp = new_wp();
    bool *success = false;
    New_Wp->old_val = expr(e, success);
    New_Wp->new_val = New_Wp->old_val;
    strcpy(New_Wp->expr, e);
    printf("Set Watchpoint %d : \n",New_Wp->NO);
    printf("                   expr  : %s \n",New_Wp->expr);
    printf("                   value : 0x%06x \n",New_Wp->old_val);
    return New_Wp->NO;
}

bool del_wp(int n){
    if(wpNUM == 0){
        printf("No Watch Point\n");
        assert(0);
    }
    WP *Del_Wp = &wp_pool[n];
    free_wp(Del_Wp);
    printf("Watchpoint %d has been deleted. \n",Del_Wp->NO);
    return true;
}

void info_wplist(){
    if(wpNUM == 0){
        printf("No Watch Point\n");
        return;
    }
    WP* wp =head;
    while(wp){
        printf("NO. %d  :\n",wp->NO);
        printf("          expr:       %s \n",wp->expr);
        printf("          old value:  0x%06x \n",wp->old_val);
        printf("          new value:  0x%06x \n",wp->new_val);
        printf("\n");
        wp = wp->next;
    }
}

WP* scan_wp(){
    if(wpNUM == 0){
        return NULL;
    }
    WP* wp =head;
    bool *success = false;
    while(wp){
        if(wp->new_val != wp->old_val){
            wp->old_val=wp->new_val;
        }
        wp->new_val = expr(wp->expr,success);
        if(wp->new_val != wp->old_val) return wp;
        wp = wp->next;
    }
    return NULL;
}
