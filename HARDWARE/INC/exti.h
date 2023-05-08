#ifndef _EXTI_H
#define _EXTI_H
#include "sys.h"

extern char cunchu[4];
extern u8 fz_flag;
extern u16 bb12;
extern u16 bb13;
extern u16 bb14;

void fzopen();
void fzclose();
void Init_TIM6(u16 arr ,u16 psc);
void zuodongzuo(u8 xuhao,u8 yanshi);
	
#endif 