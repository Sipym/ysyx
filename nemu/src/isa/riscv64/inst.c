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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <stdint.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S,
  TYPE_N, // none
  TYPE_J,
  TYPE_B,
  TYPE_R,
  TYPE_IS, //I类型的一种变种
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = SEXT((BITS(i,30,21) + (BITS(i,20,20) << 10) + (BITS(i,19,12) << 11) + (BITS(i,31,31) << 19)) << 1,20);} while(0)
#define immB() do { *imm = SEXT((BITS(i,11,8) + (BITS(i,30,25) << 4) + (BITS(i,7,7) << 10) + (BITS(i,31,31) << 11)) << 1,12);} while(0)
#define immIS() do { *imm = SEXT(BITS(i, 25, 20), 6); } while(0)


#define sext32(x) ((int64_t)(int32_t)(x))
#define zext32(x) ((uint64_t)(uint32_t)(x))

/* @param: imm: 立即数 
 * @param: src1, src2: 两个源操作数
 * @param: dest: 目的操作数
 */
static void decode_operand(Decode *s, int *dest, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rd  = BITS(i, 11, 7);
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *dest = rd;
  switch (type) {
    case TYPE_I:  src1R();          immI();   break;
    case TYPE_IS: src1R();          immIS();   break;
    case TYPE_U:                    immU();   break;
    case TYPE_S:  src1R(); src2R(); immS();   break;
    case TYPE_J:                    immJ();   break;
    case TYPE_B:  src1R(); src2R(); immB();   break;
    case TYPE_R:  src1R(); src2R();           break;


  }
}

