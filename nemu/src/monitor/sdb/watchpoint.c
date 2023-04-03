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

#include "sdb.h"
#include <stdint.h>

#define NR_WP 32


enum wp_status {
    KEEP = 1,
    SPARE,
    DISABLE,
};

typedef struct watchpoint {
    int                NO;
    struct watchpoint* next;
    /* TODO: Add more members if necessary */
    char* str[100];
} WP;

//static使得变量即使在超出文件范围时，也能保持其值  
static WP  wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
static uint64_t WP_NUM = 0; // 现有的监控点的数量

void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO   = i;
        wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    }

    head  = NULL;      //用来组织使用中的监视点结构
    free_ = wp_pool;   //用于组织空闲的监视点结构
}

/* TODO: Implement the functionality of watchpoint */

/* @brief:  删除链表某结点
 * @param:  wp 要删除的结点
 * @return: NULL
 */
void Linklist_del(WP *wp) {
    assert(wp != NULL);    
    wp->next = NULL;
}


/* @brief:  从free_中添加新的监视点,并加添进去的监视点从free_中删除
 * @param:  {NONE} 
 * @return: {NONE}
 */
void new_wp (void) {
    WP *q = head;
    Assert(free_ == NULL,"没有空闲的监视点了");
    int j = 0;
    do {
        if (q->next == NULL) {
            q->next         = free_;
            WP_NUM++;
        } else q = q->next;
        j++;
    } while (j < WP_NUM);    // 将空闲结点加入head组织的结构中
    //删除那个不再是空闲的结点
    free_ = free_->next;
}

/* @brief:  从head中释放指定的监视点结点
 * @param:  {WP* wp} 要删除的结点
 * @return: {void}
 */
void free_wp (WP *wp) {
    Assert(WP_NUM>0, "错误使用函数free_wp,在没有监视点的情况下，要求释放监视点");
    Assert(wp != head, "head不是监视点，他只是用来组织监视点结构的头结点");
    //找到wp在head组织的监视点结构中的位置
    WP *p = head, *pre = head; //pre是前导指针
    for (int i = 0; i < WP_NUM; i++) {
        pre = p;
        p = p->next;
        if (p == wp) {
            break;
        }
    }
    pre->next = p->next->next; //将结点wp从head链表中删除
    WP *q = free_;
    while (q->next != NULL) { //找到free_链表的指向NULL尾结点
       q = q->next; 
    }
    q->next = wp;
    wp->next = NULL;
}














