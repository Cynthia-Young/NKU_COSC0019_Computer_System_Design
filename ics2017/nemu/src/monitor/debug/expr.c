#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_PLUS = 255,
  TK_SUB = 254,
  TK_MUL = 253,
  TK_DIV = 252,
  TK_MOD = 251,
  TK_GE = 250,
  TK_GT = 249,
  TK_LE = 248,
  TK_LT = 247,
  TK_NEQ = 246,
  TK_EQ = 245,
  TK_AND = 244,
  TK_OR = 243,
  TK_DEC = 242,
  TK_HEX = 241,
  TK_REG = 240,
  TK_LBRACE = 239,
  TK_RBRACE = 238,
  TK_NEG = 237,
  TK_DEREF = 236,
  TK_LS = 235,
  TK_RS = 234

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},                        // spaces
  {"\\+", TK_PLUS},                         // plus
  {"-", TK_SUB},                          // sub
  {"\\*", TK_MUL},         		    // multi
  {"\\/", TK_DIV},         		    // divide
  {"\\%", TK_MOD},        		    // remaindedr
  {"\\(", TK_LBRACE},       		    // left
  {"\\)", TK_RBRACE},    		    // right
  {"<<",TK_LS},                             // left shift
  {">>",TK_RS},                             // right shift
  {">=", TK_GE},         		    // greater than or equal
  {">", TK_GT},       		    // greater than
  {"<=", TK_LE},        		    // less than or equal
  {"<", TK_LT},        		    // less than
  {"!=", TK_NEQ},       		    // not equal
  {"==", TK_EQ},         		    // equal
  {"&&", TK_AND},         		    // and
  {"\\|\\|", TK_OR},        		    // or
  {"[1-9][0-9]*",TK_DEC},        	    // dec number
  {"0[xX][0-9a-fA-F]+", TK_HEX},            // hex number
  {"\\$(eax|ecx|edx|ebx|ebp|esi|edi|esp|eip)", TK_REG} //registerr
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
	if(substr_len > 32){
	  printf("Bufferr overflows\n");
	  assert(0);
	}
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        int j;
        switch (rules[i].token_type) {
	  case TK_NOTYPE: break;

	  case TK_DEC:  
	    tokens[nr_token].type = rules[i].token_type;
	    for(j=0;j < substr_len; j++)
		tokens[nr_token].str[j] = e[position - substr_len + j];
	    tokens[nr_token].str[j] = '\0';
	    nr_token++;
	    break;

	  case TK_HEX:  
	    tokens[nr_token].type = rules[i].token_type;
	    for(j=0;j < substr_len; j++)
		tokens[nr_token].str[j] = e[position - substr_len + j];
	    tokens[nr_token].str[j] = '\0';
	    nr_token++;
	    break;

	  case TK_REG:  
	    tokens[nr_token].type = rules[i].token_type;
	    for(j=0;j < substr_len; j++)
		tokens[nr_token].str[j] = e[position - substr_len + j];
	    tokens[nr_token].str[j] = '\0';
	    nr_token++;
	    break;

          default: 
	    tokens[nr_token].type = rules[i].token_type;
	    tokens[nr_token].str[0] = e[position - substr_len];
	    tokens[nr_token].str[1] = '\0';
	    nr_token++;
	    break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}
int type_prior(int token_type)
{

  if(token_type == TK_MUL || token_type == TK_DIV || token_type == TK_MOD) return 1;
  else if(token_type == TK_PLUS || token_type == TK_SUB) return 2;
  else if(token_type == TK_LS || token_type == TK_RS) return 3;
  else if(token_type == TK_GT || token_type == TK_GE || token_type == TK_LT || token_type == TK_LE) return 4;
  else if(token_type == TK_EQ || token_type == TK_NEQ) return 5;
  else if(token_type == TK_AND) return 6;
  else if(token_type == TK_OR) return 7;
  else return -1;
}


int dominant_op(int p, int q) {
  int cur_op = -1;
  int op_position = -1;
  int count = 0;
  for(int i=p;i<=q;i++){
    if(tokens[i].type == TK_LBRACE) count++;
    else if(tokens[i].type == TK_RBRACE) count--;
    else if(tokens[i].type == TK_DEC || tokens[i].type == TK_HEX ||tokens[i].type == TK_REG || count != 0) continue; 
    else if(count == 0){
      int token_prior = type_prior(tokens[i].type);
      int curop_prior = type_prior(cur_op);
      if(token_prior>0) {
        if(token_prior >= curop_prior){
          cur_op = tokens[i].type;
          op_position = i;
        }
      }
    }
  }
  return op_position;
}