static int decode_exec(Decode *s) {
  int dest = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;
  //Mw(0xa00003f8,1,0x41);
#define INSTPAT_INST(s) ((s)->isa.inst.val)  
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &dest, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}
  INSTPAT_START();
  //INSTPAT用来定义一条模式匹配规则
  //INSTPAT(模式字符串, 指令名称, 指令类型, 指令执行操作);
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(dest) = s->pc + imm);  

  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(dest) = imm);  
  //读取存储器的值到寄存器,???:代表的是宽度
  INSTPAT("??????? ????? ????? 011 ????? 00000 11", ld     , I, R(dest) = Mr(src1 + imm, 8));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(dest) = SEXT(Mr(src1 + imm, 4),32));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(dest) = Mr(src1 + imm, 1));


  
  //将rs1的值写入存储器
  INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd     , S, Mw(src1 + imm, 8, src2));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0

  // li和addi的区别: li的sr1相当于0,而r(0)恒为0
  INSTPAT("??????? ????? 00000 000 ????? 00100 11", li     , I, R(dest) = imm);
  //相当于 addi rd,rs1,0
  INSTPAT("0000000 00000 ????? 000 ????? 00100 11", mv     , I, R(dest) = src1);
  // 加立即数
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(dest) = src1 + imm);
  // sext.w相当于addiw rd,rs1,0
  INSTPAT("0000000 00000 ????? 000 ????? 00110 11", sext.w , I, R(dest) = SEXT(src1,32));
  // addiw与addi的区别,addiw将结果阶段为32位
  INSTPAT("??????? ????? ????? 000 ????? 00110 11", addiw  , I, R(dest) = SEXT(src1 + imm,32));

  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(dest) = src1 + src2);
  INSTPAT("0000000 ????? ????? 000 ????? 01110 11", addw   , R, R(dest) = SEXT(src1 + src2,32));

  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(dest) = src1 - src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01110 11", subw   , R, R(dest) = SEXT(src1 - src2,32));
  //相当于sltiu rd,rs1,1, 功能:小于1则置1,否则为0
  INSTPAT("0000000 00001 ????? 011 ????? 00100 11", seqz   , I, R(dest) = (src1 == 0));
  // 乘法
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(dest) = src1 * src2);
  INSTPAT("0000001 ????? ????? 000 ????? 01110 11", mulw   , R, R(dest) = SEXT(src1 * src2,32));
  //除法
  INSTPAT("0000001 ????? ????? 100 ????? 01110 11", divw   , R, R(dest) = SEXT(SEXT(src1,32) / SEXT(src2,32),32));
  // 求余数字(不保证实现正确)
  INSTPAT("0000001 ????? ????? 110 ????? 01110 11", remw   , R, R(dest) = SEXT(SEXT(src1,32) % SEXT(src2,32),32));
  //逻辑运算
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(dest) = src1 | src2);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(dest) = src1 & imm);

  /* 移位指令IS类型 */
  ///逻辑移位
  
  // 立即数逻辑左移(未检查) 不知道是否需要使得shamt[5]==0,手册上只要求了riscv32I
  INSTPAT("000000? ????? ????? 001 ????? 00100 11", slli   , IS, R(dest) = src1 << imm );

  INSTPAT("000000? ????? ????? 001 ????? 00110 11", slliw  , IS, if(BITS(imm,5,5) == 0) R(dest) = SEXT(src1 << imm,32));
  INSTPAT("000000? ????? ????? 101 ????? 00100 11", srli   , IS,  R(dest) = src1 >> imm);
  INSTPAT("000000? ????? ????? 101 ????? 00110 11", srliw  , IS, if(BITS(imm,5,5) == 0) R(dest) = SEXT(BITS(src1,31,0) >> imm,32));
  INSTPAT("0000000 ????? ????? 101 ????? 01110 11", srlw   , R, R(dest) = SEXT((BITS(src1,31,0) >> BITS(src2,4,0)),32));

  ///算术移位
  
  // 实现原理: 将32位数进行符号扩展，若符号位为1,则右移时，补位为1,若符号位为0,则右移时补位为0 .(一定要将移位后的结果进行符号扩展)
  INSTPAT("010000? ????? ????? 101 ????? 00110 11", sraiw  , IS, if(BITS(imm,5,5) == 0) R(dest) = SEXT((SEXT(BITS(src1,31,0),32) >> imm),32));
  INSTPAT("0100000 ????? ????? 101 ????? 01110 11", sraw   , R, R(dest) = SEXT((SEXT(BITS(src1,31,0),32) >> BITS(src2,4,0)),32));
  INSTPAT("010000? ????? ????? 101 ????? 00100 11", srai   , IS, R(dest) = ((int64_t)src1 >> imm));

  /* 置位指令 */
  //有符号小于则置位
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(dest) = ((int64_t)src1 < (int64_t)src2));


  /*******跳转指令********/
  /*无条件跳转*/
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, R(dest) = s->pc + 4; s->dnpc = s->pc + imm); //跳转指令就要区分dnpc和snpc
  //ret 指令，相当于jalr x0,0(x1);
  //使用方法: 将调用函数时的(pc地址+4)存到src1,在函数调用结束后，使得pc = src1 
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", ret    , I, s->dnpc = src1); 

  /*条件分支,满足时pc加上偏移量，注意偏移量要乘2*/
  //不相等时分支
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, if(src1 != src2) s->dnpc = s->pc +imm); 
  //等于0时分支
  INSTPAT("??????? 00000 ????? 000 ????? 11000 11", beqz   , B, if(src1 == 0) s->dnpc = s->pc +imm); 
  //相等时分支
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, if(src1 == src2) s->dnpc = s->pc +imm); 
  // 大于等于时分支,有符号
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, if((int64_t)src1 >= (int64_t)src2) s->dnpc = s->pc +imm); 
  // 无符号小于时分支
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, if(src1 < src2) s->dnpc = s->pc + imm); 
  // 有符号小于时分支
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, if((int64_t)src1 < (int64_t)src2) s->dnpc = s->pc + imm); 

  
  //非法指令
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4); // // 取指，并更新snpc使其指向下一个pc
  return decode_exec(s); // 译码
}
