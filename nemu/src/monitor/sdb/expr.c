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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_DIGTAL_NUM = 254,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus, 这里是两个\\的原因是因为c中要表示\符号就是用的"\\"
  {"==", TK_EQ},        // equal
  {"-", '-'},           // 减号
  {"/", '/'},           // 除号
  {"\\(", '('},         // 右括号 
  {"\\)", ')'},         // 左括号 
  {"*[0-9]", TK_DIGTAL_NUM},

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {}; // 指向了各个token类型的模式缓冲区存储区域

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED); 
    if (ret != 0) { // regcomp返回0时表示编译正确
      regerror(ret, &re[i], error_msg, 128);// 将错误代码转换为错误信息字符串
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {}; //用来顺序记录已识别了的token  
static int nr_token __attribute__((used))  = 0; //记录了已识别来的token的数目

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {  /*因为要顺序识别token，所以在保证字符串中有匹配项的前提下，
                                                                                       还要保证是匹配项在字符串的首位*/
          
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len; // 剔除已匹配了的token

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
            case TK_NOTYPE: tokens[nr_token++].type = TK_NOTYPE;
            case '+': {tokens[nr_token++].type = '+'; break;}
            case '-': {tokens[nr_token++].type = '-'; break;}
            case '/': {tokens[nr_token++].type = '/'; break;}
            case '(': {tokens[nr_token++].type = '('; break;}
            case ')': {tokens[nr_token++].type = ')'; break;}
            case TK_DIGTAL_NUM: {
                      tokens[nr_token].type =  TK_DIGTAL_NUM;
                      strncpy(tokens[nr_token].str, e + position, substr_len);
                      nr_token++;
                      break;}
            default: TODO();
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


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
