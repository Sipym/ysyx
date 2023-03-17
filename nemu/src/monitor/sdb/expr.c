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

#include "common.h"
#include "debug.h"
#include <isa.h>
#include "sdb.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
  {"[0-9]+", TK_DIGTAL_NUM},

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
            case TK_NOTYPE: 
                    //tokens[nr_token].type = TK_NOTYPE; 
                    //nr_token++;
                    //不记录空格，将空格忽略
                    break;
            case '+': 
                    tokens[nr_token].type = '+'; 
                    nr_token++;
                    break;
            case '-':  
                    tokens[nr_token].type = '-'; 
                    nr_token++;
                    break; 
            case '/':  
                    tokens[nr_token].type = '/'; 
                    nr_token++;
                    break; 
            case '(':  
                    tokens[nr_token].type = '(';
                    nr_token++;
                    break; 
            case ')':  
                    tokens[nr_token].type = ')';
                    nr_token++;
                    break; 
            case TK_DIGTAL_NUM: 
                    tokens[nr_token].type =  TK_DIGTAL_NUM;
                    strncpy(tokens[nr_token].str, substr_start, substr_len);
                    nr_token++;
                    break;
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

unsigned int eval(int p, int q);
int check_parentheses(int p, int q);

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  printf("表达式结果为: %d \n", eval(0, nr_token-1));

  return 0;
}


struct operation {
    int op;
    int op_type;
};

/*
 * 找到主运算符所处位置
 */
int find_op(int p, int q) {
    struct operation operation[nr_token];
    int *token_type =  (int *)malloc(nr_token * sizeof(int));
    int i, op_Num = 0, tokens_Num = 0;
    for (i = p; i <= q; i ++) {  // 将tokens的p到q范围内的type这个成员提出来作为一个数组
        token_type[tokens_Num] = tokens[i].type;
        tokens_Num ++;
    }
    for (i = 0; i < tokens_Num; i ++) {  // 找到所有可能是主运算符的运算符
        if (token_type[i] == '(') {     // 把不可能是主运算符的符号变为空格' '
            while (token_type[i] != ')') {
                token_type[i] = ' ';  
                i++;
            }
            token_type[i] = ' ';
        } else if (token_type[i] == '+' || token_type[i] == '-' ||  \
                   token_type[i] == '*' || token_type[i] == '/') {
            operation[op_Num].op = i + p;
            operation[op_Num].op_type = token_type[i];
            op_Num++;
        }
    }
    for (i = op_Num; i > 0;  i --) {  //从右导左循环,同类型的运算符优先级是从左导右的
        if (operation[i-1].op_type == '+' || operation[i-1].op_type == '-')
            return operation[i-1].op;
    }
    // 进入到这里表示表达式中可能的主运算符里没有'+-'的存在,则最右边的可能符号就是主运算符
    return operation[op_Num-1].op;
}

unsigned int eval(int p, int q)
{
    Log("p = %d, q = %d\n", p, q);
    if (p > q) {
        /* Bad expression */
        Assert(0, "错误表达式");
    }
    else if (p == q) {
        /* Single token.
         * For now this token should be a number.
         * Return the value of the number.
         */
        Assert(tokens[p].type == TK_DIGTAL_NUM , "表达式错误!");
        return getstr_num(tokens[p].str, 10);
    }
    else if (check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
         * If that is the case, just throw away the parentheses.
         */
        Log("已删除表达式两边匹配的括号\n");
        return eval(p + 1, q - 1);
    }
    else {
        int op = find_op(p, q); //主运算符的位置,以0为开始的
        Assert(op < q, "产生错误"); 
        printf("op = %d\n",op);

        uint64_t val1, val2;
        val1 = eval(p, op - 1);
        val2 = eval(op + 1, q);
        
        switch (tokens[op].type) {
            case '+': return val1 + val2;
            case '-': return val1 -val2;
            case '*': return val1*val2;
            case '/': if (val2 == 0) assert(0); // 表达式错误
                      return val1/val2; // 要保证val2不等于0
            default: assert(0);
        }
    }
    return 0;
}


typedef struct Paratheses_match {
    int c;
    struct Paratheses_match *next;
}Node;

static int nr_Nodes = 0;

Node* LinkedListCreatT(int *tokens_type, int len) {
    Node *L, *r; 
    L = (Node *)malloc(sizeof(Node));
    L->next = NULL;
    r = L;
    nr_Nodes = 0;  // 避免创建多个链表时，使得nr_Nodes累加，导致程序判断错误
    for (int i = 0; i < len; i++) {
        if (tokens_type[i] == '(' || tokens_type[i] == ')' )  //取出字符串中的'(',')'存入链表中
        {
            Node *p;
            p = (Node *)malloc(sizeof(Node));
            p->c= tokens_type[i];
            r->next = p;
            r = p;
            nr_Nodes++;
        }
    }
    return L;
}

int check_parentheses(int p1, int q) {
    Log("进入到该函数\n");
    int *tokens_type = (int *)malloc(nr_token * sizeof(int));
    int tokens_Num = 0;
    for (int i = p1; i <= q; i++) {
        tokens_type[tokens_Num] = tokens[i].type;
        tokens_Num++;
    }

    Node *L = LinkedListCreatT(tokens_type, tokens_Num);

    if (L->next == NULL)  // 当表达式中不含括号是，链表只有一个头结点
        return 0;

    Node *p = L->next, *r = (Node *)malloc(sizeof(Node)); // r尾结点
    int i,j;
    for (p = L->next, i = 0, j = 0; p != NULL; p = p->next) {
        if (p->c == '(') 
            i++;
        else if (p->c == ')') 
            j++;
        if (i < j) {                 // 表达式从左到由，如果右括号数目>左括号，则表达式必定错误
            return 0;
        }
        if (p->next == NULL) {
            r = p;                  // 记住尾结点位置
        }
    }
    if ( i != j) {
        //assert(0);//非法表达式
        return 0;                 // 左右括号的数目是一定要相等的
    }

    if (nr_Nodes == 2) {           // 表示此时只有一对(),只需要判断tokens_type[0]是否等于'(' 
        if (tokens_type[0] == '(')
            return 1;
        return 0;
    }

    for (p = L->next; p != NULL; ) { //去掉所有相邻两项是()的情况，最左右匹配的圆括号将会留下
        if (p->c == '(' && p->next->c == ')') {
            p->c = 0;
            p->next->c = 0;
            p = p->next->next;
        }else 
            p = p->next;
    }
    if (L->next->c == '(' && r->c == ')') {
        if (tokens_type[0] == '(') 
            return 1;                   // 表达式正确
    }
    return 0;
}