bool check_paretheses(int p, int q) {
  if(!(tokens[p].type == TK_LBRACE && tokens[q].type == TK_RBRACE)) return false;
  int num = 0;
  for( int i=p+1;i<q;i++){
    if(tokens[i].type == TK_LBRACE) num++;
    else if(tokens[i].type == TK_RBRACE) num--;
    if(num<0) return false;
  }
  if(num == 0) return true;
  else return false;
}







uint32_t eval(int p, int q) {
  int val = 0;
  if (p>q)
  {
    printf("Wrong Expression\n");
    assert(0);
  }
  else if(p == q)
  {
    
    if(tokens[p].type == TK_DEC) 
      sscanf(tokens[p].str, "%d", &val);
    else if(tokens[p].type == TK_HEX) 
      sscanf(tokens[p].str, "%x", &val);
    else if(tokens[p].type == TK_REG) 
    {
      if(strcmp(tokens[p].str, "$eax") == 0) val = cpu.eax;
      else if(strcmp(tokens[p].str, "$edx") == 0) val = cpu.edx;
      else if(strcmp(tokens[p].str, "$ecx") == 0) val = cpu.ecx;
      else if(strcmp(tokens[p].str, "$ebx") == 0) val = cpu.ebx;
      else if(strcmp(tokens[p].str, "$ebp") == 0) val = cpu.ebp;
      else if(strcmp(tokens[p].str, "$esi") == 0) val = cpu.esi;
      else if(strcmp(tokens[p].str, "$edi") == 0) val = cpu.edi;
      else if(strcmp(tokens[p].str, "$esp") == 0) val = cpu.esp;
      else if(strcmp(tokens[p].str, "$eip") == 0) val = cpu.eip;
    }
    else 
    {
      printf("Wrong Expression\n");
      assert(0);
    }
    
    return val;
  }
  else if(check_paretheses(p,q) == true)
    return eval(p+1,q-1);
  else
  {
    int op = dominant_op(p,q);
    if(op == -1)
    {
      int k = p;
      while((tokens[k].type == TK_NEG || tokens[k].type == TK_DEREF)) k++;
      val = eval(k,q);
      for(int i=k-1; i >= p; i--)
      {
        if(tokens[i].type == TK_NEG)
        {
          val = (-1)*val;
        }
        else if(tokens[i].type == TK_DEREF)
        {          
          val = vaddr_read(val,4);
        }
        return val;
      }
    }
    int val1 = eval(p,op-1);
    int val2 = eval(op+1,q);
    switch(tokens[op].type)
    {
      case(TK_PLUS): return val1+val2;
      case(TK_SUB): return val1-val2;
      case(TK_MUL): return val1*val2;
      case(TK_DIV): return val1/val2;
      case(TK_MOD): return val1%val2;
      case(TK_LS): return val1<<val2;
      case(TK_RS): return val1>>val2;
      case(TK_GE): return val1>=val2;
      case(TK_GT): return val1>val2;
      case(TK_LE): return val1<=val2;
      case(TK_LT): return val1<val;
      case(TK_EQ): return val1==val2;
      case(TK_NEQ): return val1!=val2;
      case(TK_AND): return val1&&val2;
      case(TK_OR): return val1||val2;
      default: assert(0);
    }
  }
}




bool legal_check(){
  int num = 0;
  for(int i=0; i < nr_token; i++){
    if(tokens[i].type == TK_LBRACE)  num++;
    else if(tokens[i].type == TK_RBRACE) num--;
    if(num<0) return false;
  }

  if(num != 0) return false;
  return true;
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */

  if(legal_check() == false) {
    printf("Wrong Expression\n");
    assert(0);
  }

  for(int i = 0; i < nr_token; i ++) {
    if(tokens[i].type == TK_SUB && (i==0 || (tokens[i-1].type!=TK_DEC && tokens[i-1].type!=TK_HEX && tokens[i-1].type!=TK_RBRACE && tokens[i-1].type!=TK_REG) ))
      tokens[i].type = TK_NEG;
    else if(tokens[i].type == TK_MUL && (i==0 || (tokens[i-1].type!=TK_DEC && tokens[i-1].type!=TK_HEX && tokens[i-1].type!=TK_RBRACE && tokens[i-1].type!=TK_REG) ))
      tokens[i].type = TK_DEREF;
  }
  return eval(0, nr_token-1);
}
