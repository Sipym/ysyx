/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "common.h"
#include "debug.h"
#include "memory/paddr.h"
#include "sdb.h"
#include "utils.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

// 将字符串中的数字转换为指定进制的数字  
uint64_t getstr_num(char* str,uint8_t num_system);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
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

static int cmd_si(char *args) {
    if (args == NULL) {
        cpu_exec(1);
    } else {
        uint64_t N = 0, medium_Num = 1;
        int i,j;
    // 将字符串中的数字提取出来 
        for (i = 0; i < strlen(args); i++) {  
            if ( args[i] > '9') {
                Log("si 的参数错误，含有非数字符号！");
                return 0;
            }
            for (j = 0, medium_Num = args[i] - '0'; j < (strlen(args) - i - 1); j++) {
               medium_Num *= 10; 
            }
            N += medium_Num;
        }
        cpu_exec(N);
    }
    return 0;
}

static int cmd_info(char *args) {
    if (args[0] == 'r') {
        isa_reg_display();
    } else if (args[0] == 'w') {

    } else {
        Log("命令info的参数错误");
    }
    return 0;
}

static int cmd_x(char *args) {
    uint64_t N, addr;
    char *p_num, *p;
    p_num = strtok(args, " ");
    p = strtok(NULL," ");
    N = getstr_num(p_num, 10);
    addr = getstr_num(p+2, 16);
    for (int i = 0; i < N; i++) {
        printf(ANSI_FMT("%016lx",ANSI_FG_BLUE)" 0x%016lx\n",addr,paddr_read(addr, 8));
        addr++;
    }
    return 0;
}

static int cmd_p(char *args) {
    bool *success = (bool *)malloc(sizeof(bool)) ;
    *success = true;
    expr(args, success);
    free(success);
    return 0;
}
static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *); //是一个指向参数为char*,返回整数的函数的指针
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Step over", cmd_si },
  { "info", "打印程序状态", cmd_info},
  { "x", "扫描内存", cmd_x},
  { "p", "表达式求值", cmd_p},
};

#define NR_CMD ARRLEN(cmd_table) //指令数量

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

void sdb_set_batch_mode() {
  is_batch_mode = true;
}


/* @brief:  将nemu/toool/gen-expr/input中生成的表达式传递给主函数，来验证表达式求值的功能是否正确
 */
void check_expression(void) {
  bool* success  = (bool*)malloc(sizeof(bool));
  char* filename = "/home/awjl/workspace/ysyx/ysyx-workbench/nemu/tools/gen-expr/input";//生成表达式所在目录
  FILE* fp       = fopen(filename, "r+");
  assert(fp != NULL);

  char* line = (char*)malloc(60000 * sizeof(char));
  line       = fgets(line, 60000, fp);

  while (line != NULL) {
    char* expression = strchr(line, ' ');   // 包含了换行符
    char  value_len = expression - line;
    int ex_len = strlen(expression);
    char* expression1 = (char*)malloc(60000 * sizeof(char));
    expression1       = strncpy(expression1, expression, ex_len - 1);// 删除换行符

    char* ex_value = (char*)malloc(32 * sizeof(char));
    ex_value       = strncpy(ex_value, line, value_len);
    
    printf("ex_value = %s\t", ex_value);

    expr(expression1, success);
    line = fgets(line, 60000, fp);
  }
  fclose(fp);
}
void sdb_mainloop() {

  //check_expression();  // 使用生成的表达式对表达式求值进行检查

  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1; //获取指令后的参数
    if (args >= str_end) {              //如果满足则不可能有参数  
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) { // 比较输入的指令与指令表中的哪一个相等
        if (cmd_table[i].handler(args) < 0) { return; } //当是q指令时，cmd_q的返回值为-1,从而满足条件，使得程序结束。  
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); } //没找到对应的指令
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}

// num_system: 字符串str中数字的进制
uint64_t getstr_num(char* str,uint8_t num_system)
{
    uint64_t N = 0, medium_Num = 1;
    int      i, j;
    // 将字符串中的数字提取出来
    for (i = 0; i < strlen(str); i++) {
        if (str[i] > '9') {
            Log("非法参数");
            return 0;
        }
        for (j = 0, medium_Num = str[i] - '0'; j < (strlen(str) - i - 1); j++) {
            medium_Num *= num_system;
        }
        N += medium_Num;
    }
    return N;
}
